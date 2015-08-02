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
// Copyright 2012-2014 dalerank, dalerankn8@gmail.com

#include "immigrant.hpp"
#include "core/position.hpp"
#include "objects/road.hpp"
#include "gfx/animation_bank.hpp"
#include "gfx/cart_animation.hpp"
#include "city/city.hpp"
#include "constants.hpp"
#include "city/statistic.hpp"
#include "helper.hpp"
#include "corpse.hpp"
#include "core/variant_map.hpp"
#include "walkers_factory.hpp"
#include "game/resourcegroup.hpp"

using namespace gfx;
using namespace city;

REGISTER_CLASS_IN_WALKERFACTORY(walker::immigrant, Immigrant)

enum class ThinksAboutFood : unsigned short
{
   nothing = 0,
   onemonth,
   somemonth,
   much,
   count
};

static const std::string ThinksAboutFoodDesc[] = {
  "##immigrant_nofood_here##",
  "##immigrant_onemonthfood_here##",
  "##immigrant_somefood_here##",
  "##immigrant_much_food_here##",
  ""
};

Immigrant::Immigrant( PlayerCityPtr city ) : Emigrant( city )
{
  CitizenGroup peoples;
  peoples[ CitizenGroup::matureMin ] = 2;
  peoples[ CitizenGroup::childMin ] = 2;
  setPeoples( peoples );

  _setType( walker::immigrant );
}

CartAnimation& Immigrant::_cart()
{
  if( !Emigrant::_cart().isValid() )
  {
    Emigrant::_cart().load( AnimationBank::animImmigrantCart + G_EMIGRANT_CART1, direction() );
  }

  return Emigrant::_cart();
}

void Immigrant::getPictures( Pictures& oPics)
{
  oPics.clear();

  // depending on the walker direction, the cart is ahead or behind
  switch (direction())
  {
  case direction::north:
  case direction::northEast:
  case direction::northWest:
  case direction::west:
    oPics.push_back( getMainPicture() );
    oPics.push_back( _cart().currentFrame() );
  break;

  case direction::southWest:
  case direction::southEast:
  case direction::east:
  case direction::south:
    oPics.push_back( _cart().currentFrame() );
    oPics.push_back( getMainPicture() );
  break;

  default:
  break;
  }
}

void Immigrant::_changeDirection()
{
  Emigrant::_changeDirection();
  _setCart( CartAnimation() );  // need to get the new graphic
}

void Immigrant::_updateThoughts()
{
  StringArray thinks;
  thinks << "##immigrant_where_my_home##";
  thinks << "##immigrant_want_to_be_liontamer##";

  int fstock = _city()->statistic().food.inGranaries();
  int mconsumption = _city()->statistic().food.monthlyConsumption();
  int index = math::clamp<int>( fstock / (mconsumption+1), (int)ThinksAboutFood::nothing, (int)ThinksAboutFood::much );
  thinks << ThinksAboutFoodDesc[ index ];

  setThinks( thinks.random() );
}

void Immigrant::timeStep(const unsigned long time)
{  
  _cart().update( time );
  Walker::timeStep(time);
}

bool Immigrant::die()
{
  bool created = Walker::die();

  if( !created )
  {
    Corpse::create( _city(), pos(), ResourceGroup::citizen1, 1129, 1136 );
    return true;
  }

  return false;
}

ImmigrantPtr Immigrant::create(PlayerCityPtr city )
{
  ImmigrantPtr newEmigrant( new Immigrant( city ) );
  newEmigrant->initialize( WalkerHelper::getOptions( walker::immigrant ) );
  newEmigrant->drop();
  return newEmigrant;
}

Immigrant::~Immigrant(){}
