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

#include "build.hpp"
#include "objects/objects_factory.hpp"
#include "game/game.hpp"
#include "game/funds.hpp"
#include "playsound.hpp"
#include "walker/enemysoldier.hpp"
#include "city/statistic.hpp"
#include "core/logger.hpp"
#include "objects/working.hpp"
#include "warningmessage.hpp"

using namespace gfx;
using namespace city;

namespace events
{

GameEventPtr BuildAny::create( const TilePos& pos, const object::Type type )
{
  return create( pos, TileOverlayFactory::instance().create( type ) );
}

GameEventPtr BuildAny::create(const TilePos& pos, OverlayPtr overlay)
{
  BuildAny* ev = new BuildAny();
  ev->_pos = pos;
  ev->_overlay = overlay;

  GameEventPtr ret( ev );
  ret->drop();

  return ret;
}

bool BuildAny::_mayExec(Game&, unsigned int) const {  return true;}

void BuildAny::_exec( Game& game, unsigned int )
{  
  if( _overlay.isNull() )
    return;

  OverlayPtr ctOv = game.city()->getOverlay( _pos );

  bool mayBuild = true;
  if( ctOv.isValid() )
  {
    mayBuild = ctOv->isDestructible();
  }

  TilePos offset(10, 10);
  EnemySoldierList enemies =  game.city()->statistic().walkers.find<EnemySoldier>( walker::any, _pos - offset, _pos + offset );
  if( !enemies.empty() && _overlay->group() != object::group::disaster)
  {
    GameEventPtr e = WarningMessage::create( "##too_close_to_enemy_troops##", 2 );
    e->dispatch();
    return;
  }

  if( !_overlay->isDeleted() && mayBuild )
  {
    city::AreaInfo info = { game.city(), _pos, TilesArray() };
    bool buildOk = _overlay->build( info );

    if( !buildOk )
    {
      Logger::warning( "BuildAny: some error when build [%d,%d] type:%s", _pos.i(), _pos.j(), _overlay->name().c_str() );
      return;
    }

    Desirability::update( game.city(), _overlay, Desirability::on );
    game.city()->addOverlay( _overlay );

    ConstructionPtr construction = ptr_cast<Construction>( _overlay );
    if( construction.isValid() )
    {
      const MetaData& buildingData = MetaDataHolder::getData( _overlay->type() );
      game.city()->treasury().resolveIssue( econ::Issue( econ::Issue::buildConstruction,
                                                         -(int)buildingData.getOption( MetaDataOptions::cost ) ) );

      if( construction->group() != object::group::disaster )
      {
        GameEventPtr e = PlaySound::create( "buildok", 1, 100 );
        e->dispatch();
      }

      if( construction->isNeedRoad() && construction->roadside().empty() )
      {
        GameEventPtr e = WarningMessage::create( "##building_need_road_access##", 1 );
        e->dispatch();
      }

      std::string error = construction->errorDesc();
      if( !error.empty() )
      {
        GameEventPtr e = WarningMessage::create( error, 1 );
        e->dispatch();
      }

      WorkingBuildingPtr wb = construction.as<WorkingBuilding>();
      if( wb.isValid() && wb->maximumWorkers() > 0 )
      {
        unsigned int worklessCount = game.city()->statistic().workers.workless();
        if( worklessCount < wb->maximumWorkers() )
        {
          GameEventPtr e = WarningMessage::create( "##city_need_more_workers##", 2 );
          e->dispatch();
        }

        int laborAccessKoeff = wb->laborAccessPercent();
        if( laborAccessKoeff < 50 )
        {
          GameEventPtr e = WarningMessage::create( "##working_build_poor_labor_warning##", 2 );
          e->dispatch();
        }
      }
    }
  }
  else
  {
    ConstructionPtr construction = _overlay.as<Construction>();
    if( construction.isValid() )
    {
      GameEventPtr e = WarningMessage::create( construction->errorDesc(), 1 );
      e->dispatch();
    }
  }
}

} //end namespace events
