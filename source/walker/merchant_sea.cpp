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

#include "merchant_sea.hpp"
#include "good/storage.hpp"
#include "pathway/pathway_helper.hpp"
#include "city/statistic.hpp"
#include "core/variant_map.hpp"
#include "city/statistic.hpp"
#include "gfx/tile.hpp"
#include "world/empire.hpp"
#include "core/utils.hpp"
#include "game/funds.hpp"
#include "city/trade_options.hpp"
#include "name_generator.hpp"
#include "gfx/tilemap.hpp"
#include "events/event.hpp"
#include "core/logger.hpp"
#include "objects/constants.hpp"
#include "world/merchant.hpp"
#include "objects/dock.hpp"
#include "game/gamedate.hpp"
#include "walkers_factory.hpp"
#include "gfx/helper.hpp"

using namespace city;

REGISTER_CLASS_IN_WALKERFACTORY(walker::seaMerchant, SeaMerchant)

class SeaMerchant::Impl
{
public:  
  typedef enum { stFindDock=0,
                 stGoOutFromCity,
                 stRequestGoods,
                 stWaitFreeDock,
                 stSellGoods,
                 stBuyGoods,
                 stNothing,
                 stWaitGoods,
                 stBackToBaseCity } State;

  TilePos destBuildingPos;  // dock
  good::Storage sell;
  good::Storage buy;
  int tryDockCount;
  int maxTryDockCount;
  int waitInterval;
  int currentSell;
  int currentBuys;
  DateTime landingDate;
  std::string baseCityName;
  State nextState;
  bool anyBuy, anySell;

  void resolveState(PlayerCityPtr city, WalkerPtr wlk);
  Pathway findNearbyDock(const DockList& docks, TilePos position );
  void goAwayFromCity( PlayerCityPtr city, WalkerPtr walker );
  DockPtr findLandingDock(PlayerCityPtr city, WalkerPtr walker );
  Pathway findRandomRaid(const DockList& docks, TilePos position);
};

SeaMerchant::SeaMerchant(PlayerCityPtr city )
  : Merchant( city ), _d( new Impl )
{
  _setType( walker::seaMerchant );
  _d->waitInterval = 0;
  _d->tryDockCount = 0;
  _d->maxTryDockCount = 3;
  _d->currentSell = 0;
  _d->currentBuys = 0;
  _d->anyBuy = false;
  _d->anySell = false;

  setName( NameGenerator::rand( NameGenerator::male ) );
}

SeaMerchant::~SeaMerchant()
{
}

void SeaMerchant::Impl::resolveState(PlayerCityPtr city, WalkerPtr wlk )
{
  switch( nextState )
  {
  case stFindDock:
  {    
    destBuildingPos = gfx::tilemap::invalidLocation();  // no destination yet

    Pathway pathway;
    // get the list of buildings within reach   
    if( tryDockCount < maxTryDockCount )
    {
      DockList docks = city->statistic().objects.find<Dock>( object::dock );

      if( !docks.empty() )
      {
        DockList freeDocks;
        for( auto dock : docks )
        {
          if( !dock->isBusy() )
          {
            freeDocks.push_back( dock );
          }
        }

        if( freeDocks.empty() )
        {
          pathway = findRandomRaid( docks, wlk->pos() );
          nextState = stWaitFreeDock;
        }
        else
        {
          pathway = findNearbyDock( freeDocks, wlk->pos() );
          nextState = stRequestGoods;
        }
      }
    }
    tryDockCount++;

    if( pathway.isValid() )
    {
      // we found a destination!      
      wlk->setPathway( pathway );
      wlk->go();
    }
    else
    {
      nextState = stGoOutFromCity;
      resolveState( city, wlk );
    }
  }
  break;

  case stWaitFreeDock:
  {
    waitInterval = game::Date::days2ticks( DateTime::daysInWeek );
    nextState = stFindDock;
  }
  break;

  case stRequestGoods:
  {
    landingDate = game::Date::current();

    WalkerList walkers = city->walkers( wlk->pos() );
    walkers.remove( wlk );

    bool emptyDock = walkers.empty();

    DockPtr myDock = findLandingDock( city, wlk );
    if( myDock.isValid() && emptyDock )
    {
      trade::Options& options = city->tradeOptions();
      good::ProductMap cityGoodsAvailable = city->statistic().goods.inWarehouses();
      //request goods
      for( auto& goodType : good::all() )
      {
        int needQty = buy.freeQty( goodType );
        if (!options.isExporting( goodType) )
        {
          continue;
        }
        int exportLimit = options.tradeLimit( trade::exporting, goodType ).toQty();
        int maySell = math::clamp<unsigned int>( cityGoodsAvailable[ goodType ] - exportLimit, 0, needQty );

        if( maySell > 0)
        {
          good::Stock stock( goodType, maySell, maySell );
          myDock->requestGoods( stock );
          anyBuy = true;
        }
      }

      nextState = stSellGoods;
      resolveState( city, wlk );
    }
    else
    {
      nextState = stFindDock;
      resolveState( city, wlk );
    }
  }
  break;

  case stBuyGoods:
  {
    DockPtr myDock = findLandingDock( city, wlk );

    if( myDock.isValid() )
    {
      trade::Options& options = city->tradeOptions();
      //try buy goods
      for( auto& goodType : good::all() )
      {
        if (!options.isExporting( goodType))
        {
          continue;
        }

        int needQty = buy.freeQty( goodType );

        if( needQty > 0 )
        {
          good::Stock& stock = buy.getStock( goodType );
          currentBuys += myDock->exportingGoods( stock, needQty );
          anyBuy = true;
        }
      }

      nextState = stWaitGoods;
      waitInterval = anyBuy ? game::Date::days2ticks( DateTime::daysInWeek ) : 0;

      if( 0 == buy.freeQty() ) //all done
      {
        nextState = stGoOutFromCity;
      }
    }
    else
    {
      nextState = stFindDock;
      resolveState( city, wlk );
    }
  }
  break;

  case stWaitGoods:
  {
    if( !anyBuy && !anySell ) //...bad city, nothing to trade
    {
      nextState = stGoOutFromCity;
      resolveState( city, wlk );
    }
    else
    {
      if( landingDate.monthsTo( game::Date::current() ) > 2 )
      {
        nextState = stGoOutFromCity;
        resolveState( city, wlk );
      }
      else
      {
        nextState = stBuyGoods;
        resolveState( city, wlk );
      }
    }
  }
  break;

  case stGoOutFromCity:
  {
    // we have nothing to buy/sell with city, or cannot find available warehouse -> go out
    waitInterval = game::Date::days2ticks( DateTime::daysInWeek );
    goAwayFromCity( city, wlk );
    nextState = stBackToBaseCity;
  }
  break;

  case stSellGoods:
  {    
    DockPtr myDock = findLandingDock( city, wlk );
    if( myDock.isValid() )
    {
      trade::Options& options = city->tradeOptions();
      const good::Store& importing = options.buys();
      //try sell goods
      for( auto& goodType : good::all() )
      {
        if (!options.isImporting(goodType))
        {
          continue;
        }

        if( sell.qty(goodType) > 0 && importing.capacity(goodType) > 0)
        {
          currentSell += myDock->importingGoods( sell.getStock(goodType) );
          anySell = true;
        }
      }
    }

    nextState = stBuyGoods;
    resolveState( city, wlk );
    waitInterval = anySell ? game::Date::days2ticks( DateTime::daysInWeek ) : 0;
  }
  break;

  case stBackToBaseCity:
  {
    if( sell.freeQty() == 0 && buy.qty() == 0 )
    {
      wlk->setThinks( "##seamerchant_noany_trade##" );
    }

    // walker on exit from city
    wlk->deleteLater();
    world::EmpirePtr empire = city->empire();
    const std::string& ourCityName = city->name();
    world::TraderoutePtr route = empire->findRoute( ourCityName, baseCityName );
    if( route.isValid() )
    {
      route->addMerchant( ourCityName, sell, buy );
    }

    nextState = stNothing;
  }
  break;

  default:
    Logger::warning( "SeaMerchant: unknown state resolved" );
  }
}

Pathway SeaMerchant::Impl::findRandomRaid(const DockList& docks, TilePos position)
{
  DockPtr minQueueDock;
  int minQueue = 999;

  for( auto it : docks )
  {
    int currentQueueSize = it->queueSize();
    if( currentQueueSize < minQueue )
    {
      minQueue = currentQueueSize;
      minQueueDock = it;
    }
  }

  Pathway ret;
  if( minQueueDock.isValid() )
  {
    ret = PathwayHelper::create( position, minQueueDock->queueTile().pos(), PathwayHelper::deepWater );
  }

  return ret;
}

Pathway SeaMerchant::Impl::findNearbyDock(const DockList& docks, TilePos position)
{
  DockList::const_iterator i = docks.begin();
  Pathway ret = PathwayHelper::create( position, (*i)->landingTile().pos(), PathwayHelper::deepWater );

  ++i;
  for( ; i != docks.end(); ++i )
  {
    Pathway tmp = PathwayHelper::create( position, (*i)->landingTile().pos(), PathwayHelper::deepWater );
    if( tmp.length() < ret.length() )
    {
      ret = tmp;
    }
  }
  return ret;
}

void SeaMerchant::_reachedPathway()
{
  Walker::_reachedPathway();
  _d->resolveState( _city(), this );
}

void SeaMerchant::Impl::goAwayFromCity( PlayerCityPtr city, WalkerPtr walker )
{
  Pathway pathway = PathwayHelper::create( walker->pos(), city->borderInfo().boatExit, PathwayHelper::deepWater );
  if( !pathway.isValid() )
  {
    walker->deleteLater();
  }
  else
  {
    walker->setPathway( pathway );
    walker->go();
  }
}

DockPtr SeaMerchant::Impl::findLandingDock(PlayerCityPtr city, WalkerPtr walker)
{
  DockList docks = city->statistic().objects.find<Dock>( object::dock,
                                                         walker->pos() - TilePos( 1, 1),
                                                         walker->pos() + TilePos( 1, 1 ) );
  for( auto dock : docks )
  {
    if( dock->landingTile().pos() == walker->pos() )
    {
      return dock;
    }
  }

  return DockPtr();
}

void SeaMerchant::send2city()
{
  _d->nextState = Impl::stFindDock;
  setPos( _city()->borderInfo().boatEntry );
  _d->resolveState( _city(), this );
  attach();
}

void SeaMerchant::save( VariantMap& stream ) const
{
  Walker::save( stream );
  VARIANT_SAVE_ANY_D( stream, _d, destBuildingPos )
  VARIANT_SAVE_STR_D( stream, _d, baseCityName )
  VARIANT_SAVE_ANY_D( stream, _d, waitInterval )
  VARIANT_SAVE_ANY_D( stream, _d, currentSell )
  VARIANT_SAVE_ANY_D( stream, _d, currentBuys )
  VARIANT_SAVE_ENUM_D( stream, _d, nextState )
  VARIANT_SAVE_CLASS_D( stream, _d, buy )
  VARIANT_SAVE_CLASS_D( stream, _d, sell )
}

void SeaMerchant::load( const VariantMap& stream)
{
  Walker::load( stream );
  VARIANT_LOAD_ANY_D( _d, destBuildingPos, stream )
  VARIANT_LOAD_STR_D( _d, baseCityName, stream )
  VARIANT_LOAD_ANY_D( _d, waitInterval, stream )
  VARIANT_LOAD_ENUM_D( _d, nextState, stream )
  VARIANT_LOAD_ANY_D( _d, currentBuys, stream )
  VARIANT_LOAD_ANY_D( _d, currentSell, stream )
  VARIANT_LOAD_CLASS_D( _d, buy, stream )
  VARIANT_LOAD_CLASS_D( _d, sell, stream )
}

good::ProductMap SeaMerchant::sold() const { return _d->sell.details(); }
good::ProductMap SeaMerchant::bougth() const { return _d->buy.details(); }
good::ProductMap SeaMerchant::mayBuy() const { return _d->buy.amounts(); }

void SeaMerchant::timeStep(const unsigned long time)
{
  if( _d->waitInterval > 0 )
  {
    _d->waitInterval--;
    return;
  }

  switch( _d->nextState )
  {
  case Impl::stWaitGoods:
  case Impl::stWaitFreeDock:
    _d->resolveState( _city(), this );
  break;

  default: break;
  }

  Walker::timeStep( time );
}

bool SeaMerchant::isWaitFreeDock() const {  return Impl::stWaitFreeDock == _d->nextState; }

std::string SeaMerchant::thoughts(Thought th) const
{
  switch( th )
  {
  case thCurrent:
    switch( _d->nextState )
    {
    case Impl::stSellGoods:
    case Impl::stBuyGoods:
      if( _d->anySell || _d->anyBuy )
      {
        return "##dockers_taking_our_goods##";
      }
    break;

    case Impl::stBackToBaseCity:
    {
      if( _d->currentSell - _d->currentBuys > 100 )
      {
        return "##seamrchant_another_successful_voyage##";
      }
      else if( abs( _d->currentSell - _d->currentBuys ) < 100 )
      {
        return "##seamerchant_noany_trade##";
      }
    }
    break;

    case Impl::stWaitGoods:
    {
      return "##docked_buying_selling_goods##";
    }
    break;

    case Impl::stWaitFreeDock:
    case Impl::stRequestGoods:
      if( action() == acMove )
      {
        return "##sailing_to_city_docks##";
      }
      else
      {
        return "##waiting_for_free_dock##";
      }
    break;

    default: break;
    }
  break;

  default: break;
  }

  return Walker::thoughts(th);
}

TilePos SeaMerchant::places(Walker::Place type) const
{
  switch( type )
  {
  case plDestination: return _d->destBuildingPos;
  default: break;
  }

  return Human::places( type );
}


WalkerPtr SeaMerchant::create(PlayerCityPtr city) {  return create( city, world::MerchantPtr() ).object(); }

SeaMerchantPtr SeaMerchant::create(PlayerCityPtr city, world::MerchantPtr merchant )
{
  SeaMerchantPtr cityMerchant( new SeaMerchant( city ) );
  cityMerchant->drop();

  if( merchant.isValid() )
  {
    cityMerchant->_d->sell.resize( merchant->sellGoods() );
    cityMerchant->_d->sell.storeAll( merchant->sellGoods() );
    cityMerchant->_d->buy.resize( merchant->buyGoods() );
    cityMerchant->_d->buy.storeAll( merchant->buyGoods() );
    cityMerchant->_d->baseCityName = merchant->baseCity();
  }

  return cityMerchant;
}

std::string SeaMerchant::parentCity() const { return _d->baseCityName; }
