// This file is part of CaesarIA.
//
// CaesarIA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CaesarIA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CaesarIA.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2015 Dalerank, dalerankn8@gmail.com

#include "build.hpp"
#include "objects/aqueduct.hpp"
#include "constants.hpp"
#include "walker/constants.hpp"
#include "game/roadbuild_helper.hpp"
#include "core/logger.hpp"
#include "events/build.hpp"
#include "core/foreach.hpp"
#include "city/city.hpp"
#include "core/event.hpp"
#include "gfx/sdl_engine.hpp"
#include "core/font.hpp"
#include "objects/fortification.hpp"
#include "core/utils.hpp"
#include "gfx/camera.hpp"
#include "gui/dialogbox.hpp"
#include "gfx/renderermode.hpp"
#include "events/warningmessage.hpp"
#include "game/funds.hpp"
#include "game/settings.hpp"
#include "walker/walker.hpp"
#include "gfx/tilearea.hpp"
#include "gfx/city_renderer.hpp"
#include "gfx/helper.hpp"
#include "layerdestroy.hpp"
#include "gfx/tilemap.hpp"

using namespace gui;
using namespace gfx;
using namespace events;

namespace citylayer
{
static const int frameCountLimiter=25;

class Build::Impl
{
public:
  bool multiBuilding;
  TilePos lastTilePos;
  TilePos startTilePos;
  bool kbShift, kbCtrl;
  bool lmbPressed;
  bool borderBuilding;
  bool roadAssignment;
  bool needUpdateTiles;
  bool drawTileBasicPicture;
  int drawLayerIndex;
  int frameCount;
  int money4Construction;
  Renderer* renderer;
  LayerPtr lastLayer;
  std::string resForbiden;
  Font textFont;
  Picture textPic;
  TilesArray buildTiles;  // these tiles have draw over "normal" tilemap tiles!
};

void Build::_discardPreview()
{
  __D_IMPL(d,Build)
  foreach( tile, d->buildTiles )
  {
    if( (*tile)->overlay().isValid() )
    {
      (*tile)->overlay()->deleteLater();
    }

    delete *tile;
  }

  d->buildTiles.clear();
}

void Build::_checkPreviewBuild(TilePos pos)
{
  __D_IMPL(d,Build);
  BuildModePtr bldCommand = ptr_cast<BuildMode>( d->renderer->mode() );

  if (bldCommand.isNull())
    return;

  // TODO: do only when needed, when (i, j, _buildInstance) has changed
  ConstructionPtr overlay = bldCommand->contruction();

  if( !overlay.isValid() )
  {
    return;
  }

  Size size = overlay->size();
  int cost = MetaDataHolder::getData( overlay->type() ).getOption( MetaDataOptions::cost );

  bool walkersOnTile = false;
  if( bldCommand->flag( LayerMode::checkWalkers ) )
  {
    TilesArray tiles = overlay->area();
    foreach( t, tiles )
    {
      const WalkerList& walkers = _city()->walkers( (*t)->pos() );

      if( !walkers.empty() )
      {
        walkersOnTile = true;
        break;
      }
    }
  }

  city::AreaInfo areaInfo = { _city(), pos, d->buildTiles };
  if( !walkersOnTile && overlay->canBuild( areaInfo ) )
  {
    //bldCommand->setCanBuild(true);
    Tilemap& tmap = _city()->tilemap();
    Tile *masterTile=0;
    d->money4Construction += cost;
    for (int dj = 0; dj < size.height(); ++dj)
    {
      for (int di = 0; di < size.width(); ++di)
      {
        Tile& basicTile =  tmap.at( pos + TilePos( di, dj ) );
        Tile* tile = new Tile( basicTile.pos() );  // make a copy of tile
        tile->setEPos( basicTile.epos() );

        if (di==0 && dj==0)
        {
          // this is the masterTile
          masterTile = tile;
        }
        tile->setPicture( tmap.at( pos + TilePos( di, dj ) ).picture() );
        tile->setMasterTile( masterTile );
        tile->setOverlay( ptr_cast<Overlay>( overlay ) );
        d->buildTiles.push_back( tile );
      }
    }
  }
  else
  {
    //bldCommand->setCanBuild(false);
    Picture grnPicture( d->resForbiden, 1);
    Picture redPicture( d->resForbiden, 2);

    //TilemapArea area = til
    Tilemap& tmap = _city()->tilemap();
    for (int dj = 0; dj < size.height(); ++dj)
    {
      for (int di = 0; di < size.width(); ++di)
      {
        TilePos rPos = pos + TilePos( di, dj );
        if( !tmap.isInside( rPos ) )
          continue;

        const Tile& basicTile = tmap.at( rPos );
        const bool isConstructible = basicTile.getFlag( Tile::isConstructible );
        Tile* tile = new Tile( basicTile.pos() );  // make a copy of tile
        tile->setEPos( basicTile.epos() );

        walkersOnTile = false;
        if( bldCommand->flag( LayerMode::checkWalkers ) )
        {
          walkersOnTile = !_city()->walkers( rPos ).empty();
        }

        tile->setPicture( (!walkersOnTile && isConstructible) ? grnPicture : redPicture );
        tile->setMasterTile( 0 );
        tile->setFlag( Tile::clearAll, true );
        tile->setOverlay( 0 );
        d->buildTiles.push_back( tile );
      }
    }
  }
}

void Build::_checkBuildArea()
{
  __D_IMPL(_d,Build);
  if( !_d->lmbPressed || _startCursorPos().x() < 0 )
  {
    _setStartCursorPos( _lastCursorPos() );

    Tile* tile = _camera()->at( _lastCursorPos(), true );
    _d->startTilePos = tile ? tile->epos() : tilemap::invalidLocation();
  }
}

bool compare_tile(const Tile* one, const Tile* two)
{
  return one->pos().z() > two->pos().z();
}

void Build::_updatePreviewTiles( bool force )
{
  __D_IMPL(d,Build);
  Tile* curTile = _camera()->at( _lastCursorPos(), true );

  if( !curTile )
    return;

  if( !force && d->lastTilePos == curTile->epos() )
    return;

  if( !d->multiBuilding )
  {
    _setStartCursorPos( _lastCursorPos() );
    d->startTilePos = curTile->pos();
  }

  d->lastTilePos = curTile->epos();

  _discardPreview();
  d->money4Construction = 0;

  if( d->borderBuilding )
  {
    Tile* startTile = _camera()->at( d->startTilePos );  // tile under the cursor (or NULL)
    Tile* stopTile  = _camera()->at( _lastCursorPos(),  true );

    TilesArray pathWay = RoadPropagator::createPath( _city()->tilemap(),
                                                     startTile->epos(), stopTile->epos(),
                                                     d->roadAssignment, d->kbShift );
    Tilemap& tmap = _city()->tilemap();
    TilePos leftUpCorner = pathWay.leftUpCorner();
    TilePos rigthDownCorner = pathWay.rightDownCorner();
    TilePos leftDownCorner( leftUpCorner.i(), rigthDownCorner.j() );
    TilesArray ret;

    int mmapSize = std::max<int>( leftUpCorner.j() - rigthDownCorner.j() + 1,
                                  rigthDownCorner.i() - leftUpCorner.i() + 1 );
    for( int y=0; y < mmapSize; y++ )
    {
      for( int t=0; t <= y; t++ )
      {
        TilePos tpos = leftDownCorner + TilePos( t, mmapSize - 1 - ( y - t ) );
        if( pathWay.contain( tpos ) )
          ret.push_back( &tmap.at( tpos ) );
      }
    }

    for( int x=1; x < mmapSize; x++ )
    {
      for( int t=0; t < mmapSize-x; t++ )
      {
        TilePos tpos = leftDownCorner + TilePos( x + t, t );
        if( pathWay.contain( tpos ) )
          ret.push_back( &tmap.at( tpos ) );
      }
    }

    pathWay = ret;
    foreach( it, pathWay )
    {
      _checkPreviewBuild( (*it)->epos() );
    }
  }
  else
  {
    TilesArray tiles = _getSelectedArea( d->startTilePos );

    foreach( it, tiles ) { _checkPreviewBuild( (*it)->epos() ); }
  }  

  std::sort( d->buildTiles.begin(), d->buildTiles.end(), compare_tile );

  d->textPic.fill( 0x0, Rect() );
  d->textFont.setColor( 0xffff0000 );
  d->textFont.draw( d->textPic, utils::i2str( d->money4Construction ) + " Dn", Point() );
}

void Build::_buildAll()
{
  __D_IMPL(d,Build);
  BuildModePtr bldCommand = ptr_cast<BuildMode>( d->renderer->mode() );
  if( bldCommand.isNull() )
    return;

  ConstructionPtr cnstr = bldCommand->contruction();

  if( !cnstr.isValid() )
  {
    Logger::warning( "LayerBuild: No construction for build" );
    return;
  }

  if( !_city()->treasury().haveMoneyForAction( 1 ) )
  {
    GameEventPtr e = WarningMessage::create( "##out_of_credit##", 2 );
    e->dispatch();
    return;
  }

  bool buildOk = false;  
  city::AreaInfo areaInfo = { _city(), TilePos(), TilesArray() };
  foreach( it, d->buildTiles )
  {
    Tile* tile = *it;
    areaInfo.pos = tile->epos();
    if( cnstr->canBuild( areaInfo ) && tile->isMasterTile())
    {
      GameEventPtr event = BuildAny::create( tile->epos(), cnstr->type() );
      event->dispatch();
      buildOk = true;
    }
  }

  d->startTilePos = d->lastTilePos;

  if( !buildOk )
  {
    std::string errorStr = cnstr->errorDesc();

    GameEventPtr event = WarningMessage::create( errorStr.empty()
                                                   ? "##need_build_on_cleared_area##"
                                                   : errorStr, WarningMessage::neitral );
    event->dispatch();
  }
}

void Build::_exitBuildMode()
{
  __D_IMPL(_d,Build);
  DrawOptions::instance().setFlag( DrawOptions::mayChangeLayer, true );
  _setNextLayer( _d->lastLayer.isValid() ? _d->lastLayer->type() : citylayer::simple );
  _discardPreview();
}

void Build::handleEvent(NEvent& event)
{
  __D_IMPL(_d,Build);
  if( event.EventType == sEventMouse )
  {
    _d->kbShift = event.mouse.shift;
    _d->kbCtrl = event.mouse.control;
    _d->lmbPressed = event.mouse.isLeftPressed();

    switch( event.mouse.type  )
    {
    case mouseMoved:
    {
      _setLastCursorPos( event.mouse.pos() );
      _checkBuildArea();
      _updatePreviewTiles( false );
    }
    break;

    case mouseLbtnPressed:
    {
      _updatePreviewTiles( false );
    }
    break;

    case mouseLbtnRelease:            // left button
    {
      Tile* tile = _camera()->at( event.mouse.pos(), false );  // tile under the cursor (or NULL)
      if( tile == 0 )
      {
        break;
      }

#ifndef CAESARIA_PLATFORM_ANDROID
      _finishBuild();
#endif
    }
    break;

    case mouseRbtnRelease: { _exitBuildMode(); } break;
    default:    break;
    }
  }

  if( event.EventType == sEventKeyboard )
  {
    bool handled = _moveCamera( event );
    _d->kbShift = event.keyboard.shift;
    _d->kbCtrl = event.keyboard.control;

    if( !handled )
    {
      switch( event.keyboard.key )
      {
      case KEY_ESCAPE: _exitBuildMode(); break;
      case KEY_RETURN:
      {
        if( !event.keyboard.pressed )  //button was left up
        {
          _finishBuild();
        }
      }
      break;
      default: break;
      }
    }
    else
    {
      _d->needUpdateTiles = true;
    }
  }
}

void Build::_finishBuild()
{
  _buildAll();
  _setStartCursorPos( _lastCursorPos() );
  _updatePreviewTiles( true );
}

int Build::type() const {  return citylayer::build; }

void Build::_drawBuildTiles( Engine& engine)
{
  __D_IMPL(_d,Build);
  Point offset = _camera()->offset();
  city::AreaInfo areaInfo = { _city(), TilePos(), _d->buildTiles };
  foreach( it, _d->buildTiles )
  {
    Tile* postTile = *it;
    postTile->resetWasDrawn();

    if( postTile->masterTile() )
      postTile = postTile->masterTile();

    ConstructionPtr ptr_construction = ptr_cast<Construction>( postTile->overlay() );
    engine.resetColorMask();

    areaInfo.pos = postTile->epos();
    if( ptr_construction.isValid() && ptr_construction->canBuild( areaInfo ) )
    {
      engine.setColorMask( 0x00000000, 0x0000ff00, 0, 0xff000000 );

      drawPass( engine, **it, offset, Renderer::ground );
      drawPass( engine, **it, offset, Renderer::groundAnimation );
    }

    drawPass( engine, *postTile, offset, Renderer::ground );
    drawPass( engine, *postTile, offset, Renderer::groundAnimation );

    drawProminentTile( engine, *postTile, offset, postTile->epos().z(), true );
  }

  engine.resetColorMask();
}

void Build::drawTile( Engine& engine, Tile& tile, const Point& offset )
{
  __D_IMPL(_d,Build);
  Point screenPos = tile.mappos() + offset;

  ConstructionPtr cntr = ptr_cast<Construction>( tile.overlay() );
  city::AreaInfo info = { _city(), tile.epos(), _d->buildTiles };

  if( _d->drawTileBasicPicture )
  {
    const Picture* picBasic = 0;
    const Picture* picOver = 0;
    if( cntr.isValid() && info.aroundTiles.size() > 0 )
    {
      picBasic = &cntr->picture();
      picOver = &cntr->picture( info );
    }

    if( picOver && picBasic != picOver )
    {
      drawPass( engine, tile, offset, Renderer::ground );
      engine.draw( *picOver, screenPos );
      drawPass( engine, tile, offset, Renderer::overlayAnimation );
    }
    else if( _d->lastLayer.isValid() )
    {
      _d->lastLayer->drawTile( engine, tile, offset );
    }
    else
    {
      Layer::drawTile( engine, tile, offset );
    }
  }
  else
  {
    if( cntr.isValid() )
    {
      const Picture& picOver = cntr->picture( info );
      engine.draw( picOver, screenPos );
      drawPass( engine, tile, offset, Renderer::overlayAnimation );
    }
    else
    {
      Layer::drawTile( engine, tile, offset );
    }
  }
}

void Build::render( Engine& engine)
{
  __D_IMPL(d,Build);
  d->drawTileBasicPicture = true;
  Layer::render( engine );

  if( ++d->frameCount >= frameCountLimiter)
  {
    _updatePreviewTiles( true );
  }

  d->frameCount %= frameCountLimiter;

  d->drawTileBasicPicture = false;
  _drawBuildTiles( engine );
}

void Build::_initBuildMode()
{
  __D_IMPL(_d,Build);
  BuildModePtr command = ptr_cast<BuildMode>( _d->renderer->mode() );
  Logger::warningIf( !command.isValid(), "LayerBuild: init unknown command" );

  _d->multiBuilding = command.isValid() ? command->flag( LayerMode::multibuild ) : false;
  _d->roadAssignment = command.isValid() ? command->flag( LayerMode::assign2road ) : false;
  _d->borderBuilding = command.isValid() ? command->flag( LayerMode::border ) : false;
}

void Build::init(Point cursor)
{
  __D_IMPL(_d,Build);
  Layer::init( cursor );

  _d->lastTilePos = tilemap::invalidLocation();
  _d->startTilePos = tilemap::invalidLocation();
  _d->kbShift = false;
  _d->kbCtrl = false;

  changeLayer( _d->renderer->currentLayer()->type() );

  DrawOptions::instance().setFlag( DrawOptions::mayChangeLayer, false );
}

void Build::beforeRender(Engine& engine)
{
  __D_IMPL(_d,Build);
  if( _d->lastLayer.isValid() )
    _d->lastLayer->beforeRender( engine );
  else
    Layer::beforeRender( engine );
}

void Build::afterRender(Engine& engine)
{
  __D_IMPL(_d,Build);
  if( _d->needUpdateTiles )
  {
    _setLastCursorPos( engine.cursorPos() );
    _checkBuildArea();
    _updatePreviewTiles( false );
  }

  if( _d->lastLayer.isValid() )
    _d->lastLayer->afterRender( engine );
  else
    Layer::afterRender( engine );
}

const Layer::WalkerTypes& Build::visibleTypes() const
{
  __D_IMPL_CONST(_d,Build);
  if( _d->lastLayer.isValid() )
    return _d->lastLayer->visibleTypes();

  return Layer::visibleTypes();
}

void Build::renderUi(Engine &engine)
{
  Layer::renderUi( engine );
  engine.draw( _dfunc()->textPic, engine.cursorPos() + Point( 10, 10 ));
}

void Build::changeLayer(int layer)
{
  __D_IMPL(_d,Build);
  if( layer != type() )
  {
    if( layer == citylayer::destroyd )
    {
       //_d->lastLayer = LayerPtr();
       _exitBuildMode();
    }
    else
    {
      _d->lastLayer = _d->renderer
                          ? _d->renderer->getLayer( layer )
                          : LayerPtr();
    }
  }

  _initBuildMode();
}

LayerPtr Build::create(Renderer& renderer, PlayerCityPtr city)
{
  LayerPtr ret( new Build( renderer, city ) );
  ret->drop();

  return ret;
}

LayerPtr Build::drawLayer() const { return _dfunc()->lastLayer; }
Build::~Build() {}

Build::Build( Renderer& renderer, PlayerCityPtr city)
  : Layer( renderer.camera(), city ),
    __INIT_IMPL(Build)
{
  __D_IMPL(d,Build);
  d->renderer = &renderer;
  d->frameCount = 0;
  d->needUpdateTiles = false;
  d->resForbiden = SETTINGS_VALUE( forbidenTile ).toString();
  d->startTilePos = gfx::tilemap::invalidLocation();
  d->textFont = Font::create( FONT_5 );
  d->textPic = Picture( Size( 100, 30 ), 0, true );
  _addWalkerType( walker::all );
}

}//end namespace citylayer
