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

#include "tree.hpp"
#include "game/resourcegroup.hpp"
#include "gfx/helper.hpp"
#include "city/statistic.hpp"
#include "gfx/tilemap.hpp"
#include "objects/construction.hpp"
#include "objects_factory.hpp"
#include "core/variant_map.hpp"

using namespace gfx;

REGISTER_CLASS_IN_OVERLAYFACTORY(object::tree, Tree)

namespace {
CAESARIA_LITERALCONST(treeFlat)
}

Tree::Tree()
  : Overlay( object::tree, Size(1) )
{
}

void Tree::timeStep( const unsigned long time )
{
  Overlay::timeStep( time );
}

bool Tree::isFlat() const { return _isFlat; }

void Tree::initTerrain(Tile& terrain)
{
  terrain.setFlag( Tile::tlTree, true );
}

bool Tree::build( const city::AreaInfo& info )
{
  std::string picname = imgid::toResource( info.city->tilemap().at( info.pos ).originalImgId() );
  _picture().load( picname );
  _isFlat = (picture().height() <= tilemap::cellPicSize().height());
  return Overlay::build( info );
}

void Tree::save(VariantMap& stream) const
{
  Overlay::save( stream );

  stream[ literals::treeFlat ] = _isFlat;
}

void Tree::load(const VariantMap& stream)
{
  Overlay::load( stream );

  _isFlat = stream.get( literals::treeFlat );
}

void Tree::destroy()
{
  TilesArray tiles = area();
  for( auto it : tiles )
    it->setFlag( Tile::tlTree, false );
}
