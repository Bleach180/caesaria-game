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

#ifndef __CAESARIA_PATHWAY_H_INCLUDED__
#define __CAESARIA_PATHWAY_H_INCLUDED__

#include "predefinitions.hpp"
#include "core/position.hpp"
#include "core/variant.hpp"
#include "core/scopedptr.hpp"
#include "core/direction.hpp"
#include "core/delegate.hpp"
#include "gfx/predefinitions.hpp"

class Pathway : public ReferenceCounted
{
public:
  typedef enum { noFlags=0x0,
                 checkStart=0x1, checkStop=0x2,
                 roadOnly=0x4, deepWaterOnly=0x8, terrainOnly=0x10,
                 waterOnly=0x20, traversePath=0x40,
                 everyWhere=0x80, fourDirection=0x100,
                 customCondition=0x200, ignoreRoad=0x400 } Flag;

  typedef enum { forward, reverse } DirectionType;
  Pathway();
  Pathway( const Pathway& copy );

  virtual ~Pathway();

  void init(const gfx::Tile& origin);

  unsigned int length() const;

  const gfx::Tile& front() const;
  const gfx::Tile& back() const;
  const gfx::Tile& current() const;

  TilePos startPos() const;
  TilePos stopPos() const;

  bool isReverse() const;
  unsigned int curStep() const;

  void move( DirectionType type );

  bool isDestination() const;

  void next();
  Direction direction() const;

  void setNextDirection(const gfx::Tilemap& tmap, Direction direction );
  void setNextTile( const gfx::Tile& tile);
  void append( const Pathway& other );
  bool contains(const gfx::Tile &tile);
  const gfx::TilesArray& allTiles() const;

  void prettyPrint() const;
  void toggleDirection();

  Pathway& operator=(const Pathway& other);

  Pathway copy(unsigned int start, int stop=-1) const;

  void load( const gfx::Tilemap& tmap, const VariantMap& stream );
  VariantMap save() const;

  bool isValid() const;
private:
  class Impl;
  ScopedPtr< Impl > _d;
};

bool operator<(const Pathway& v1, const Pathway& v2);
typedef Delegate2< const gfx::Tile*, bool& > TilePossibleCondition;

#endif //__CAESARIA_PATHWAY_H_INCLUDED__
