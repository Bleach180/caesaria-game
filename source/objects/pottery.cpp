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

#include "pottery.hpp"
#include "gfx/picture.hpp"
#include "game/resourcegroup.hpp"
#include "city/statistic.hpp"
#include "core/gettext.hpp"
#include "constants.hpp"
#include "good/store.hpp"
#include "objects_factory.hpp"

using namespace gfx;

REGISTER_CLASS_IN_OVERLAYFACTORY(object::pottery_workshop, Pottery)

Pottery::Pottery() : Factory(good::clay, good::pottery, object::pottery_workshop, Size(2))
{
  _fgPictures().resize( 3 );
}

bool Pottery::canBuild( const city::AreaInfo& areaInfo ) const
{
  bool ret = Factory::canBuild( areaInfo );
  return ret;
}

bool Pottery::build( const city::AreaInfo& info )
{
  Factory::build( info );
  bool haveClaypit = !info.city->statistic().objects.find<Building>( object::clay_pit ).empty();

  _setError( haveClaypit ? "" : "##need_clay_pit##" );

  return true;
}

void Pottery::timeStep( const unsigned long time )
{
  Factory::timeStep( time );
}

void Pottery::deliverGood()
{
  Factory::deliverGood();
}

void Pottery::_storeChanged()
{
  _fgPicture(1) = inStockRef().empty() ? Picture() : Picture( ResourceGroup::commerce, 157 );
  _fgPicture(1).setOffset( 45, -10 );
}
