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

#include "computer_city.hpp"
#include "empire.hpp"
#include "trading.hpp"
#include "good/storage.hpp"
#include "good/helper.hpp"
#include "game/gamedate.hpp"
#include "core/foreach.hpp"
#include "merchant.hpp"
#include "game/funds.hpp"
#include "game/resourcegroup.hpp"
#include "empiremap.hpp"
#include "game/player.hpp"
#include "world/barbarian.hpp"
#include "core/metric.hpp"
#include "objects/metadata.hpp"
#include "core/variant_map.hpp"
#include "city/states.hpp"
#include "objects/constants.hpp"
#include "game/citizen_group.hpp"
#include "core/priorities.hpp"
#include "config.hpp"

using namespace gfx;
using namespace metric;

class CcTargets
{
public:

  int population;
  unsigned int need[ object::userType ];
  unsigned int idle[ object::userType ];
  unsigned int coverage[ object::userType ];

  CcTargets()
  {
    population = 0;
    int dd = sizeof( need );
    memset( need, 0, sizeof( need ) );
    memset( idle, 0, sizeof( need ) );
    memset( coverage, 0, sizeof( need ) );
  }

  VariantMap save() const
  {
    return VariantMap();
  }

  void load( const VariantMap& vm )
  {

  }
};

class CcStorage : public good::Storage
{
public:
  bool needExpand;

  CcStorage()
  {
    needExpand = 0;
  }

  VariantMap save() const
  {
    VariantMap ret;
    foreach( gtype, good::all() )
    {
      std::string tname = good::Helper::getTypeName( *gtype );
      int ncapacity = capacity( *gtype );
      if( ncapacity > 0 )
      {
        ncapacity = Unit::fromQty( ncapacity ).ivalue();
        int value = Unit::fromQty( qty( *gtype ) ).ivalue();
        Point p( value, ncapacity );

        ret[ tname ] = p;
      }
    }
    return ret;
  }

  virtual void store( good::Stock& stock, const int amount)
  {
    if( freeQty() < stock.qty() )
      needExpand++;

    good::Storage::store( stock, amount );
  }

  void load( const VariantMap& stream )
  {
    foreach( it, stream )
    {
      good::Product gtype = good::Helper::getType( it->first );
      Variant value = it->second;
      switch( value.type() )
      {
        case Variant::Int:
        case Variant::UInt:
        {
          setCapacity( gtype, Unit::fromValue( it->second ).toQty() );
        }
        break;

        case Variant::List:
        {
          Point p = value.toPoint();
          setCapacity( gtype, Unit::fromValue( p.y() ).toQty() );
          setQty( gtype, Unit::fromValue( p.x() ).toQty() );
        }
        break;

        default: break;
      }
    }
  }
};

struct PriceInfo
{
  good::Product product;
  int buy;
  int sell;

  bool operator<(const PriceInfo& a)
  {
    return product < a.product;
  }
};

class Prices : public std::set<PriceInfo>
{
public:
};

struct BuildingInfo
{
  object::Type type;
  int maxWorkersNumber;
  int workersNumber;
  int progress, productively;
  int maxService;
  bool producing;
  good::Stock ingoods, outgoods;

  BuildingInfo()
  {
    type = object::unknown;
    progress = 0;
    maxService = 0;
    producing = false;
    productively = 50;
  }

  void threatGoods( CcStorage& storage )
  {
    if( workersNumber == 0 )
      return;

    if( ingoods.type() != good::none )
    {
      if( !producing )
      {
        if( storage.qty( ingoods.type() ) >= 100 )
        {
          good::Stock stock( ingoods.type(), 100, 100 );
          storage.retrieve( stock, 100 );
          producing = true;
        }
      }
    }
    else
      producing = true;

    if( producing )
    {
      float workersKoeff = workersNumber / maxWorkersNumber;

      progress += productively * workersKoeff;

      if( progress >= 100 )
      {
        if( outgoods.type() != good::none && outgoods.qty() > 0 )
        {
          good::Stock stock( outgoods.type(), 100, 100 );
          storage.store( stock, 100 );
        }

        progress -= 100;
      }
    }
  }
};

class Buildings : public std::vector<BuildingInfo> 
{
public:
  int count( object::Type type )
  {
    int ret = 0;
    foreach( it, *this )
      ret += (it->type == type ? 1 : 0);

    return ret;
  }

  VariantMap save() const
  {
    return VariantMap();
  }

  void load( const VariantMap& vm )
  {

  }
};

namespace world
{

class ComputerCity::Impl
{
public:
  unsigned int tradeType;
  bool distantCity;
  bool available;
  int strength;
  City::AiMode modeAI;
  int sentiment;
  Buildings buildings;
  CitizenGroup peoples;
  CcTargets targets;
  Prices prices;

  city::States states;
  unsigned int tradeDelay;

  CcStorage sells;
  good::Storage realSells;
  CcStorage buys;
  CcStorage internalGoods;

  DateTime lastTimeUpdate;
  DateTime lastTimeMerchantSend;
  DateTime lastAttack;

  unsigned int merchantsNumber;
  econ::Treasury funds;

public:
  void initPeoples();
  void calculateMonthState();
  void calcPopulationChange();
  void citizensConsumeGoods();
  void placeNewBuildings();
  void updateWorkersInBuildings();
  void placeNewBuilding( object::Type type );
  void citizensConsumeServices();
  void updateWorkingWarehouse();
};

void ComputerCity::Impl::calculateMonthState()
{
  if( modeAI == City::inactive )
    return;

  updateWorkersInBuildings();
  updateWorkingWarehouse();
  citizensConsumeGoods();
  citizensConsumeServices();

  int idleBuildings = 0;
  foreach( it, buildings )
  {
    BuildingInfo& info = *it;
    info.threatGoods( internalGoods );
    if( info.maxWorkersNumber > 0 && info.workersNumber == 0 )
      idleBuildings++;
  }

  if( idleBuildings == 0 )
    placeNewBuildings();
}

void ComputerCity::Impl::updateWorkersInBuildings()
{
  int workersCount = peoples.count( CitizenGroup::mature );

  int workersNeed = 0;
  foreach( it, buildings )
  {
    it->workersNumber = 0;
    workersNeed += it->maxWorkersNumber;
  }

  float koeffLabor = workersCount / (float)workersNeed;
  foreach( it, buildings )
  {
    if( workersCount <= 0 )
      break;

    if( it->maxWorkersNumber > 0 )
    {
      int workers = ceil( it->maxWorkersNumber * koeffLabor );
      workers = math::clamp( workers, 0, workersCount );
      it->workersNumber = workers;
      workersCount -= workers;
    }
  }
}

void ComputerCity::Impl::placeNewBuilding(object::Type type)
{
  BuildingInfo info;

  info.type = type;
  const  MetaData& md = MetaDataHolder::getData( info.type );
  good::Product output = good::Helper::getType( md.getOption( "output" ).toString() );
  info.outgoods.setType( output );
  info.ingoods.setType( good::getMaterial( output ) );
  info.maxWorkersNumber = md.getOption( "employers" );
  info.maxService = md.getOption( "maxServe" );
  info.workersNumber = 0;
  info.progress = 0;
  info.productively = md.getOption( "productRate" ).toFloat() / 12.f;

  buildings.push_back( info );
}

struct ServiceInfo
{
  unsigned int type[object::userType];

  ServiceInfo() { memset( this, 0, sizeof(ServiceInfo) ); }
};

int isNeedNewBuilding( unsigned int idle, unsigned int have, unsigned int served, int multiplier )
{
  if( idle > 0 )
     return 0;

  return (served > have ? 0 : 1) * multiplier;
}

struct CheckType
{
  object::Type building, building2;
  CitizenGroup::Age age;
  int multiplier;

  CheckType( object::Type b, CitizenGroup::Age a, int m ) :
     building( b ), building2( object::unknown ), age( a ), multiplier( m ) {}

  CheckType( object::Type b, object::Type b2, CitizenGroup::Age a, int m ) :
     building( b ), building2( b2 ), age( a ), multiplier( m ) {}

  bool operator<( const CheckType& a) const { return building < a.building; }
};

void ComputerCity::Impl::citizensConsumeServices()
{
  ServiceInfo mayServe;
  ServiceInfo idle;

  foreach( it, buildings )
  {
    BuildingInfo& info = *it;
    idle.type[info.type] &= (info.workersNumber < info.maxWorkersNumber);
    if( info.maxWorkersNumber > 0 )
    {
      mayServe.type[ info.type ] += (info.workersNumber / (float)info.maxWorkersNumber) * info.maxService;
    }
  }

  std::set<CheckType> checkTypes;
  checkTypes << CheckType( object::school, CitizenGroup::scholar, 4 )
             << CheckType( object::academy, CitizenGroup::student, 4 )
             << CheckType( object::library, CitizenGroup::mature, 4 )
             << CheckType( object::small_ceres_temple, object::big_ceres_temple, CitizenGroup::any, 4 )
             << CheckType( object::small_mars_temple, object::big_mars_temple, CitizenGroup::any, 4 )
             << CheckType( object::small_mercury_temple, object::big_mercury_temple, CitizenGroup::any, 4 )
             << CheckType( object::small_neptune_temple, object::big_neptune_temple, CitizenGroup::any, 4 )
             << CheckType( object::small_venus_temple, object::big_venus_temple, CitizenGroup::any, 4 );

  foreach( it, checkTypes )
  {
    const CheckType& check = *it;

    int addServe = 0;
    if( check.building2 )
      addServe = mayServe.type[ check.building2 ];

    unsigned int pop = peoples.count( check.age );
    targets.need[ check.building ] += isNeedNewBuilding( idle.type[ check.building ],
                                                         pop,
                                                         mayServe.type[ check.building ] + addServe, check.multiplier );

    int coverage = 0;
    if( pop > 0 )
      coverage = mayServe.type[ check.building ] * 100 / pop;

    targets.coverage[ check.building ] = math::clamp<unsigned int>( coverage, 0, 100 );
    targets.idle[ check.building ] = idle.type[ check.building ];
  }
}

void ComputerCity::Impl::updateWorkingWarehouse()
{
  int workingWarehouse = 0;
  foreach( it, buildings )
  {
    BuildingInfo& info = *it;
    if( info.type == object::warehouse )
    {
      if( info.workersNumber >= info.maxWorkersNumber / 2 )
        workingWarehouse++;
    }
  }

  int futureCityCapacity = workingWarehouse * 3200;
  int currentGoodsQty = internalGoods.qty();

  if( currentGoodsQty > futureCityCapacity )
  {
    good::Products products;
    products << good::foods() << good::materials();

    std::vector<good::Product> revProducts;
    foreach( it, products )
      revProducts.insert( revProducts.begin(), *it );

    int leftQty = futureCityCapacity;
    foreach( it, revProducts )
    {
      int qty = internalGoods.qty( *it );
      if( leftQty < qty )
      {
        internalGoods.setQty( *it, leftQty );
        break;
      }

      leftQty -= qty;
    }
  }

  internalGoods.setCapacity( futureCityCapacity );
}

void ComputerCity::_checkMerchantsDeadline()
{
  if( _d->lastTimeMerchantSend.monthsTo( game::Date::current() ) > config::trade::minMonthsMerchantSend )
  {
    TraderouteList routes = empire()->tradeRoutes( name() );

    if( routes.empty() )
      return;

    if( !_mayTrade() )
      return;

    _d->lastTimeMerchantSend = game::Date::current();

    if( _d->merchantsNumber >= routes.size() )
    {
      return;
    }

    good::Storage sellGoods, buyGoods;
    sellGoods.setCapacity( Merchant::defaultCapacity );
    buyGoods.setCapacity( Merchant::defaultCapacity );

    foreach( gtype, good::all() )
    {
      buyGoods.setCapacity( *gtype, _d->buys.capacity( *gtype ) );

      //how much space left
      int maxQty = (std::min)( _d->sells.capacity( *gtype ) / 4, sellGoods.freeQty() );

      //we want send merchants to all routes
      maxQty /= routes.size();

      int qty = math::clamp( _d->sells.qty( *gtype ), 0, maxQty );

      //have no goods to sell
      if( qty == 0 )
        continue;

      good::Stock& stock = sellGoods.getStock( *gtype );
      stock.setCapacity( qty );

      //move goods to merchant's storage
      _d->sells.retrieve( stock, qty );
    }

    //send merchants to all routes
    foreach( route, routes )
    {
      _d->merchantsNumber++;
      (*route)->addMerchant( name(), sellGoods, buyGoods );
    }
  }

}

void ComputerCity::Impl::placeNewBuildings()
{
  if( targets.need[ object::wheat_farm ] > 8 )
  {
    object::Type type = object::wheat_farm;
    int wheatFarmNumber = buildings.count( object::wheat_farm );
    int fishFarmNumber = buildings.count( object::wharf );
    if( wheatFarmNumber > fishFarmNumber )
      type = object::wharf;

    placeNewBuilding( type );
    targets.need[ object::wheat_farm ] = 0;
  }

  if( internalGoods.needExpand > 8 )
  {
    placeNewBuilding( object::warehouse );
    internalGoods.needExpand = 0;
  }

  std::set<object::Type> types;
  types << object::vegetable_farm << object::pottery_workshop
        << object::meat_farm << object::fig_farm
        << object::oil_workshop << object::wine_workshop
        << object::furniture_workshop << object::school
        << object::small_ceres_temple << object::small_mars_temple
        << object::small_neptune_temple << object::small_mercury_temple
        << object::small_venus_temple;

  foreach( it, types )
  {
    if( targets.need[ *it ] > 8 )
    {
      placeNewBuilding( *it );
      targets.need[ *it ] = 0;
    }
  }
}

void ComputerCity::Impl::calcPopulationChange()
{
  int currentPopulation = peoples.count();
  int popChange = targets.population - currentPopulation;

  if( popChange > 20 )
    popChange = math::clamp<int>( popChange, 0, currentPopulation * 0.05 );

  if( popChange < 0 )
  {
    peoples.retrieve( abs(popChange) );
  }
  else
  {
    CitizenGroup toAdd = CitizenGroup::random( popChange );
    peoples.include( toAdd );
  }

  states.population = peoples.count();
}

struct GoodInfo
{
  int need;
  int have;
};

void ComputerCity::Impl::citizensConsumeGoods()
{
  int matureCount = peoples.count( CitizenGroup::mature );
  int foodQtyNeed = matureCount * 2;
  foodQtyNeed += peoples.count( CitizenGroup::child );
  foodQtyNeed += peoples.count( CitizenGroup::student );
  foodQtyNeed += peoples.count( CitizenGroup::aged );

  GoodInfo wine2Consume = { (int)(foodQtyNeed * 0.05f), internalGoods.qty( good::wine ) };
  GoodInfo oil2Consume = { (int)(foodQtyNeed * 0.05f), internalGoods.qty( good::oil ) };
  GoodInfo fruit2Consume = { (int)(foodQtyNeed * 0.1f), internalGoods.qty( good::fruit ) };
  GoodInfo meat2Consume = { (int)(foodQtyNeed * 0.15f), internalGoods.qty( good::meat ) };
  GoodInfo vegetable2Consume = { (int)(foodQtyNeed * 0.15f), internalGoods.qty( good::vegetable ) };
  GoodInfo fishWheat2Consume = { (int)(foodQtyNeed * 0.5f), internalGoods.qty( good::wheat ) + internalGoods.qty( good::fish ) };
  GoodInfo pottery2Consume = { (int)(matureCount * 0.1f), internalGoods.qty( good::pottery ) };
  GoodInfo furniture2Consume = { (int)(matureCount * 0.1f), internalGoods.qty( good::furniture ) };

  int targetSentiment = 0;
  bool mayContinue = false;

  if( fishWheat2Consume.need > 0 )
  {
    int fishConsume = math::clamp( fishWheat2Consume.need / 2, 0, internalGoods.qty( good::fish ) );
    good::Stock fishStock( good::fish, fishConsume, fishConsume );
    internalGoods.retrieve( fishStock, fishConsume );

    int wheatConsume = math::clamp( fishWheat2Consume.need - fishConsume, 0, internalGoods.qty( good::wheat ) );
    good::Stock wheatStock( good::wheat, wheatConsume, wheatConsume);
    internalGoods.retrieve( wheatStock, wheatConsume );

    int anyHungry = fishWheat2Consume.need - wheatConsume;
    mayContinue = (anyHungry / (float)fishWheat2Consume.need) < 0.5;
    targetSentiment += 40 * ((fishWheat2Consume.need - anyHungry) / (float)fishWheat2Consume.need);

    if( anyHungry )
      targets.need[ object::wheat_farm ]++;
  }

  if( fishWheat2Consume.have > fishWheat2Consume.need )
  {
    float koeff = fishWheat2Consume.have / (float)fishWheat2Consume.need;
    if( koeff > 1.f )
    {
      koeff = math::clamp( koeff, 1.0f, 1.2f );
      targets.population += targets.population * koeff;
    }
  }
  else
  {
     targets.population -= targets.population * 0.05f;
  }

  if( mayContinue && vegetable2Consume.need > 0 )
  {
    int realyConsume = math::clamp( vegetable2Consume.need, 0, vegetable2Consume.have );
    good::Stock stock( good::vegetable, realyConsume, realyConsume);
    internalGoods.retrieve( stock, realyConsume );
    int anyHungry = vegetable2Consume.need - realyConsume;
    mayContinue = (anyHungry / (float)vegetable2Consume.need) < 0.5;
    targetSentiment += 10 * ((vegetable2Consume.need - anyHungry) / (float)vegetable2Consume.need);

    if( anyHungry )
      targets.need[ object::vegetable_farm ]++;
  }

  if( mayContinue && pottery2Consume.need > 0 )
  {
    int realyConsume = math::clamp( pottery2Consume.need, 0, pottery2Consume.have );
    good::Stock stock( good::pottery, realyConsume, realyConsume);
    internalGoods.retrieve( stock, realyConsume );
    int anyBroken = pottery2Consume.need - realyConsume;
    mayContinue = (anyBroken / (float)pottery2Consume.need) < 0.5;
    targetSentiment += 10 * ((pottery2Consume.need - anyBroken) / (float)pottery2Consume.need);

    if( anyBroken )
      targets.need[ object::pottery_workshop ]++;
  }

  if( mayContinue && meat2Consume.need > 0 )
  {
    int realyConsume = math::clamp( meat2Consume.need, 0, meat2Consume.have );
    good::Stock stock( good::meat, realyConsume, realyConsume);
    internalGoods.retrieve( stock, realyConsume );
    int anyHungry = meat2Consume.need - realyConsume;
    mayContinue = (anyHungry / (float)meat2Consume.need) < 0.5;
    targetSentiment += 10 * ((meat2Consume.need - anyHungry) / (float)meat2Consume.need);

    if( anyHungry )
      targets.need[ object::meat_farm ]++;
  }

  if( mayContinue && fruit2Consume.need > 0 )
  {
    int realyConsume = math::clamp( fruit2Consume.need, 0, fruit2Consume.have );
    good::Stock stock( good::fruit, realyConsume, realyConsume);
    internalGoods.retrieve( stock, realyConsume );
    int anyHungry = fruit2Consume.need - realyConsume;
    mayContinue = (anyHungry / (float)fruit2Consume.need) < 0.5;
    targetSentiment += 10 * ((fruit2Consume.need - anyHungry) / (float)fruit2Consume.need);

    if( anyHungry )
      targets.need[ object::fig_farm ]++;
  }

  if( mayContinue && furniture2Consume.need > 0 )
  {
    int realyConsume = math::clamp( furniture2Consume.need, 0, furniture2Consume.have );
    good::Stock stock( good::furniture, realyConsume, realyConsume);
    internalGoods.retrieve( stock, realyConsume );
    int anyBroken = furniture2Consume.need - realyConsume;
    mayContinue = (anyBroken / (float)furniture2Consume.need) < 0.5;
    targetSentiment += 10 * ((furniture2Consume.need - anyBroken) / (float)furniture2Consume.need);

    if( anyBroken )
      targets.need[ object::furniture_workshop ]++;
  }

  if( mayContinue && oil2Consume.need > 0 )
  {
    int realyConsume = math::clamp( oil2Consume.need, 0, oil2Consume.have );
    good::Stock stock( good::oil, realyConsume, realyConsume);
    internalGoods.retrieve( stock, realyConsume );
    int anyHungry = oil2Consume.need - realyConsume;
    mayContinue = (anyHungry / (float)oil2Consume.need) < 0.5;
    targetSentiment += 5 * ((oil2Consume.need - anyHungry) / (float)oil2Consume.need);

    if( anyHungry )
      targets.need[ object::oil_workshop ]++;
  }

  if( mayContinue && wine2Consume.need > 0 )
  {
    int realyConsume = math::clamp( wine2Consume.need, 0, oil2Consume.have );
    good::Stock stock( good::wine, realyConsume, realyConsume);
    internalGoods.retrieve( stock, realyConsume );
    int anyHungry = wine2Consume.need - realyConsume;
    mayContinue = (anyHungry / (float)wine2Consume.need) < 0.5;
    targetSentiment += 5 * ((wine2Consume.need - anyHungry) / (float)wine2Consume.need);

    if( anyHungry )
      targets.need[ object::wine_workshop ]++;
  }

  int delta = math::clamp( targetSentiment - sentiment, -2, 2);
  sentiment += delta;
}

void ComputerCity::Impl::initPeoples()
{
  int peoplesDelta = states.population - peoples.count();
  CitizenGroup appendGroup;
  appendGroup[ CitizenGroup::child   ] += floor( peoplesDelta * 0.1 );
  appendGroup[ CitizenGroup::student ] += floor( peoplesDelta * 0.2 );
  appendGroup[ CitizenGroup::mature  ] += floor( peoplesDelta * 0.5 );
  appendGroup[ CitizenGroup::aged    ] += floor( peoplesDelta * 0.2 );
  appendGroup[ CitizenGroup::mature  ] += peoplesDelta - appendGroup.count();

  peoples.include( appendGroup );
}

ComputerCity::ComputerCity( EmpirePtr empire, const std::string& name )
  : City( empire ), _d( new Impl )
{
  setName( name );
  _d->tradeDelay = 0;
  _d->distantCity = false;
  _d->merchantsNumber = 0;
  _d->available = true;
  _d->modeAI = City::inactive;
  _d->states.population = 0;
  _d->states.nation = world::nation::unknown;
  _d->sells.setCapacity( 99999 );
  _d->buys.setCapacity( 99999 );
  _d->realSells.setCapacity( 99999 );
  _d->states.age = 0;
  _d->states.romeCity = false;

  _initTextures();
}

bool ComputerCity::_mayTrade() const { return _d->tradeDelay <= 0; }
econ::Treasury& ComputerCity::treasury() { return _d->funds; }
bool ComputerCity::isPaysTaxes() const { return true; }
bool ComputerCity::haveOverduePayment() const { return false; }
City::AiMode ComputerCity::modeAI() const { return _d->modeAI; }
bool ComputerCity::isDistantCity() const{  return _d->distantCity;}
bool ComputerCity::isAvailable() const{  return _d->available;}
void ComputerCity::setAvailable(bool value){  _d->available = value;}
SmartPtr<Player> ComputerCity::mayor() const { return 0; }

void ComputerCity::setModeAI(City::AiMode mode)
{
  _d->modeAI = mode;
  if( mode == indifferent )
  {
    if( _d->peoples.count() == 0 )
    {
      _d->targets.population = 2000;
      _d->peoples += CitizenGroup::random( _d->targets.population );
    }
  }
}

void ComputerCity::save( VariantMap& options ) const
{
  City::save( options );
  _d->states.population = _d->peoples.count();

  VARIANT_SAVE_CLASS_D( options, _d, sells )
  VARIANT_SAVE_CLASS_D( options, _d, buys )
  VARIANT_SAVE_CLASS_D( options, _d, realSells )
  VARIANT_SAVE_CLASS_D( options, _d, peoples )
  VARIANT_SAVE_CLASS_D( options, _d, buildings )
  VARIANT_SAVE_CLASS_D( options, _d, targets )

  options[ "sea" ] = (_d->tradeType & EmpireMap::sea ? true : false);
  options[ "land" ] = (_d->tradeType & EmpireMap::land ? true : false);

  VARIANT_SAVE_ENUM_D( options, _d, modeAI )
  VARIANT_SAVE_ANY_D( options, _d, lastTimeMerchantSend )
  VARIANT_SAVE_ANY_D( options, _d, lastTimeUpdate )
  VARIANT_SAVE_ANY_D( options, _d, states.age )
  VARIANT_SAVE_ANY_D( options, _d, available )
  VARIANT_SAVE_ANY_D( options, _d, merchantsNumber )
  VARIANT_SAVE_ANY_D( options, _d, distantCity )
  VARIANT_SAVE_ANY_D( options, _d, states.romeCity )
  VARIANT_SAVE_ANY_D( options, _d, tradeDelay )
  VARIANT_SAVE_ANY_D( options, _d, lastAttack )  
  VARIANT_SAVE_ANY_D( options, _d, states.population )
  VARIANT_SAVE_ANY_D( options, _d, strength )
  VARIANT_SAVE_ANY_D( options, _d, sentiment )
}

void ComputerCity::load( const VariantMap& options )
{
  City::load( options );

  VARIANT_LOAD_TIME_D  ( _d, lastTimeUpdate,        options )
  VARIANT_LOAD_TIME_D  ( _d, lastTimeMerchantSend,  options )
  VARIANT_LOAD_ANY_D   ( _d, available,             options )
  VARIANT_LOAD_ANY_D   ( _d, merchantsNumber,       options )
  VARIANT_LOAD_ANY_D   ( _d, distantCity,           options )
  VARIANT_LOAD_ANY_D   ( _d, available,             options )
  VARIANT_LOAD_ANY_D   ( _d, states.romeCity,       options )
  VARIANT_LOAD_ANY_D   ( _d, states.age,            options )
  VARIANT_LOAD_ANY_D   ( _d, tradeDelay,            options )
  VARIANT_LOAD_TIME_D  ( _d, lastAttack,            options )
  VARIANT_LOAD_ANY_D   ( _d, strength,              options )
  VARIANT_LOAD_ENUM_D  ( _d, modeAI,                options )
  VARIANT_LOAD_ANYDEF_D( _d, sentiment,         50, options )
  VARIANT_LOAD_ANYDEF_D( _d, states.population, _d->states.population, options )

  foreach( gtype, good::all() )
  {
    _d->sells.setCapacity( *gtype, 0 );
    _d->buys.setCapacity( *gtype, 0 );
    _d->realSells.setCapacity( *gtype, 0 );
  }

  VARIANT_LOAD_CLASS_D( _d, targets,options )
  VARIANT_LOAD_CLASS_D( _d, sells, options )
  VARIANT_LOAD_CLASS_D( _d, buys, options )
  VARIANT_LOAD_CLASS_D( _d, realSells, options )
  VARIANT_LOAD_CLASS_D_LIST( _d, peoples, options )
  VARIANT_LOAD_CLASS_D( _d, buildings, options )

  if( _d->realSells.empty() )
  {
    foreach( it, good::all() )
    {
      _d->realSells.setCapacity( *it, _d->sells.capacity( *it ) );
    }
  }

  if( _d->peoples.count() != _d->states.population )
  {
      _d->initPeoples();
  }

  _d->tradeType = (options.get( "sea" ).toBool() ? EmpireMap::sea : EmpireMap::unknown)
                  + (options.get( "land" ).toBool() ? EmpireMap::land : EmpireMap::unknown);

  _initTextures();
}

const good::Store& ComputerCity::sells() const { return _d->realSells; }
const good::Store& ComputerCity::buys() const{ return _d->buys; }
const city::States& ComputerCity::states() const { return _d->states; }
void ComputerCity::delayTrade(unsigned int month){  _d->tradeDelay = month;}
void ComputerCity::empirePricesChanged(good::Product, const PriceInfo &){}

CityPtr ComputerCity::create( EmpirePtr empire, const std::string& name )
{
  CityPtr ret( new ComputerCity( empire, name ) );
  ret->drop();

  return ret;
}

void ComputerCity::addObject(ObjectPtr object )
{
  if( is_kind_of<Merchant>( object ) )
  {
    MerchantPtr merchant = ptr_cast<Merchant>( object );
    good::Store& sellGoods = merchant->sellGoods();
    good::Store& buyGoods = merchant->buyGoods();

    _d->buys.storeAll( buyGoods );

    foreach( gtype, good::all() )
    {
      int qty = sellGoods.freeQty( *gtype );
      good::Stock stock( *gtype, qty, qty );
      _d->realSells.store( stock, qty );
    }

    _d->sells.storeAll( sellGoods );

    _d->merchantsNumber = std::max<int>( 0, _d->merchantsNumber-1);
  }
  else if( is_kind_of<Barbarian>( object ) )
  {
    BarbarianPtr brb = ptr_cast<Barbarian>( object );
    _d->lastAttack = game::Date::current();
    int attack = std::max<int>( brb->strength() - strength(), 0 );
    if( !attack ) attack = 10;
    _d->strength = math::clamp<int>( _d->strength - math::random( attack ), 0, 100 );

    if( _d->strength > 0 )
    {
      int resist = std::max<int>( strength() - brb->strength(), 0 );
      brb->updateStrength( math::random( resist ) );
    }
    else
    {
      delayTrade( brb->strength() );
    }
  }
}

void ComputerCity::changeTradeOptions(const VariantMap& stream)
{
  VariantMap sells_vm = stream.get( "sells" ).toMap();
  foreach( it, sells_vm )
  {
    good::Product gtype = good::Helper::getType( it->first );
    _d->sells.setCapacity( gtype, Unit::fromValue( it->second ).toQty() );
    _d->realSells.setCapacity( gtype, Unit::fromValue( it->second ).toQty() );
  }

  VariantMap buys_vm = stream.get( "buys" ).toMap();
  foreach( it, buys_vm )
  {
    good::Product gtype = good::Helper::getType( it->first );
    _d->buys.setCapacity( gtype, Unit::fromValue( it->second ).toQty() );
  }
}

ComputerCity::~ComputerCity() {}

void ComputerCity::timeStep( unsigned int time )
{
  if( game::Date::isWeekChanged() )
  {
    _d->calcPopulationChange();
    _d->strength = math::clamp<int>( _d->strength+1, 0, _d->states.population / 100 );
  }

  if( game::Date::isMonthChanged() )
  {
    _d->tradeDelay = math::clamp<int>( _d->tradeDelay-1, 0, 99 );
    _d->calculateMonthState();    
  }

  if( game::Date::isYearChanged() )
  {
    _d->states.age++;

    //debug muleta
    if( _d->funds.money() < 1000 )
    {
      _d->funds.resolveIssue( econ::Issue( econ::Issue::donation, 1000 ) );
    }
  }

  //one year before step need
  if( _d->lastTimeUpdate.monthsTo( game::Date::current() ) > DateTime::monthsInYear-1 )
  {
    _d->merchantsNumber = math::clamp<int>( _d->merchantsNumber-1, 0, config::trade::maxMerchantsInRoute );
    _d->lastTimeUpdate = game::Date::current();

    foreach( gtype, good::all() )
    {
      _d->sells.setQty( *gtype, _d->sells.capacity( *gtype ) );
      _d->buys.setQty( *gtype, 0  );
      _d->realSells.setQty( *gtype, 0 );
    }
  }

  _checkMerchantsDeadline();
}

DateTime ComputerCity::lastAttack() const { return _d->lastAttack; }

std::string ComputerCity::about(Object::AboutType type)
{
  std::string ret;
  switch(type)
  {
  case empireMap:
    if( isDistantCity() ) ret = "##empmap_distant_romecity_tip##";
    else ret = name();
  break;

  case empireAdvInfo:
    if( isDistantCity() ) ret = "##empiremap_distant_city##";
    else ret = "";
  break;

  default:
    ret = "##compcity_unknown_about##";
  }

  return ret;
}

unsigned int ComputerCity::tradeType() const { return _d->tradeType; }
int ComputerCity::strength() const { return _d->strength; }

void ComputerCity::_initTextures()
{
  int index = PicID::otherCity;

  if( _d->distantCity ) { index = PicID::distantCity; }
  else if( _d->states.romeCity ) { index = PicID::romeCity; }

  setPicture( Picture( ResourceGroup::empirebits, index ) );
  _animation().load( ResourceGroup::empirebits, index+1, 6 );
  _animation().setLoop( true );
  _animation().setDelay( 2 );
}

}//end namespace world
