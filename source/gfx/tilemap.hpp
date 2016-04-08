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
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#ifndef __CAESARIA_TILEMAP_H_INCLUDED__
#define __CAESARIA_TILEMAP_H_INCLUDED__

#include "core/serializer.hpp"
#include "predefinitions.hpp"
#include "objects/predefinitions.hpp"
#include "core/scopedptr.hpp"
#include "core/direction.hpp"
#include "game/climate.hpp"

namespace gfx
{

// Square Map of the Tiles.
class Tilemap : public Serializable
{
public:
  typedef enum { fSvkGround=0x1 } Flag;

  Tilemap();
  virtual ~Tilemap();
  void resize(const unsigned int size);

  bool isInside( const TilePos& pos ) const;

  Tile& at( const int i, const int j );
  Tile& at( const TilePos& ij );

  void setClimate( ClimateType climate);
  ClimateType climate() const;

  OverlayPtr overlay( const TilePos& ij );
  template<class T>
  SmartPtr<T> overlay(const TilePos &ij) { return ptr_cast<T>( overlay( ij ) ); }

  const Tile& at( const int i, const int j ) const;
  const Tile& at( const TilePos& ij ) const;
  TilesArray allTiles() const;
  const TilesArray& border() const;

  void clearSvkBorder();
  void setSvkBorderEnabled( bool enabled );
  Tile* svk_at( int i, int j ) const;
  TilesArray svkTiles() const;

  // returns all tiles on a rectangular perimeter
  // (i1, j1) : left corner of the rectangle (minI, minJ)
  // (i2, j2) : right corner of the rectangle (maxI, maxJ)
  // corners  : if false, don't return corner tiles
  TilesArray rect(TilePos start, TilePos stop, const bool corners = true);
  TilesArray rect(TilePos pos, Size size, const bool corners = true );
  TilesArray rect(unsigned int range, TilePos center );

  enum TileNeighbors
  {
    CheckCorners=1,
    FourNeighbors,
    //Corners,
    AllNeighbors
  };

  TilesArray getNeighbors(const TilePos& pos,int type=AllNeighbors);

  // returns all tiles in a rectangular area
  // (i1, j1) : left corner of the rectangle (minI, minJ)
  // (i2, j2) : right corner of the rectangle (maxI, maxJ)
  TilesArray area(const TilePos& start, const TilePos& stop ) const;
  TilesArray area(const TilePos& start, const Size& size ) const;
  TilesArray area(int range, const TilePos& center ) const;

  int size() const;

  void save( VariantMap& stream) const;
  void load( const VariantMap& stream);

  void turnRight();
  void turnLeft();

  void setFlag(Flag flag, bool enabled);

  Direction direction() const;

  TilePos fit( const TilePos& pos ) const;

  Tile* at(const Point& pos, bool overborder);
  TilePos p2tp( const Point& pos );

private:
  class Impl;
  ScopedPtr< Impl > _d;
};

}//end namespace gfx
#endif //__CAESARIA_TILEMAP_H_INCLUDED__
