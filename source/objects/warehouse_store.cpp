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
// Copyright 2012-2013 Dalerank, dalerankn8@gmail.com

#include "warehouse_store.hpp"
#include "good/helper.hpp"
#include "core/utils.hpp"
#include "core/logger.hpp"
#include "core/variant_map.hpp"
#include "core/variant_list.hpp"
#include "core/metric.hpp"

using namespace metric;

WarehouseStore::WarehouseStore()
{
  _warehouse = NULL;

  foreach( goodType, good::all() )
  {
    setOrder( *goodType, good::Orders::accept );
    _capacities[ *goodType ] = 9999;
  }
}

void WarehouseStore::init(Warehouse& warehouse){  _warehouse = &warehouse; }

int WarehouseStore::qty(const good::Product &goodType) const
{
  if( _warehouse->numberWorkers() == 0 )
    return 0;

  int amount = 0;

  foreach( room, _warehouse->rooms() )
  {
    if ( room->type() == goodType || goodType == good::any() )
    {
      amount += room->qty();
    }
  }

  return amount;
}

int WarehouseStore::qty() const { return qty( good::any() ); }

int WarehouseStore::getMaxStore(const good::Product goodType)
{
  if( getOrder( goodType ) == good::Orders::reject || isDevastation() || _warehouse->onlyDispatchGoods() )
  { 
    return 0;
  }

  // compute the quantity of each goodType in the warehouse, taking in account all reservations
  good::ProductMap maxStore;

  // init the map
  foreach( i, good::all() )
  {
    maxStore[ *i ] = 0;
  }
  // put current stock in the map
  foreach( room, _warehouse->rooms() )
  {
    maxStore[ room->type() ] += room->qty();
  }

  // add reservations
  good::Reservations& reservations = _getStoreReservations();
  foreach( i, reservations )
  {
    const good::Stock& reservationStock = i->stock;
    maxStore[ reservationStock.type() ] += reservationStock.qty();
  }

  if( maxStore[ goodType ] >= _capacities[ goodType ])
    return 0;

  // compute number of free tiles
  int nbFreeTiles = _warehouse->rooms().size();
  foreach( mapItem, maxStore )
  {
    good::Product otherGoodType = mapItem->first;
    if (otherGoodType == goodType)
    {
      // don't count this goodType
      continue;
    }
    Unit units = Unit::fromQty( mapItem->second );
    int nbTiles = ( units.ivalue() + 3 )/4;  // nb of subTiles this goodType occupies
    nbFreeTiles -= nbTiles;
  }

  int freeRoom = Warehouse::Room::basicCapacity * nbFreeTiles - maxStore[goodType];

  return freeRoom;
}

void WarehouseStore::applyStorageReservation( good::Stock &stock, const int reservationID )
{
  good::Stock reservedStock = getStorageReservation(reservationID, true);

  if (stock.type() != reservedStock.type())
  {
    Logger::warning( "Warehouse: GoodType does not match reservation" );
    return;
  }

  if (stock.qty() < reservedStock.qty())
  {
    Logger::warning( "Warehouse: Quantity does not match reservation" );
    return;
  }


  int amount = reservedStock.qty();
  // std::cout << "WarehouseStore, store qty=" << amount << " resID=" << reservationID << std::endl;

  // first we look at the half filled subTiles
  if (amount > 0)
  {
    foreach( room, _warehouse->rooms() )
    {
      if( room->type() == stock.type() && room->freeQty() > 0 )
      {
        int tileAmount = std::min(amount, room->freeQty());
        // std::cout << "put in half filled" << std::endl;
        room->append(stock, tileAmount);
        amount -= tileAmount;
      }
    }

    // then we look at the empty subTiles
    foreach( room, _warehouse->rooms() )
    {
      if( room->type() == good::none)
      {
        int tileAmount = std::min(amount, room->capacity() );
        // std::cout << "put in empty tile" << std::endl;
        room->append(stock, tileAmount);
        amount -= tileAmount;
      }
    }
  }

  _warehouse->computePictures();
}

void WarehouseStore::applyRetrieveReservation(good::Stock& stock, const int reservationID)
{
  good::Stock reservedStock = getRetrieveReservation(reservationID, true);

  if( stock.type() != reservedStock.type() )
  {   
    Logger::warning( "Warehouse: GoodType does not match reservation need=%s have=%s",
                     good::Helper::name(reservedStock.type()).c_str(),
                     good::Helper::name(stock.type()).c_str() );
    return;
  }
  if( stock.capacity() < stock.qty() + reservedStock.qty() )
  {
    Logger::warning( "Warehouse: Retrieve stock[%s] less reserve qty, decrease from %d to &%d",
                     good::Helper::name(stock.type()).c_str(),
                     reservedStock.qty(), stock.freeQty() );
    reservedStock.setQty( stock.freeQty() );
  }

  int amount = reservedStock.qty();

  // first we look at the half filled subTiles
  foreach( room, _warehouse->rooms() )
  {
    if (amount == 0)
    {
      break;
    }

    if( room->type() == stock.type() && room->freeQty() > 0 )
    {
      int tileAmount = std::min(amount, room->qty());
      // std::cout << "retrieve from half filled" << std::endl;
      stock.append( *room, tileAmount);
      if( room->empty() )
        room->setType( good::none );

      amount -= tileAmount;
    }
  }

  // then we look at the filled subTiles
  foreach( room, _warehouse->rooms() )
  {
    if (amount == 0)
    {
      break;
    }

    if( room->type() == stock.type())
    {
      int tileAmount = std::min(amount, room->qty());
      // std::cout << "retrieve from filled" << std::endl;
      stock.append( *room, tileAmount);
      amount -= tileAmount;
    }
  }

  _warehouse->computePictures();
}

void WarehouseStore::retrieve(good::Stock& stock, const int amount)
{
  good::Store::retrieve( stock, amount );

  _warehouse->computePictures();
}

VariantMap WarehouseStore::save() const
{
  VariantMap ret = Store::save();
  VARIANT_SAVE_CLASS( ret, _capacities )

  return ret;
}

void WarehouseStore::load(const VariantMap &stream)
{
  Store::load( stream );
  int maxCapacity = capacity();

  VARIANT_LOAD_CLASS_LIST( _capacities, stream )

  foreach( it, _capacities )
    if( it->second == 0 )
      it->second = maxCapacity;
}

int WarehouseStore::capacity() const
{
  return Warehouse::Room::basicCapacity * _warehouse->rooms().size();
}

good::ProductMap WarehouseStore::details() const
{
  good::ProductMap ret;

  foreach( room, _warehouse->rooms() )
  {
    ret[ room->type() ] += room->qty();
  }

  return ret;
}

void WarehouseStore::setCapacity(const int) {}
void WarehouseStore::setCapacity(const good::Product &goodType, const int maxQty) {  _capacities[ goodType ] = maxQty; }

int WarehouseStore::capacity( const good::Product& goodType ) const
{
  good::ProductMap::const_iterator it = _capacities.find( goodType );
  return it != _capacities.end() ? it->second : 0;
}
