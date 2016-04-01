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

#include "well.hpp"
#include "game/resourcegroup.hpp"
#include "walker/serviceman.hpp"
#include "gfx/tile.hpp"
#include "core/common.hpp"
#include "house.hpp"
#include "city/statistic.hpp"
#include "constants.hpp"
#include "objects_factory.hpp"

using namespace gfx;

REGISTER_CLASS_IN_OVERLAYFACTORY(object::well, Well)

namespace {
const unsigned int wellServiceRange = 2;
}

Well::Well() : ServiceBuilding( Service::well, object::well, Size::square(1) )
{
  setWorkers( 0 );
}

void Well::deliverService()
{
  ServiceWalkerPtr walker = Walker::create<ServiceWalker>( _city(), serviceType() );
  walker->setBase( this );

  ReachedBuildings reachedBuildings = walker->getReachedBuildings( tile().pos() );

  for( auto bld : reachedBuildings)
    bld->applyService( walker );

  HouseList houses = reachedBuildings.select<House>().toList();
  HousePtr illHouse = utils::withMinParam( houses, pr::health );

  unsigned int lowHealth = utils::objectState( illHouse, pr::health, 100 );
  if( lowHealth < 30 )
  {
    lowHealth = (100 - lowHealth) / 10;
    houses.where( [] (HousePtr h) { return h->state( pr::health ) > 10; })
          .for_each( [lowHealth] (HousePtr h) { return h->updateState( pr::health, -lowHealth ); } );
  }
}

bool Well::isNeedRoad() const {  return false; }
void Well::burn() { collapse(); }
bool Well::isDestructible() const{  return true; }

std::string Well::sound() const
{
  return ServiceBuilding::sound();
}

Variant Well::getProperty(const std::string & name) const
{
  if (name == "coverageHouse") {
    auto area = coverageArea();
    bool haveHouseInArea = false;
    for (auto tile : area) {
      haveHouseInArea |= tile->overlay().is<House>();
    }
    return haveHouseInArea;
  }
  if (name == "housesNeedWell") {
    bool housesNeedWell = false;
    auto houses = coverageArea().overlays().select<House>();
    for (auto& house : houses) {
      housesNeedWell |= (house->getServiceValue(Service::fountain) == 0);
    }
    return housesNeedWell;
  }
  if (name == "lowHealthHouseNumber") {
    auto houses = coverageArea().overlays<House>();
    int lowHealthHouse = houses.count([](HousePtr h) { return h->state(pr::health) < 10; });
    return lowHealthHouse;
  }

  return ServiceBuilding::getProperty(name);
}

bool Well::build( const city::AreaInfo& areainfo )
{
  ServiceBuilding::build( areainfo );

  Picture rpic = info().randomPicture( size() )
                       .withFallback( ResourceGroup::utilitya, 1 );

  setPicture( rpic );

  setState( pr::inflammability, 0 );
  setState( pr::collapsibility, 0 );
  return true;
}

TilesArea Well::coverageArea() const
{
  TilesArea ret( _map(), wellServiceRange, pos() );
  return ret;
}
