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

#include "trading_options.hpp"
#include "core/stringhelper.hpp"
#include "game/game.hpp"
#include "world/empire.hpp"
#include "good/goodhelper.hpp"
#include "world/city.hpp"

namespace events
{

GameEventPtr TradingOptions::create()
{
  GameEventPtr ret( new TradingOptions() );
  ret->drop();

  return ret;
}

void TradingOptions::load(const VariantMap& stream)
{
  _options = stream;
}

void TradingOptions::_exec(Game& game, unsigned int)
{
  VariantMap citiesVm = _options.get( "cities" ).toMap();
  foreach( it, citiesVm )
  {
    std::string cityName = it->first;
    int trade_delay = it->second.toMap().get( "delay_trade" );
    if( trade_delay > 0 )
    {
      world::CityPtr cityp = game.empire()->findCity( cityName );
      if( cityp.isValid() )
      {
        cityp->delayTrade( trade_delay );
      }
    }
  }

  VariantMap goodsVm = _options.get( "goods" ).toMap();
  foreach( it, goodsVm )
  {
    Good::Type gtype = GoodHelper::getType( it->first );
    if( gtype != Good::none )
    {
      VariantMap goodInfo = it->second.toMap();
      int buy = goodInfo.get( "buy" );
      int sell = goodInfo.get( "sell" );
      game.empire()->setPrice( gtype, buy, sell );
    }
  }
}

bool TradingOptions::_mayExec(Game&, unsigned int) const{  return true; }

}
