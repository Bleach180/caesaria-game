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

#include "working.hpp"
#include "walker/walker.hpp"
#include "city/statistic.hpp"
#include "events/returnworkers.hpp"
#include "core/utils.hpp"
#include "core/variant_map.hpp"
#include "game/gamedate.hpp"
#include "objects/house.hpp"
#include "objects/house_level.hpp"
#include "core/format.hpp"
#include "events/removecitizen.hpp"
#include "core/common.hpp"
#include "core/logger.hpp"
#include "walker/typeset.hpp"

using namespace gfx;
using namespace events;

class WorkingBuilding::Impl
{
public:
  struct
  {
    unsigned int current;
    unsigned int maximum;
    walker::Type type;
  } workers;

  bool isActive;
  WalkerList walkerList;
  std::string errorStr;
  bool clearAnimationOnStop;
  unsigned int laborAccessKoeff;

public signals:
  Signal1<bool> onActiveChangeSignal;
};

WorkingBuilding::WorkingBuilding(const object::Type type, const Size& size)
  : Building(type, size), _d( new Impl )
{
  _d->workers.current = 0;
  _d->workers.type = walker::unknown;
  _d->workers.maximum = 0;
  _d->isActive = true;
  _d->clearAnimationOnStop = true;
  _d->laborAccessKoeff = 100;
  _animation().stop();
}

void WorkingBuilding::save( VariantMap& stream ) const
{
  Building::save( stream );
  VARIANT_SAVE_ANY_D( stream, _d, workers.current );
  VARIANT_SAVE_ANY_D( stream, _d, isActive );
  VARIANT_SAVE_ANY_D( stream, _d, workers.maximum );
  VARIANT_SAVE_ANY_D( stream, _d, laborAccessKoeff );
}

void WorkingBuilding::load( const VariantMap& stream)
{
  Building::load( stream );
  VARIANT_LOAD_ANY_D( _d, workers.current, stream );
  VARIANT_LOAD_ANY_D( _d, isActive, stream );
  VARIANT_LOAD_ANY_D( _d, workers.maximum, stream );
  VARIANT_LOAD_ANY_D( _d, laborAccessKoeff, stream );

  if( !_d->workers.maximum )
  {
    _d->workers.maximum = info().employers();
  }
}

std::string WorkingBuilding::workersProblemDesc() const
{
  return WorkingBuildingHelper::productivity2desc( const_cast<WorkingBuilding*>( this ) );
}

std::string WorkingBuilding::sound() const
{
  return (isActive() && numberWorkers() > 0
            ? Building::sound()
            : "");
}

std::string WorkingBuilding::troubleDesc() const
{
  std::string trouble = Building::troubleDesc();

  if( isNeedRoad() && roadside().empty() )
  {
    trouble = "##working_building_need_road##";
  }

  if( trouble.empty() )
  {
    if( numberWorkers() < maximumWorkers() / 2)
    {
      trouble = workersProblemDesc();
    }
    else if( _d->laborAccessKoeff < 50 )
    {
      trouble = "##working_build_poor_labor_warning##";
    }
  }

  return trouble;
}

void WorkingBuilding::initialize(const object::Info& mdata)
{
  Building::initialize( mdata );

  setMaximumWorkers( (unsigned int)mdata.employers() );
}

std::string WorkingBuilding::workersStateDesc() const { return ""; }
void WorkingBuilding::setMaximumWorkers(const unsigned int maxWorkers) { _d->workers.maximum = maxWorkers; }
unsigned int WorkingBuilding::maximumWorkers() const { return _d->workers.maximum; }
void WorkingBuilding::setWorkers(const unsigned int currentWorkers){  _d->workers.current = math::clamp( currentWorkers, 0u, _d->workers.maximum );}
unsigned int WorkingBuilding::numberWorkers() const { return _d->workers.current; }
unsigned int WorkingBuilding::needWorkers() const { return maximumWorkers() - numberWorkers(); }
math::Percent WorkingBuilding::productivity() const { return math::percentage( numberWorkers(), maximumWorkers() ); }
unsigned int WorkingBuilding::laborAccessPercent() const { return _d->laborAccessKoeff; }
bool WorkingBuilding::mayWork() const { return numberWorkers() > 0; }
void WorkingBuilding::setActive(const bool value) { _d->isActive = value; }
bool WorkingBuilding::isActive() const { return _d->isActive; }
WorkingBuilding::~WorkingBuilding(){}
const WalkerList& WorkingBuilding::walkers() const {  return _d->walkerList; }
bool WorkingBuilding::haveWalkers() const { return !_d->walkerList.empty(); }
std::string WorkingBuilding::errorDesc() const { return _d->errorStr; }
void WorkingBuilding::_setError(const std::string& err) { _d->errorStr = err;}
void WorkingBuilding::_setWorkersType(walker::Type type) { _d->workers.type = type; }
Signal1<bool>& WorkingBuilding::onActiveChange() { return _d->onActiveChangeSignal; }

unsigned int WorkingBuilding::addWorkers(const unsigned int workers )
{
  unsigned int maxAdd = std::min( workers, needWorkers() );
  setWorkers( numberWorkers() + maxAdd );
  return maxAdd;
}

unsigned int WorkingBuilding::removeWorkers(const unsigned int workers)
{
  unsigned int maxRemove = std::min( numberWorkers(), workers );
  setWorkers( numberWorkers() - maxRemove );
  return maxRemove;
}

void WorkingBuilding::timeStep( const unsigned long time )
{
  Building::timeStep( time );

  utils::eraseIfDeleted( _d->walkerList );

  if( game::Date::isMonthChanged() && numberWorkers() > 0 )
  {
    _d->laborAccessKoeff = _city()->statistic().objects.laborAccess( this );
  }

  if( isActive() )
    _updateAnimation( time );
}

void WorkingBuilding::_updateAnimation(const unsigned long time )
{
  if (game::Date::isDayChanged())
  {
    if (mayWork())
    {
      if (_animation().isStopped())
      {
        _changeAnimationState( true );
      }
    }
    else
    {
      if (_animation().isRunning())
      {
        _changeAnimationState( false );
      }
    }
  }

  if (_animation().isRunning())
  {
    _animation().update(time);
    const Picture& pic = _animation().currentFrame();
    if (pic.isValid() && !_fgPictures().empty())
    {
      _fgPictures().back() = _animation().currentFrame();
    }
  }
}

void WorkingBuilding::_changeAnimationState(bool enabled)
{
  if( enabled )
    _animation().start();
  else
  {
    _animation().stop();

    if( _d->clearAnimationOnStop && !_fgPictures().empty() )
    {
      _fgPictures().back() = Picture::getInvalid();
    }
  }
}

void WorkingBuilding::_setClearAnimationOnStop(bool value) {  _d->clearAnimationOnStop = value; }
walker::Type WorkingBuilding::workerType() { return _d->workers.type; }

void WorkingBuilding::_disaster()
{
  unsigned int buriedCitizens = math::random( numberWorkers() );

  events::dispatch<ReturnWorkers>( pos(), numberWorkers() );
  events::dispatch<RemoveCitizens>( pos(), CitizenGroup( CitizenGroup::mature, buriedCitizens ) );

  setWorkers( 0 );
}

void WorkingBuilding::addWalker( WalkerPtr walker )
{
  if( walker.isNull() )
  {
    Logger::warning( "!!! WorkingBuilding [{},{}] cant add null walker", pos().i(), pos().j() );
    return;
  }

  if( walker->isDeleted() )
  {
     Logger::warning( "!!! WorkingBuilding [{},{}] cant add walker [{}], because it also deleted", pos().i(), pos().j(), walker->name() );
    return;
  }

  _d->walkerList.push_back( walker );
}

void WorkingBuilding::destroy()
{
  Building::destroy();

  WalkerList mayDelete = walkers();
  utils::excludeByType( mayDelete, WalkerTypeSet( walker::cartPusher,
                                                  walker::supplier ) );
  for( auto wlk : mayDelete )
    wlk->deleteLater();

  if( numberWorkers() > 0 )
  {
    events::dispatch<ReturnWorkers>( pos(), numberWorkers() );
  }
}

void WorkingBuilding::collapse()
{
  Building::collapse();

  _disaster();
}

void WorkingBuilding::burn()
{
  Building::burn();

  _disaster();
}


namespace {

static const unsigned int productivityDescriptionCount = 6;
static const char* productivityDescription[] =
{
  "no_workers", "bad_work",
  "slow_work", "patrly_workers",
  "need_some_workers", "full_work"
};

}

std::string WorkingBuildingHelper::productivity2desc( WorkingBuildingPtr w, const std::string& prefix )
{
  std::string factoryType = w->info().typeName();
  unsigned int workKoeff = w->productivity() * productivityDescriptionCount / 100;

  workKoeff = math::clamp( workKoeff, 0u, productivityDescriptionCount-1 );

  if( prefix.empty() )
  {
    return fmt::format( "##{0}_{1}##",
                        factoryType, productivityDescription[ workKoeff ] );
  }
  else
  {
    return fmt::format( "##{0}_{1}_{2}##",
                        factoryType, prefix, productivityDescription[ workKoeff ] );
  }
}

std::string WorkingBuildingHelper::productivity2str( WorkingBuildingPtr w )
{
  unsigned int workKoeff = w->productivity() * productivityDescriptionCount / 100;
  workKoeff = math::clamp( workKoeff, 0u, productivityDescriptionCount-1 );

  return productivityDescription[ workKoeff ];
}
