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

#include "romechastenerarmy.hpp"
#include "empire.hpp"
#include "events/notification.hpp"
#include "emperor.hpp"
#include "core/variant_map.hpp"
#include "core/logger.hpp"
#include "events/showinfobox.hpp"
#include "game/gamedate.hpp"
#include "config.hpp"
#include "city.hpp"
#include "objects_factory.hpp"

namespace world
{

namespace {
const int defaultSoldiersCount=16;
}

REGISTER_CLASS_IN_WORLDFACTORY(RomeChastenerArmy)

class RomeChastenerArmy::Impl
{
public:
  int soldiersNumber;
  bool messageSent;
  bool checkFavor;
};

RomeChastenerArmyPtr RomeChastenerArmy::create( EmpirePtr empire )
{
  RomeChastenerArmyPtr ret( new RomeChastenerArmy( empire ) );
  ret->drop();

  return ret;
}

void RomeChastenerArmy::setSoldiersNumber(unsigned int count) { _d->soldiersNumber = count; }

std::string RomeChastenerArmy::type() const { return TEXT(RomeChastenerArmy); }
unsigned int RomeChastenerArmy::soldiersNumber() const { return _d->soldiersNumber; }
void RomeChastenerArmy::setCheckFavor(bool value) { _d->checkFavor = value; }

void RomeChastenerArmy::timeStep(const unsigned int time)
{
  Army::timeStep( time );

  if( !_d->messageSent && game::Date::isWeekChanged() && _d->checkFavor )
  {
    bool emperorWantAttackCity = empire()->emperor().relation( target() ) > config::chastener::brokeAttack;
    if( emperorWantAttackCity )
    {
      Messenger::now( empire(), target(), "##message_from_centurion##", "##centurion_new_order_to_save_player##" );

      empire()->emperor().remSoldiers( target(), _d->soldiersNumber );
      deleteLater();
    }

    _d->messageSent = true;
  }
}

void RomeChastenerArmy::save(VariantMap& stream) const
{
  Army::save( stream );

  VARIANT_SAVE_ANY_D( stream, _d, soldiersNumber )
  VARIANT_SAVE_ANY_D( stream, _d, checkFavor )
  VARIANT_SAVE_ANY_D( stream, _d, messageSent )
}

void RomeChastenerArmy::load(const VariantMap& stream)
{
  Army::load( stream );

  VARIANT_LOAD_ANY_D( _d, soldiersNumber, stream )
  VARIANT_LOAD_ANY_D( _d, checkFavor, stream )
  VARIANT_LOAD_ANY_D( _d, messageSent, stream )
}

void RomeChastenerArmy::attack(ObjectPtr obj)
{
  Army::attack( obj );

  if( !target().empty() )
  {
    empire()->emperor().addSoldiers( target(), _d->soldiersNumber );

    events::GameEventPtr e = events::Notify::attack( obj->name(), "##rome_attack_empire_city##", this );
    e->dispatch();
  }
  else
  {
    Logger::warning( "WARNING!!! RomeChastenerArmy::attack cant attack unexist object" );
    deleteLater();
  }
}

RomeChastenerArmy::RomeChastenerArmy(EmpirePtr empire)
 : Army( empire ), _d( new Impl )
{
  _d->checkFavor = false;
  _d->messageSent = false;
  _d->soldiersNumber = defaultSoldiersCount;
}

}
