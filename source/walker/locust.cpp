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
// Copyright 2012-2013 Dalerank, dalerankn8@gmail.com

#include "locust.hpp"
#include "core/variant_map.hpp"
#include "core/gettext.hpp"
#include "core/foreach.hpp"
#include "gfx/tilemap.hpp"
#include "constants.hpp"
#include "city/city.hpp"
#include "game/gamedate.hpp"
#include "objects/farm.hpp"

using namespace gfx;

class Locust::Impl
{
public:
  int counter;
  int time;
  bool loop;
  Picture picture;
};

WalkerPtr Locust::create(PlayerCityPtr city)
{
  Locust* locust = new Locust( city );

  WalkerPtr ret( locust );
  ret->drop();

  return ret;
}

void Locust::create(PlayerCityPtr city, TilePos pos, int time)
{
  Locust* locust = new Locust( city );
  locust->setPos( pos );
  locust->_d->time = time;

  WalkerPtr ret( locust );
  ret->drop();
  ret->attach();
}

Locust::Locust( PlayerCityPtr city ) : Walker( city ), _d( new Impl )
{
  _setType( walker::locust );

  _d->time = 0;

  setName( _("##locust##") );
  _setHealth( 0 );

  setFlag( vividly, false );
}

Locust::~Locust() {}

void Locust::timeStep(const unsigned long time)
{
  _d->counter++;

  if( game::Date::isWeekChanged() )
  {
    FarmPtr farm = _city()->getOverlay( pos() ).as<Farm>();
    if( farm.isValid() && farm->type() != object::meat_farm )
    {
      farm->updateProgress( -50 );
    }
  }

  if( _d->counter > _d->time )
  {
    deleteLater();
  }
}

void Locust::save( VariantMap& stream ) const
{
  Walker::save( stream );

  VARIANT_SAVE_ANY_D( stream, _d, time )
  VARIANT_SAVE_ANY_D( stream, _d, counter )
}

void Locust::load( const VariantMap& stream )
{
  Walker::load( stream );

  VARIANT_LOAD_ANY_D( _d, time, stream )
  VARIANT_LOAD_ANY_D( _d, counter, stream )
}

const Picture& Locust::getMainPicture()
{
  return _d->picture;
}
