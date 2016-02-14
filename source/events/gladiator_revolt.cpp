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

#include "gladiator_revolt.hpp"
#include "game/game.hpp"
#include "gfx/engine.hpp"
#include "core/utils.hpp"
#include "gui/environment.hpp"
#include "core/logger.hpp"
#include "world/romechastenerarmy.hpp"
#include "city/statistic.hpp"
#include "walker/enemysoldier.hpp"
#include "walker/walkers_factory.hpp"
#include "core/variant_map.hpp"
#include "core/gettext.hpp"
#include "showinfobox.hpp"
#include "game/funds.hpp"
#include "objects/training.hpp"
#include "gui/film_widget.hpp"
#include "world/empire.hpp"
#include "game/gamedate.hpp"
#include "fundissue.hpp"
#include "factory.hpp"

namespace events
{

REGISTER_EVENT_IN_FACTORY(GladiatorRevolt, "gladiator_revolt")

class GladiatorRevolt::Impl
{
public:
  int count;
  bool isDeleted;
};

GameEventPtr GladiatorRevolt::create()
{
  GameEventPtr ret( new GladiatorRevolt() );
  ret->drop();

  return ret;
}

bool GladiatorRevolt::isDeleted() const { return _d->isDeleted; }

void GladiatorRevolt::load(const VariantMap& stream)
{
  GameEvent::load( stream );
  VARIANT_LOAD_ANY_D( _d, count, stream )
  VARIANT_LOAD_ANY_D( _d, isDeleted, stream )
}

VariantMap GladiatorRevolt::save() const
{
  VariantMap ret = GameEvent::save();
  VARIANT_SAVE_ANY_D( ret, _d, count )
  VARIANT_SAVE_ANY_D( ret, _d, isDeleted )
  return ret;
}

bool GladiatorRevolt::_mayExec(Game& game, unsigned int time) const
{
  return true;
}

GladiatorRevolt::GladiatorRevolt() : _d( new Impl )
{
  _d->isDeleted = false;
  _d->count = 0;
}

void GladiatorRevolt::_exec(Game& game, unsigned int)
{
  GladiatorSchoolList gladSchool = game.city()->statistic().objects.find<GladiatorSchool>( object::gladiatorSchool );

  if( !gladSchool.empty() )
  {
    TilePos location = gladSchool.random()->pos();

    for( int k=0; k < _d->count; k++ )
    {
      auto rioter = WalkerManager::instance().create<EnemySoldier>( walker::gladiatorRiot, game.city() );
      if( rioter.isValid() )
      {
        rioter->send2City( location );
        rioter->wait( math::random( k * 30 ) );
        rioter->setAttackPriority( EnemySoldier::attackAll );
        rioter->setSpeedMultiplier( 0.7 + math::random( 60 ) / 100.f  );
      }
    }
  }
}

}
