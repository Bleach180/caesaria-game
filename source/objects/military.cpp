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

#include "military.hpp"
#include "constants.hpp"
#include "game/resourcegroup.hpp"
#include "gfx/tilemap.hpp"
#include "walker/romesoldier.hpp"
#include "core/logger.hpp"
#include "events/event.hpp"
#include "walker/patrolpoint.hpp"
#include "barracks.hpp"
#include "city/city.hpp"
#include "game/gamedate.hpp"
#include "game/settings.hpp"
#include "core/saveadapter.hpp"
#include "walker/romehorseman.hpp"
#include "walker/helper.hpp"
#include "walker/romearcher.hpp"
#include "objects_factory.hpp"

using namespace gfx;

REGISTER_CLASS_IN_OVERLAYFACTORY(object::fort_javelin, FortJaveline)
REGISTER_CLASS_IN_OVERLAYFACTORY(object::fort_horse, FortMounted)
REGISTER_CLASS_IN_OVERLAYFACTORY(object::fort_legionaries, FortLegionary)

FortLegionary::FortLegionary()
  : Fort( object::fort_legionaries, 16 )
{
  _picture().load( ResourceGroup::security, 12 );
  _setFlagIndex( 21 );

  _addFormation( frmNorthDblLine );
  _addFormation( frmWestDblLine );
  _addFormation( frmSquad );
  _addFormation( frmOpen );

  _setWorkersType( walker::legionary );
}

int FortLegionary::flagIndex() const { return 21; }

void FortLegionary::_readyNewSoldier()
{
  TilesArray tiles = enterArea();

  if( !tiles.empty() )
  {
    auto soldier = Walker::create<RomeSoldier>( _city(), walker::legionary );
    soldier->send2city( this, tiles.front()->pos() );
    addWalker( soldier.object() );
  }
}

FortMounted::FortMounted()
  : Fort( object::fort_horse, 15 )
{
  _picture().load( ResourceGroup::security, 12 );
  _setFlagIndex( 39 );

  _addFormation( frmNorthLine );
  _addFormation( frmWestLine );
  _addFormation( frmNorthDblLine );
  _addFormation( frmWestDblLine );
  _addFormation( frmOpen );
}

bool FortMounted::build( const city::AreaInfo& info )
{
  return Fort::build( info );
}

int FortMounted::flagIndex() const { return 39; }

void FortMounted::_readyNewSoldier()
{
  TilesArray tiles = enterArea();

  if( !tiles.empty() )
  {
    auto soldier = Walker::create<RomeHorseman>( _city() );
    soldier->send2city( this, tiles.front()->pos() );
    addWalker( soldier.object() );
  }
}

FortJaveline::FortJaveline()
  : Fort( object::fort_javelin, 14 )
{
  _picture().load( ResourceGroup::security, 12 );
  _setFlagIndex( 30 );

  _addFormation( frmNorthDblLine );
  _addFormation( frmWestDblLine );
  _addFormation( frmOpen );
}

int FortJaveline::flagIndex() const { return 30;  }

void FortJaveline::_readyNewSoldier()
{
  TilesArray tiles = enterArea();

  if( !tiles.empty() )
  {
    auto archer = Walker::create<RomeArcher>( _city() );
    archer->send2city( this, tiles.front()->pos() );
    addWalker( archer.object() );
  }
}
