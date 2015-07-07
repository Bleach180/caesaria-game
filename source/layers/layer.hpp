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

#ifndef __CAESARIA_LAYER_H_INCLUDED__
#define __CAESARIA_LAYER_H_INCLUDED__

#include "core/referencecounted.hpp"
#include "core/smartptr.hpp"
#include "gfx/engine.hpp"
#include "gfx/tile.hpp"
#include "gfx/renderer.hpp"
#include "objects/constants.hpp"
#include "game/predefinitions.hpp"
#include "core/signals.hpp"
#include "walker/constants.hpp"
#include "core/flagholder.hpp"
#include "walker/constants.hpp"

#include <set>

namespace citylayer
{

class DrawOptions : public FlagHolder<int>
{
public:
  typedef enum { drawGrid=0x1, shadowOverlay=0x2, showPath=0x4, windowActive=0x8, showRoads=0x10,
                 showObjectArea=0x20, showWalkableTiles=0x40, showLockedTiles=0x80, showFlatTiles=0x100,
                 borderMoving=0x200, mayChangeLayer=0x400, oldGraphics=0x800, mmbMoving=0x1000
               } Flags;
  static DrawOptions& instance();

private:
  DrawOptions() {}
};

class Layer : public ReferenceCounted
{
public:
  typedef std::set<walker::Type> WalkerTypes;

  virtual int type() const = 0;
  virtual const WalkerTypes& visibleTypes() const;

  //draw gfx tprominent ile
  virtual void drawProminentTile(gfx::Engine& engine, gfx::Tile& tile, const Point& offset, const int depth, bool force );

  //draw gfx active tile
  virtual void drawTile(gfx::Engine& engine, gfx::Tile& tile, const Point& offset );

  //draw gfx after walkers
  virtual void drawWalkerOverlap(gfx::Engine& engine, gfx::Tile& tile, const Point& offset, const int depth );

  virtual void handleEvent( NEvent& event );
  virtual void drawPass(gfx::Engine& engine, gfx::Tile& tile, const Point& offset, gfx::Renderer::Pass pass );
  virtual void drawArea(gfx::Engine& engine, const gfx::TilesArray& area, const Point& offset,
                        const std::string& resourceGroup, int tileId );

  virtual void drawLands( gfx::Engine& engine, gfx::Camera* camera );
  virtual void drawWalkers( gfx::Engine& engine, const gfx::Tile& tile, const Point& camOffset );
  virtual void init( Point cursor );
  virtual void beforeRender( gfx::Engine& engine);
  virtual void afterRender( gfx::Engine& engine);
  virtual void render( gfx::Engine& engine);
  virtual void renderUi( gfx::Engine& engine );
  virtual void registerTileForRendering(gfx::Tile&);
  virtual void changeLayer( int type );
  virtual int nextLayer() const;
  virtual void destroy();

  virtual ~Layer();

protected:
  virtual void _initialize();

  void _setLastCursorPos( Point pos );
  Point _lastCursorPos() const;
  void _setStartCursorPos( Point pos );
  Point _startCursorPos() const;
  gfx::Tile* _currentTile() const;
  bool _isMovingButtonPressed( NEvent& event ) const;
  void _setTooltipText( const std::string& text );
  void _addWalkerType( walker::Type wtype );
  WalkerTypes& _visibleWalkers();
  bool _isVisibleObject( object::Type ovType );
  bool _moveCamera( NEvent& event );

  gfx::TilesArray _getSelectedArea( TilePos startPos=TilePos(-1,-1) );

  Layer( gfx::Camera* camera, PlayerCityPtr city );
  gfx::Camera* _camera();
  PlayerCityPtr _city();
  void _setNextLayer(int layer);

  __DECLARE_IMPL(Layer)
};

typedef SmartPtr<Layer> LayerPtr;

}//end namespace citylayer

#endif //__CAESAIRAS_LAYER_H_INCLUDED__
