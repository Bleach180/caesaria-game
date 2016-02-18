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

#include "soldier.hpp"
#include "city/statistic.hpp"
#include "helper.hpp"
#include "objects/construction.hpp"
#include "core/variant_map.hpp"

using namespace gfx;

class Soldier::Impl
{
public:
  Soldier::SldrAction sldAction;
  float strikeForce;
  float resistance;
  TilePos possibleAttackPos;
  std::set<int> friends;

  unsigned int attackDistance;
  int morale;

  Impl() : strikeForce ( 3.f ), resistance( 1.f ),
           attackDistance( 1u ), morale( 100 ) {}
};

Soldier::Soldier( PlayerCityPtr city, walker::Type type)
    : Human( city, type ), __INIT_IMPL(Soldier)
{
}

void Soldier::fight()
{
  setSpeed( 0.f );
  _setAction( acFight );
}

float Soldier::resistance() const { return _dfunc()->resistance; }
void Soldier::setResistance(float value) { _dfunc()->resistance = value;  }

float Soldier::strike() const { return _dfunc()->strikeForce; }
void Soldier::setStrike(float value) { _dfunc()->strikeForce = value; }

int Soldier::morale() const { return _dfunc()->morale; }

void Soldier::updateMorale(int value)
{
  int absValue = math::clamp( _dfunc()->morale+value, 0, 100 );
  _dfunc()->morale = absValue;
}

void Soldier::wait(int ticks)
{
  if( ticks < 0 )
  {
    _setSubAction( doNothing );
  }
  Walker::wait( ticks );
}

void Soldier::initialize(const VariantMap &options)
{
  Human::initialize( options );
  setResistance( options.get( "resistance", 1.f ) );
  setStrike( options.get( "strike", 3.f ) );

  Variant ad = options.get( "attackDistance" );
  if( ad.isValid() )
  {
    setAttackDistance( math::clamp( ad.toUInt(), 1u, 99u ) );
  }
}

bool Soldier::_move2freePos( TilePos target )
{
  const int defaultRange = 10;
  Pathway way2freeslot = _city()->statistic().walkers.freeTile<Soldier>( target, pos(), defaultRange );
  if( way2freeslot.isValid() )
  {
    _updatePathway( way2freeslot );
    go();
    _setSubAction( go2position );
    return true;
  }

  return false;
}


Soldier::~Soldier() {}

void Soldier::save(VariantMap& stream) const
{
  Walker::save( stream );
  __D_IMPL_CONST(d,Soldier);
  VARIANT_SAVE_ENUM_D(stream, d, sldAction )
  VARIANT_SAVE_ANY_D( stream, d, strikeForce )
  VARIANT_SAVE_ANY_D( stream, d, resistance )
  VARIANT_SAVE_ANY_D( stream, d, attackDistance )
  VARIANT_SAVE_ANY_D( stream, d, morale )
}

void Soldier::load(const VariantMap& stream)
{
  Walker::load( stream );
  __D_IMPL(d,Soldier)
  VARIANT_LOAD_ENUM_D(d, sldAction,         stream )
  VARIANT_LOAD_ANY_D( d, strikeForce,    stream )
  VARIANT_LOAD_ANY_D( d, resistance,     stream )
  VARIANT_LOAD_ANY_D( d, attackDistance, stream )
  VARIANT_LOAD_ANY_D( d, morale,         stream )
}

unsigned int Soldier::attackDistance() const{ return _dfunc()->attackDistance; }

Soldier::SldrAction Soldier::_subAction() const { return _dfunc()->sldAction; }
void Soldier::_setSubAction(Soldier::SldrAction action){ _dfunc()->sldAction = action; }
void Soldier::setAttackDistance(unsigned int distance) { _dfunc()->attackDistance = distance; }
int Soldier::experience() const { return 0; }
void Soldier::addFriend(walker::Type friendType){  _dfunc()->friends.insert( friendType );}

bool Soldier::isFriendTo(WalkerPtr wlk) const
{
  bool isFriend = _dfunc()->friends.count( wlk->type() ) > 0;
  if( !isFriend )
  {
    isFriend = WalkerRelations::isNeutral( type(), wlk->type() );

    if( nation() != world::nation::unknown )
    {
      isFriend = WalkerRelations::isNeutral( nation(), wlk->nation() );
    }
  }

  return isFriend;
}

void Soldier::setTarget(TilePos location) { _dfunc()->possibleAttackPos = location; }
TilePos Soldier::target() const { return _dfunc()->possibleAttackPos; }
