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
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#include "minimap_window.hpp"
#include <GameGfx>
#include "game/minimap_colours.hpp"
#include "objects/overlay.hpp"
#include "core/time.hpp"
#include "core/event.hpp"
#include "core/gettext.hpp"
#include "city/city.hpp"
#include "objects/constants.hpp"
#include "walker/walker.hpp"
#include "core/tilerect.hpp"
#include "core/color_list.hpp"
#include "texturedbutton.hpp"
#include "city/states.hpp"
#include "widget_factory.hpp"

using namespace gfx;
using namespace constants;

namespace gui
{

REGISTER_CLASS_IN_WIDGETFACTORY(Minimap)

class Minimap::Impl
{
public:
  Size size;

  struct {
    Picture landRockWater;
    Picture objects;
    Picture walkers;
  } immediate;

  struct {
    Picture image;
    bool init;
  } bg;

  PlayerCityPtr city;

  unsigned int lastObjectsCount;
  ScopedPtr<minimap::Colors> colors;

  int lastTimeUpdate;
  Point center;
  TilePos tileposCenter;
  TexturedButton* btnZoomIn = nullptr;
  TexturedButton* btnZoomOut = nullptr;

  struct {
    Signal1<TilePos> onCenterChange;
    Signal1<int>     onZoomChange;
    Signal2<Widget*, TilePos> onCenterChangeEx;
    Signal2<Widget*, int>     onZoomChangeEx;
  } signal;

public:
  void getTileColours(const Tile& tile, int &c1, int &c2);
  void getTerrainColours(const Tile& tile, bool staticTiles, int& c1, int& c2 );
  void getObjectColours(const Tile& tile, int &c1, int &c2);
  void updateImage();
  void initStaticMmap();
  void drawStaticMmap(Picture& canvas , bool clear);
  void drawObjectsMmap(Picture& canvas, bool clear, bool force);
  void drawWalkersMmap(Picture& canvas, bool clear);
};

Minimap::Minimap(Widget* parent)
 : Minimap(parent, Rect(), nullptr, Size())
{

}

Minimap::Minimap(Widget* parent, const Rect& rect, PlayerCityPtr city, const Size& size)
  : Widget( parent, Hash(TEXT(Minimap)), rect ), _d( new Impl )
{
  setupUI( ":/gui/minimap.gui" );

  _d->size = size.width() == 0 ? Size( 144, 110 ) : size;
  _d->lastTimeUpdate = 0;
  _d->lastObjectsCount = 0;
  _d->bg.image = Picture( _d->size, 0, true );
  _d->bg.init = false;
  _d->btnZoomIn = &add<TexturedButton>( righttop() - Point( 28, -2  ), Size(24,24), -1, 605 );
  _d->btnZoomOut = &add<TexturedButton>( righttop() - Point( 28, -26 ), Size(24,24), -1, 601 );

  setCity(city);
  setTooltipText( _("##minimap_tooltip##") );
}

Point getBitmapCoordinates(int x, int y, int mapsize ) { return Point( x + y, x + mapsize - y - 1 ); }
void getBuildingColours( const Tile& tile, int &c1, int &c2 );

void Minimap::Impl::getTerrainColours( const Tile& tile, bool staticTiles, int& c1, int& c2 )
{
  int rndData = tile.imgId();
  int num3 = rndData & 0x3;
  int num7 = rndData & 0x7;

  if( !staticTiles && tile.getFlag( Tile::tlTree ) )
  {
    c1 = colors->colour(minimap::Colors::MAP_TREE1, num3);
    c2 = colors->colour(minimap::Colors::MAP_TREE2, num7);
  }
  else if (tile.getFlag( Tile::tlRock ))
  {
    c1 = colors->colour(minimap::Colors::MAP_ROCK1, num3);
    c2 = colors->colour(minimap::Colors::MAP_ROCK2, num3);
  }
  else if(tile.getFlag( Tile::tlDeepWater) )
  {
    c1 = colors->colour(minimap::Colors::MAP_WATER1, num3);
    c2 = colors->colour(minimap::Colors::MAP_WATER2, num3);
  }
  else if(tile.getFlag( Tile::tlWater ))
  {
    c1 = colors->colour(minimap::Colors::MAP_WATER1, num3);
    c2 = colors->colour(minimap::Colors::MAP_WATER2, num7);
  }
  else if ( !staticTiles && tile.getFlag( Tile::tlRoad ))
  {
    c1 = colors->colour(minimap::Colors::MAP_ROAD, 0);
    c2 = colors->colour(minimap::Colors::MAP_ROAD, 1);
  }
  else if (tile.getFlag( Tile::tlMeadow ))
  {
    c1 = colors->colour(minimap::Colors::MAP_FERTILE1, num3);
    c2 = colors->colour(minimap::Colors::MAP_FERTILE2, num7);
  }
  else if ( !staticTiles && tile.getFlag( Tile::tlWall ))
  {
    c1 = colors->colour(minimap::Colors::MAP_WALL, 0);
    c2 = colors->colour(minimap::Colors::MAP_WALL, 1);
  }
  else // plain terrain
  {
    c1 = colors->colour(minimap::Colors::MAP_EMPTY1, num3);
    c2 = colors->colour(minimap::Colors::MAP_EMPTY2, num7);
  }

  c1 |= 0xff000000;
  c2 |= 0xff000000;
}

void Minimap::Impl::getTileColours(const Tile& tile, int &c1, int &c2)
{
  if( !config::tilemap.isValidLocation( tile.pos() ) )
  {
    c1 = c2 = 0xff000000;
    return;
  }

  if( tile.getFlag( Tile::tlOverlay ) )
  {
    getObjectColours(tile, c1, c2);
  }
  else
  {
    getTerrainColours(tile, false, c1, c2);
  }

  c1 |= 0xff000000;
  c2 |= 0xff000000;

#ifdef GAME_PLATFORM_ANDROID
  c1 = NColor( c1 ).abgr();
  c2 = NColor( c2 ).abgr();
#endif
}

void Minimap::Impl::getObjectColours(const Tile& tile, int &c1, int &c2)
{
  OverlayPtr overlay = tile.overlay();

  if (overlay == NULL)
    return;

  bool colorFound = overlay->getMinimapColor( c1, c2 );

  if( !colorFound )
  {
    object::Group group = overlay->group();
    switch( group )
    {
      case object::group::military:
      {
        c1 = colors->colourA(ColorList::indianRed.color,1);
        c2 = colors->colourA(ColorList::indianRed.color,0);
        colorFound = true;
      }
      break;

      case object::group::food:
      {
        c1 = colors->colourA(ColorList::green.color,1);
        c2 = colors->colourA(ColorList::green.color,0);
        colorFound = true;
      }
      break;

      case object::group::industry:
      {
        c1 = colors->colourA(ColorList::brown.color,1);
        c2 = colors->colourA(ColorList::brown.color,0);
        colorFound = true;
      }
      break;

      case object::group::obtain:
      {
        c1 = colors->colourA(ColorList::sandyBrown.color,1);
        c2 = colors->colourA(ColorList::sandyBrown.color,0);
        colorFound = true;
      }
      break;

      case object::group::religion:
      {
        c1 = colors->colourA(ColorList::snow.color,1);
        c2 = colors->colourA(ColorList::snow.color,0);
        colorFound = true;
      }
      break;

      default: break;
    }
  }

  if( !colorFound )
  {
    switch (overlay->size().width())
    {
      case 1:
      {
        c1 = colors->colour(minimap::Colors::MAP_BUILDING, 0);
        c2 = colors->colour(minimap::Colors::MAP_BUILDING, 1);
      }
      break;
      default:
      {
        c1 = colors->colour(minimap::Colors::MAP_BUILDING, 0);
        c2 = colors->colour(minimap::Colors::MAP_BUILDING, 2);
      }
      break;
    }
  }

  c1 |= 0xff000000;
  c2 |= 0xff000000;

#ifdef GAME_PLATFORM_ANDROID
  c1 = NColor( c1 ).abgr();
  c2 = NColor( c2 ).abgr();
#endif
}

void Minimap::Impl::drawObjectsMmap( Picture& canvas, bool clear, bool force )
{
  if (city.isNull())
    return;

  Tilemap& tilemap = city->tilemap();
  int mapsize = tilemap.size();

  int c1, c2;
  const OverlayList& ovs = city->overlays();
  int mmapWidth = immediate.objects.width();
  int mmapHeight = immediate.objects.height();
  unsigned int* pixelsObjects = canvas.lock();

  if( lastObjectsCount != ovs.size() || force )
  {
    if( clear )
      canvas.fill( ColorList::clear );
    lastObjectsCount = ovs.size();

    for( auto overlay : ovs )
    {
      const Tile& tile = overlay->tile();

      getObjectColours( tile, c1, c2);

      TilePos pos = overlay->pos();
      const Size& size = overlay->size();
      for( int i=0; i < size.width(); i++ )
      {
        for( int j=0; j < size.height(); j++ )
        {
          Point pnt = getBitmapCoordinates( pos.i() + i, pos.j() + j, mapsize);
          if( pnt.y() < 0 || pnt.x() < 0 || pnt.x() > mmapWidth-1 || pnt.y() > mmapHeight )
            continue;

          unsigned int* bufp32;
          bufp32 = pixelsObjects + pnt.y() * mmapWidth + pnt.x();
          *bufp32 = c1;
          *(bufp32+1) = c2;
        }
      }
    }

    canvas.unlock();
    canvas.update();
  }
}

void Minimap::Impl::drawWalkersMmap( Picture& canvas, bool clear )
{
  if (city.isNull())
    return;

  Tilemap& tilemap = city->tilemap();
  int mapsize = tilemap.size();

  // here we can draw anything
  int mmapWidth = immediate.objects.width();
  int mmapHeight = immediate.objects.height();

  const WalkerList& walkers = city->walkers();
  if( clear )
    canvas.fill( ColorList::clear );

  unsigned int* pixelsObjects = canvas.lock();

  for( auto wlk : walkers )
  {
    const TilePos& pos = wlk->pos();

    NColor cl;
    if (wlk->agressive() != 0)
    {

      if (wlk->agressive() > 0)
      {
        cl = ColorList::red;
      }
      else
      {
        cl = ColorList::blue;
      }
    }
    else if( wlk->type() == walker::immigrant )
    {
      cl = ColorList::green;
    }

    if (cl.color != 0)
    {
      Point pnt = getBitmapCoordinates(pos.i(), pos.j(), mapsize);
      //canvas.fill(cl, Rect(pnt, Size(2)));

      if( pnt.y() < 0 || pnt.x() < 0 || pnt.x() > mmapWidth-1 || pnt.y() > mmapHeight )
        continue;

      unsigned int* bufp32;
      bufp32 = pixelsObjects + pnt.y() * mmapWidth + pnt.x();
      *bufp32 = cl.color;
      *(bufp32+1) = cl.color;
    }
  }

  canvas.unlock();
  canvas.update();
}

void Minimap::Impl::updateImage()
{
  drawObjectsMmap( immediate.objects, true, false );
  drawWalkersMmap( immediate.walkers, true );

  // show center of screen on minimap
  // Exit out of image size on small carts... please fix it

  /*sdlFacade.setPixel(surface, TilemapRenderer::instance().getMapArea().getCenterX(),     mapsize * 2 - TilemapRenderer::instance().getMapArea().getCenterZ(), kWhite);
  sdlFacade.setPixel(surface, TilemapRenderer::instance().getMapArea().getCenterX() + 1, mapsize * 2 - TilemapRenderer::instance().getMapArea().getCenterZ(), kWhite);
  sdlFacade.setPixel(surface, TilemapRenderer::instance().getMapArea().getCenterX(),     mapsize * 2 - TilemapRenderer::instance().getMapArea().getCenterZ() + 1, kWhite);
  sdlFacade.setPixel(surface, TilemapRenderer::instance().getMapArea().getCenterX() + 1, mapsize * 2 - TilemapRenderer::instance().getMapArea().getCenterZ() + 1, kWhite);

  for ( int i = TilemapRenderer::instance().getMapArea().getCenterX() - 18; i <= TilemapRenderer::instance().getMapArea().getCenterX() + 18; i++ )
  {
    sdlFacade.setPixel(surface, i, mapsize * 2 - TilemapRenderer::instance().getMapArea().getCenterZ() + 34, kYellow);
    sdlFacade.setPixel(surface, i, mapsize * 2 - TilemapRenderer::instance().getMapArea().getCenterZ() - 34, kYellow);
  }

  for ( int j = mapsize * 2 - TilemapRenderer::instance().getMapArea().getCenterZ() - 34; j <= mapsize * 2 - TilemapRenderer::instance().getMapArea().getCenterZ() + 34; j++ )
  {
    sdlFacade.setPixel(surface, TilemapRenderer::instance().getMapArea().getCenterX() - 18, j, kYellow);
    sdlFacade.setPixel(surface, TilemapRenderer::instance().getMapArea().getCenterX() + 18, j, kYellow);
  }
  */

  //fullmap->unlock();

  // this is window where minimap is displayed
}

void Minimap::Impl::initStaticMmap()
{
  if (city.isNull())
    return;

  Size size;
  Tilemap& tmap = city->tilemap();
  int mapSize = tmap.size();

  bg.image.fill( ColorList::black, Rect() );
  bg.image.update();

  size.setWidth( getBitmapCoordinates( mapSize-1, mapSize-1, mapSize ).x() );
  size.setHeight( getBitmapCoordinates( mapSize-1, 0, mapSize ).y() );

  immediate.landRockWater = Picture( size, 0, true );
  immediate.objects = Picture( size, 0, true );
  immediate.walkers = Picture( size, 0, true );
}

void Minimap::Impl::drawStaticMmap(Picture& canvas, bool clear)
{
  Tilemap& tmap = city->tilemap();
  int mapSize = tmap.size();

  if( clear )
    canvas.fill( ColorList::black, Rect() );

  int c1, c2;
  int mmapWidth = canvas.width();
  int mmapHeight = canvas.height();
  uint32_t* pixels = canvas.lock();

  TilesArray tiles = tmap.allTiles();
  tiles.append( tmap.svkTiles() );

  for( auto tile : tiles )
  {
    getTerrainColours( *tile, true, c1, c2);
    Point pnt = getBitmapCoordinates( tile->i(), tile->j(), mapSize);

    if( pnt.y() < 0 || pnt.x() < 0 || pnt.x() > mmapWidth-1 || pnt.y() > mmapHeight-1 )
      continue;

    unsigned int* bufp32;
    bufp32 = pixels + pnt.y() * mmapWidth + pnt.x();
    *bufp32 = c1;
    *(bufp32+1) = c2;
  }

  canvas.unlock();
  canvas.update();
}

/* end of helper functions */
namespace
{
  static const int kWhite  = 0xFFFFFF;
  static const int kYellow = 0xFFFF00;
}

void Minimap::draw(Engine& painter)
{
  if( !visible() )
    return;

  if (_d->city.isValid())
  {
    Tilemap& tilemap = _d->city->tilemap();
    int mapsize = tilemap.size();

    float koeff = height() / (float)110;
    Point p = getBitmapCoordinates(_d->tileposCenter.i(), _d->tileposCenter.j(), mapsize);
    Point myCenter(width()/2,height()/2);

    painter.resetColorMask();

    painter.draw( _d->bg.image, absoluteRect(), &absoluteClippingRectRef() );
    Rect baseRect( Point(), _d->immediate.landRockWater.size() );
    Rect drawRect = baseRect * koeff + absoluteRect().lefttop() + myCenter - p * koeff;
    painter.draw( _d->immediate.landRockWater, baseRect, drawRect, &absoluteClippingRectRef() );
    painter.draw( _d->immediate.objects, baseRect, drawRect, &absoluteClippingRectRef() );
    painter.draw( _d->immediate.walkers, baseRect, drawRect, &absoluteClippingRectRef() );
  }

  Widget::draw( painter );
}

void Minimap::setCenter( Point pos) { _d->center = pos; }

void Minimap::setTileCenter(const TilePos& tpos)
{
  _d->tileposCenter = tpos;
}

bool Minimap::onEvent(const NEvent& event)
{
  if( sEventGui == event.EventType && event::gui::buttonClicked == event.gui.type )
  {
    if (event.gui.caller == _d->btnZoomIn || event.gui.caller == _d->btnZoomOut)
    {
      int delta = event.gui.caller == _d->btnZoomIn ? +10 : -10;
      emit _d->signal.onZoomChange(delta);
      emit _d->signal.onZoomChangeEx(this, delta);
    }
    return true;
  }

  return Widget::onEvent( event );
}

void Minimap::beforeDraw(Engine& painter)
{
  Widget::beforeDraw( painter );

  if( !_d->bg.init )
  {
    _d->bg.init = true;
    painter.resetColorMask();
    _d->initStaticMmap();
    _d->drawStaticMmap( _d->immediate.landRockWater, true );
  }

  if( DateTime::elapsedTime() - _d->lastTimeUpdate > 250 )
  {
    _d->updateImage();
    _d->lastTimeUpdate = DateTime::elapsedTime();
  }
}

void Minimap::setCity(PlayerCityPtr city)
{
  _d->city = city;

  ClimateType type = city.isValid() ? city->climate() : game::climate::central;
  _d->colors.createInstance( type );

  _d->bg.init = false;
}

void Minimap::setSize(const Size& size)
{
  _d->size = size.width() == 0 ? Size( 144, 110 ) : size;
}

void Minimap::saveImage( const std::string& filename ) const
{
  Picture savePic( _d->immediate.landRockWater.size(), 0, true );
  _d->drawStaticMmap( savePic, true );
  _d->drawObjectsMmap( savePic, false, true );
  _d->drawWalkersMmap( savePic, false );
  savePic.save( filename );
}

void Minimap::update()
{
  _d->drawStaticMmap( _d->immediate.landRockWater, true );
  _d->lastObjectsCount = 0;
}

Signal1<TilePos>& Minimap::onCenterChange() { return _d->signal.onCenterChange; }
Signal1<int>& Minimap::onZoomChange() { return _d->signal.onZoomChange; }
Signal2<Widget*, TilePos>& Minimap::onCenterChangeEx() { return _d->signal.onCenterChangeEx; }
Signal2<Widget*, int>& Minimap::onZoomChangeEx() { return _d->signal.onZoomChangeEx; }

bool Minimap::_onMousePressed(const NEvent::Mouse& event)
{
  if (_d->city.isNull())
    return true;

  Point clickPosition = screenToLocal( event.pos() );

  int mapsize = _d->city->tilemap().size();
  Size minimapSize = _d->bg.image.size();

  Point offset( minimapSize.width()/2 - _d->center.x(), minimapSize.height()/2 + _d->center.y() - mapsize*2 );
  clickPosition -= offset;
  TilePos tpos;
  tpos.setI( (clickPosition.x() + clickPosition.y() - mapsize + 1) / 2 );
  tpos.setJ( -clickPosition.y() + tpos.i() + mapsize - 1 );

  emit _d->signal.onCenterChange( tpos );
  emit _d->signal.onCenterChangeEx( this, tpos );
  return true;
}

void Minimap::_finalizeResize()
{
  if (_d->btnZoomIn) _d->btnZoomIn->setPosition(righttop() - Point(28, -2));
  if (_d->btnZoomOut) _d->btnZoomOut->setPosition(righttop() - Point(28, -26));
  Widget::_finalizeResize();
}

}//end namespace gui
