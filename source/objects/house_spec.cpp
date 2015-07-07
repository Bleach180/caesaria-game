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

#include "house_spec.hpp"

#include "objects/house.hpp"
#include "core/exception.hpp"
#include "core/utils.hpp"
#include "core/variant_map.hpp"
#include "core/saveadapter.hpp"
#include "good/store.hpp"
#include "core/foreach.hpp"
#include "city/statistic.hpp"
#include "good/helper.hpp"
#include "gfx/tilemap.hpp"
#include "core/logger.hpp"
#include "game/gamedate.hpp"
#include "game/resourcegroup.hpp"

#include <string>
#include <map>
#include <list>

using namespace gfx;

class HouseSpecification::Impl
{
public:
  int houseLevel;
  int tileCapacity;
  std::string levelName;
  std::string internalName;
  unsigned int srvcInterval, goodInterval, foodInterval;
 
  int taxRate;

  // required services
  int minEntertainmentLevel;
  int minHealthLevel;
  int minDesirability, maxDesirability;
  int minEducationLevel;
  int crime;
  int prosperity;
  int minWaterLevel;  // access to water (no water=0, well=1, fountain=2)
  int minReligionLevel;  // number of religions
  int minFoodLevel;  // number of food types

  typedef std::map<good::Product, int> RequiredGoods;
  RequiredGoods requiredGoods;  // rate of good usage for every good (furniture, pottery, ...)

  typedef std::map<good::Product, float> GoodConsumptionMuls;
  GoodConsumptionMuls consumptionMuls;
};

int HouseSpecification::level() const {   return _d->houseLevel;}
const std::string& HouseSpecification::levelName() const{   return _d->levelName;}
bool HouseSpecification::isPatrician() const{   return _d->houseLevel >= HouseLevel::smallVilla;}
int HouseSpecification::tileCapacity() const{   return _d->tileCapacity; }
int HouseSpecification::taxRate() const{   return _d->taxRate;}
int HouseSpecification::minEntertainmentLevel() const{  return _d->minEntertainmentLevel;}
int HouseSpecification::minEducationLevel() const{  return _d->minEducationLevel;}
int HouseSpecification::minHealthLevel() const {  return _d->minHealthLevel; }
int HouseSpecification::minReligionLevel() const{  return _d->minReligionLevel;}
//
// int HouseLevelSpec::getMinWaterLevel()
// {
//    return _minWaterLevel;
// }
//
int HouseSpecification::minFoodLevel() const{  return _d->minFoodLevel;}

bool HouseSpecification::checkHouse( HousePtr house, std::string* retMissing,
                                     object::Type* retBtype, TilePos* retPos ) const
{
  bool res = true;
  int value;
  std::string reason;
  std::string defaultStr;
  object::Type defaultNeedType;
  TilePos defaultPos;

  std::string& ref = retMissing ? *retMissing : defaultStr;
  object::Type& needBuilding = retBtype ? *retBtype : defaultNeedType;
  TilePos& rPos = retPos ? *retPos : defaultPos;

  needBuilding = object::unknown;

  if( house->habitants().count() == 0 )
  {
    ref = "##house_no_citizen##";
    return false;
  }

  value = computeDesirabilityLevel( house, reason );
  if( house->isCheckedDesirability() && value < _d->minDesirability )
  {
    ref = "##low_desirability##";
    needBuilding = object::garden;
    return false;
  }

  value = computeEntertainmentLevel( house );
  if( value < _d->minEntertainmentLevel )
  {
    if( value == 0 )
    {
      ref = "##missing_entertainment##";
      needBuilding = object::theater;
    }
    else
    {
      switch( _d->minEntertainmentLevel / 20 )
      {
      case 0: ref = "##missing_entertainment_theater##"; needBuilding = object::theater; break;
      case 1: ref = "##missing_entertainment_amph##"; needBuilding = object::amphitheater; break;
      case 2: ref = "##missing_entertainment_also##"; needBuilding = object::amphitheater; break;
      case 3: ref = "##missing_entertainment_colloseum##"; needBuilding = object::colloseum; break;
      case 4: ref = "##missing_entertainment_hippodrome##"; needBuilding = object::hippodrome; break;
        //##missing_entertainment_patrician##
      }
    }
    return false;
  }

  value = computeEducationLevel( house, reason );
  if( value < _d->minEducationLevel )
  {
    ref = /*_("##missing_education##") + */reason;
    switch( value )
    {
    case 0: case 1: needBuilding = object::school; break;
    case 2: needBuilding = object::academy; break;
    case 3: needBuilding = object::library; break;
    }

    return false;
  }

  value = computeHealthLevel( house, reason );
  if( value < _d->minHealthLevel )
  {
    ref = /*_( "##missing_health##" ) + */reason;

    switch( value )
    {
    case 1: needBuilding = object::baths; break;
    case 2: needBuilding = object::barber; break;
    case 3: needBuilding = object::clinic; break;
    case 4: needBuilding = object::hospital; break;
    }

    return false;
  }

  value = computeReligionLevel( house );
  if( value < _d->minReligionLevel )
  {
    switch( _d->minReligionLevel )
    {
    case 0: ref = "##religion_undef_reason##"; break;
    case 1: ref = "##missing_religion##"; break;
    case 2: ref = "##missing_second_religion##"; break;
    case 3: ref = "##missing_third_religion##"; break;
    }
    needBuilding = object::oracle;
    return false;
  }

  value = computeWaterLevel(house, reason);
  if( value < _d->minWaterLevel )
  {
    ref = reason;
    needBuilding = object::fountain;
    return false;
  }

  value = computeFoodLevel(house);
  if( value < _d->minFoodLevel )
  {
    if( !house->hasServiceAccess( Service::market ) ) { ref = "##missing_market##"; }
    else
    {
      switch( _d->minFoodLevel )
      {
      case 1: ref = "##missing_food##"; break;
      case 2: ref = "##missing_second_food##"; break;
      case 3: ref = "##missing_third_food##"; break;
      }
    }
    needBuilding = object::market;
    return false;
  }

  if( _d->requiredGoods[good::pottery] != 0 && house->goodStore().qty(good::pottery) == 0)
  {
    ref = "##missing_pottery##";
    needBuilding = object::pottery_workshop;
    return false;
  }

  if( _d->requiredGoods[good::furniture] != 0 && house->goodStore().qty(good::furniture) == 0)
  {
    ref = "##missing_furniture##";
    needBuilding = object::furniture_workshop;
    return false;
  }

  if( _d->requiredGoods[good::oil] != 0 && house->goodStore().qty(good::oil) == 0)
  {
    ref = "##missing_oil##";
    needBuilding = object::oil_workshop;
    return false;
  }

  if( _d->requiredGoods[good::wine] != 0 && house->goodStore().qty(good::wine) == 0)
  {
    ref = "##missing_wine##";
    needBuilding = object::wine_workshop;
    return false;
  }

  if( _d->requiredGoods[good::prettyWine] != 0 && house->goodStore().qty(good::prettyWine) == 0)
  {
    ref = "##missing_second_wine##";
    needBuilding = object::wine_workshop;
    return false;
  }

  return res;
}

unsigned int HouseSpecification::consumptionInterval(HouseSpecification::IntervalName name) const
{
  switch( name )
  {
  case intv_service: return _d->srvcInterval;
  case intv_foods: return _d->foodInterval;
  case intv_goods: return _d->goodInterval;

  default: break;
  }

  Logger::warning( "WARNING !!! Unknown interval name %d", name );
  return 0;
}

int HouseSpecification::findUnwishedBuildingNearby(HousePtr house, object::Type& rType, TilePos& refPos ) const
{
  int aresOffset = math::clamp<int>( house->spec().level() / 5, 1, 10 );
  TilePos offset( aresOffset, aresOffset );
  TilePos housePos = house->pos();
  int houseDesrbl = house->desirability().base;
  BuildingList buildings = city::statistic::getObjects<Building>( house->_city(), object::any, housePos - offset, housePos + offset );

  int ret = 0;
  foreach( it, buildings )
  {
    int desValue = (*it)->desirability().base;
    if( desValue < 0 && houseDesrbl > desValue && abs( houseDesrbl - desValue ) > 1 )
    {
      ret = 1;
      refPos = (*it)->pos();
      rType = (*it)->type();
      break;
    }
  }

  return ret;
}

int HouseSpecification::findLowLevelHouseNearby(HousePtr house, TilePos& refPos ) const
{
  int aresOffset = math::clamp<int>( house->spec().level() / 5, 1, 10 );
  TilePos offset( aresOffset, aresOffset );
  TilePos housePos = house->pos();
  HouseList houses = city::statistic::getObjects<House>( house->_city(), object::house, housePos - offset, housePos + offset );

  int ret = 0;
  foreach( it, houses )
  {
    int pop = (*it)->habitants().count();
    int bLevel = (*it)->spec().level();
    if( pop > 0 && (_d->houseLevel - bLevel > 2) )
    {
      ret = 1;
      refPos = (*it)->pos();
      break;
    }
  }

  return ret;
}

int HouseSpecification::computeWaterLevel(HousePtr house, std::string &oMissingRequirement) const
{
  // no water=0, well=1, fountain=2
  int res = 0;
  if (house->hasServiceAccess(Service::fountain))
  {
    res = 2;
  }
  else if (house->hasServiceAccess(Service::well))
  {
    res = 1;
    oMissingRequirement = "##missing_fountain##";
  }
  else
  {
    oMissingRequirement = "##missing_water##";
  }
  return res;
}


int HouseSpecification::computeFoodLevel(HousePtr house) const
{
  int res = 0;

  const good::Store& goodStore = house->goodStore();
  res += goodStore.qty(good::wheat) > 0 ? 1 : 0;
  res += goodStore.qty(good::fish) > 0 ? 1 : 0;
  res += goodStore.qty(good::meat) > 0 ? 1 : 0;
  res += goodStore.qty(good::fruit) > 0 ? 1 : 0;
  res += goodStore.qty(good::vegetable) > 0 ? 1 :0;

  return res;
}


int HouseSpecification::computeEntertainmentLevel(HousePtr house) const
{
   int res = 0;
   res += house->hasServiceAccess(Service::theater) ? 10 : 0;
   res += house->hasServiceAccess(Service::amphitheater) ? 20 : 0;
   res += house->hasServiceAccess(Service::colloseum) ? 30 : 0;
   res += house->hasServiceAccess(Service::hippodrome) ? 40 : 0;
   return res;
}


int HouseSpecification::computeHealthLevel(HousePtr house, std::string &oMissingRequirement) const
{
   // no health=0, bath=1, bath+doctor/hospital=2, bath+doctor/hospital+barber=3, bath+doctor+hospital+barber=4
   int res = 0;
   if (house->hasServiceAccess(Service::baths))
   {
      res = 1;

      if (house->hasServiceAccess(Service::doctor) || house->hasServiceAccess(Service::hospital))
      {
         res = 2;

         if (house->hasServiceAccess(Service::barber))
         {
           res = 3;

           if (house->hasServiceAccess(Service::doctor) && house->hasServiceAccess(Service::hospital))
           {
              res = 4;
           }
           else
           {
             if (house->hasServiceAccess(Service::doctor))
             {
               oMissingRequirement = "##missing_hospital##";
             }
             else
             {
               oMissingRequirement = "##missing_doctor##";
             }
           }
         }
         else
         {
           oMissingRequirement = "##missing_barber##";
         }
      }
      else
      {
        oMissingRequirement = "##missing_doctor_or_hospital##";
      }
   }
   else
   {
     oMissingRequirement = "##missing_bath##";
   }
   return res;
}


int HouseSpecification::computeEducationLevel(HousePtr house, std::string &oMissingRequirement) const
{
  int res = 0;
  bool haveSchool = house->hasServiceAccess(Service::school);
  bool haveAcademy = house->hasServiceAccess(Service::academy);
  bool haveLibrary = house->hasServiceAccess(Service::library);
  if( haveSchool )
  {
    res = 1;   
    if( haveAcademy )
    {
      res = 2;      
      if( haveLibrary )
      {
        res = 3;
      }
      else
      {
        oMissingRequirement = "##missing_library##";
      }
    }
    else
    {
      oMissingRequirement = "##missing_college##";
    }
  }
  else
  {
     oMissingRequirement = haveLibrary
                             ? "##missing_school##"
                             : "##missing_school_or_library##";
  }

  return res;
}

int HouseSpecification::computeReligionLevel(HousePtr house) const
{
  int res = 0;
  res += house->hasServiceAccess(Service::religionMercury) ? 1 : 0;
  res += house->hasServiceAccess(Service::religionVenus) ? 1 : 0;
  res += house->hasServiceAccess(Service::religionMars) ? 1 : 0;
  res += house->hasServiceAccess(Service::religionNeptune) ? 1 : 0;
  res += house->hasServiceAccess(Service::religionCeres) ? 1 : 0;
  return res;
}

float HouseSpecification::evaluateServiceNeed(HousePtr house, const Service::Type service)
{
   float res = 0;

   switch (service)
   {
   case Service::religionCeres:
   case Service::religionNeptune:
   case Service::religionMars:
   case Service::religionVenus:
   case Service::religionMercury:
      res = evaluateReligionNeed(house, service);
      break;
   case Service::theater:
   case Service::amphitheater:
   case Service::colloseum:
   case Service::hippodrome:
      res = evaluateEntertainmentNeed(house, service);
      break;
   case Service::school:
   case Service::library:
   case Service::academy:
      res = evaluateEducationNeed(house, service);
      break;
   case Service::baths:
   case Service::barber:
   case Service::doctor:
   case Service::hospital:
      res = evaluateHealthNeed(house, service);
      break;
   case Service::forum:
      res = 1;
      break;
   default:
      break;
   }

   return res * (100 - house->getServiceValue(service));
}

float HouseSpecification::evaluateEntertainmentNeed(HousePtr house, const Service::Type service)
{
   //int houseLevel = house.getLevelSpec().getHouseLevel();
   return (float)next()._d->minEntertainmentLevel;
}

float HouseSpecification::evaluateEducationNeed(HousePtr house, const Service::Type service)
{
  float res = 0;
  //int houseLevel = house.getLevelSpec().getHouseLevel();
  int minLevel = next()._d->minEducationLevel;
  switch( minLevel )
  {
  case 1:  // need school
    res = (service == Service::school
            ? (float)( 100 - house->getServiceValue(service) )
            : 0);
  break;

  case 2: // need school and academy
    res = (service != Service::library
            ? (float)( 100 - house->getServiceValue(service) )
            : 0);
  break;

  case 3: // need school and library and college
     res = (float)( 100 - house->getServiceValue(service) );
  break;
  }
  // std::cout << "education need: " << service << " " << res << std::endl;
  return res;
}

float HouseSpecification::evaluateHealthNeed(HousePtr house, const Service::Type service)
{
   float res = 0;
   //int houseLevel = house.getLevelSpec().getHouseLevel();
   int minLevel = next()._d->minHealthLevel;
   if (minLevel >= 1 && service == Service::baths)
   {
      // minLevel>=1  => need baths
      res = (float)( 100 - house->getServiceValue(service) );
   }

   if (minLevel >= 2 && (service == Service::doctor || service == Service::hospital))
   {
      if (minLevel == 4)
      {
         // need doctor and hospital
         res = (float)( 100 - house->getServiceValue(service) );
      }
      else
      {
         // need doctor or hospital
         res = (float)( 100 - std::max(house->getServiceValue(Service::doctor), house->getServiceValue(Service::hospital)) );
      }
   }

   if (minLevel >= 3 && service == Service::barber)
   {
      // minLevel>=3  => need barber
      res = (float)( 100 - house->getServiceValue(service) );
   }

   return std::max<float>( res, 100 - house->state( pr::health ) );
}

float HouseSpecification::evaluateReligionNeed(HousePtr house, const Service::Type service)
{
   //int houseLevel = house.getLevelSpec().getHouseLevel();
   int minLevel = next()._d->minReligionLevel;

   return (float)minLevel;
}

int HouseSpecification::minDesirabilityLevel() const { return _d->minDesirability; }
int HouseSpecification::maxDesirabilityLevel() const { return _d->maxDesirability; }

int HouseSpecification::computeMonthlyGoodConsumption( HousePtr house, const good::Product goodType, bool real) const
{
  if( house.isNull() )
  {
    Logger::warning( "HouseLevelSpec::computeMonthlyConsumption parameter is null" );
    return 0;
  }

  int res=0;
  if( goodType == good::furniture || goodType == good::oil ||
      goodType == good::pottery  || goodType ==  good::wine )
  {
    res = 2;
  }
  else if( goodType == good::wheat || goodType ==  good::meat ||
           goodType == good::fish || goodType == good::fruit ||
           goodType == good::vegetable )
  {
    res = house->habitants().count() / 2;
  }
  else
  {
     res = 0;
  }

  res *= (real ? _d->consumptionMuls[ goodType ] : 1);

  return (res * _d->requiredGoods[goodType]);
}

int HouseSpecification::computeMonthlyFoodConsumption(HousePtr house) const
{
  if( house.isNull() )
  {
    Logger::warning( "HouseLevelSpec::computeMonthlyFoodConsumption parameter is null" );
    return 0;
  }

  return house->habitants().count() / 2;
}

const std::string& HouseSpecification::internalName() const{  return _d->internalName; }
int HouseSpecification::getRequiredGoodLevel(good::Product type) const{  return _d->requiredGoods[type];}
int HouseSpecification::prosperity() const{  return _d->prosperity;}
int HouseSpecification::crime() const{  return _d->crime;}

HouseSpecification::~HouseSpecification() {}

HouseSpecification::HouseSpecification() : _d( new Impl )
{
  _d->srvcInterval = game::Date::days2ticks( 2 );
  _d->foodInterval = game::Date::days2ticks( 30 );
  _d->goodInterval = game::Date::days2ticks( 15 );
}

HouseSpecification::HouseSpecification( const HouseSpecification& other ) : _d( new Impl )
{
  *this = other;
}

HouseSpecification HouseSpecification::next() const
{
  return HouseSpecHelper::instance().getSpec(_d->houseLevel+1);
}

int HouseSpecification::computeDesirabilityLevel(HousePtr house, std::string& oMissingRequirement) const
{
  PlayerCityPtr city = house->_city();

  TilesArea area( city->tilemap(), house->pos() - TilePos( 2, 2 ), house->size() + Size( 4 ) );

  float middleDesirbl = 0;;

  foreach( tile, area )
    middleDesirbl += (float)(*tile)->param( Tile::pDesirability );

  if( !area.empty() )
    middleDesirbl /= area.size();

  return (int)middleDesirbl;
}

HouseSpecification& HouseSpecification::operator=( const HouseSpecification& other )
{
  _d->houseLevel = other._d->houseLevel;
  _d->tileCapacity = other._d->tileCapacity;
  _d->levelName = other._d->levelName;
  _d->internalName = other._d->internalName;
  _d->taxRate = other._d->taxRate;

  // required services
  _d->minEntertainmentLevel = other._d->minEntertainmentLevel;
  _d->minHealthLevel = other._d->minHealthLevel;
  _d->minDesirability = other._d->minDesirability;
  _d->maxDesirability = other._d->maxDesirability;
  _d->minEducationLevel = other._d->minEducationLevel;
  _d->crime = other._d->crime;
  _d->prosperity = other._d->prosperity;
  _d->minWaterLevel = other._d->minWaterLevel;  // access to water (no water=0, well=1, fountain=2)
  _d->minReligionLevel = other._d->minReligionLevel;  // number of religions
  _d->minFoodLevel = other._d->minFoodLevel;  // number of food types
  _d->requiredGoods = other._d->requiredGoods;
  _d->consumptionMuls = other._d->consumptionMuls;

  return *this;
}

class HouseSpecHelper::Impl
{
public:
  typedef std::map<int, HouseSpecification > HouseLevels;
  typedef std::map<std::string, StringArray > HouseTextures;

  HouseLevels levels;
  HouseTextures houseTextures;
};

HouseSpecHelper& HouseSpecHelper::instance()
{
  static HouseSpecHelper inst;
  return inst;
}

HouseSpecHelper::HouseSpecHelper() : _d( new Impl )
{
  Logger::warning( "HouseLevelSpec INIT" );
}

HouseSpecification HouseSpecHelper::getSpec(const int houseLevel)
{
  int level = math::clamp<int>(houseLevel, 0, 20);
  return _d->levels[level];
}

int HouseSpecHelper::getLevel( const std::string& name )
{
  foreach( item, _d->levels )
  {
    if( item->second.internalName() == name )
    {
      return item->second.level();
    }
  }

  return 0;
}

HouseSpecHelper::~HouseSpecHelper(){}

void HouseSpecHelper::initialize( const vfs::Path& filename )
{
  VariantMap houseSpecs = config::load( filename );

  if( houseSpecs.empty() )
  {
    Logger::warning( "Can't load house model from " + filename.toString() );
    return;
  }

  foreach( item, houseSpecs )
  {
    // this is not a comment (comments start by #)
    // std::cout << "Line #" << linenum << ":" << line << std::endl;
    VariantMap hSpec = item->second.toMap();

    HouseSpecification spec;
    spec._d->houseLevel = hSpec[ "level" ];
    spec._d->internalName = item->first;
    spec._d->levelName = hSpec[ "title" ].toString();
    spec._d->tileCapacity = hSpec.get( "habitants" ).toInt();
    spec._d->minDesirability = hSpec.get( "minDesirability" ).toInt();  // min desirability
    spec._d->maxDesirability = hSpec.get( "maxDesirability" ).toInt();  // desirability levelUp
    spec._d->minEntertainmentLevel = hSpec.get( "entertainment" ).toInt();
    spec._d->minWaterLevel = hSpec.get( "water" ).toInt();
    spec._d->minReligionLevel = hSpec.get( "religion" ).toInt();
    spec._d->minEducationLevel = hSpec.get( "education" ).toInt();
    spec._d->minHealthLevel = hSpec.get( "health" ).toInt();
    spec._d->minFoodLevel = hSpec.get( "food" ).toInt();
    
    spec._d->requiredGoods[good::wheat] = 1;  // hard coded ... to be changed!
    spec._d->requiredGoods[good::fish] = 1;
    spec._d->requiredGoods[good::meat] = 1;
    spec._d->requiredGoods[good::fruit] = 1;
    spec._d->requiredGoods[good::vegetable] = 1;
    spec._d->requiredGoods[good::pottery] = hSpec.get( "pottery" ).toInt();  // pottery
    spec._d->requiredGoods[good::oil] = hSpec.get( "oil" ).toInt();  // oil
    spec._d->requiredGoods[good::furniture] = hSpec.get( "furniture").toInt();// furniture
    spec._d->requiredGoods[good::wine] = hSpec.get( "wine" ).toInt();  // wine
    spec._d->crime = hSpec.get( "crime" ).toInt();  // crime
    spec._d->prosperity = hSpec.get( "prosperity" ).toInt();  // prosperity
    spec._d->taxRate = hSpec.get( "tax" ).toInt();// tax_rate

    foreach(i, good::all() )
    {
      spec._d->consumptionMuls[ *i ] = 1;
    }

    //load consumption goods koefficient
    VariantMap varConsumptions = hSpec.get( "consumptionkoeff" ).toMap();
    foreach( v, varConsumptions )
    {
      spec._d->consumptionMuls[ good::Helper::getType( v->first ) ] = (float)v->second;
    }

    VariantMap vmTextures = hSpec.get( "txs" ).toMap();
    foreach( it, vmTextures )
    {
      std::string arName = utils::format( 0xff, "h%d_%s", spec._d->houseLevel, it->first.c_str() );
      StringArray txNames = it->second.toStringArray();

      StringArray& hSizeTxs = _d->houseTextures[ arName ];
      foreach( tx, txNames )
      {
        Picture pic( *tx );
        if( pic.isValid() )
        {
          hSizeTxs.push_back( *tx );
        }
      }
    }

    _d->levels[ spec._d->houseLevel ] = spec;
  }
}

Picture HouseSpecHelper::getPicture( int houseLevel, int size ) const
{
  std::string arName = utils::format( 0xff, "h%d_s%d", houseLevel, size );
  StringArray& array = _d->houseTextures[ arName ];

  if( !array.empty() )
  {
    return Picture( array.random() );
  }

  return Picture::getInvalid();
}
