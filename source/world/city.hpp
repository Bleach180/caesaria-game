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

#ifndef __CAESARIA_EMPIRECITY_H_INCLUDED__
#define __CAESARIA_EMPIRECITY_H_INCLUDED__

#include "object.hpp"
#include "core/referencecounted.hpp"
#include "core/position.hpp"
#include "predefinitions.hpp"
#include "good/good.hpp"
#include "nation.hpp"
#include "game/predefinitions.hpp"

namespace econ { class Treasury; }
namespace city { struct States; }

namespace world
{

struct PriceInfo;

class City : public Object
{
public:
  typedef enum { inactive=0, indifferent, agressive, friendly, aiCount } AiMode;
  City( EmpirePtr empire );

  // performs one simulation step
  virtual bool isAvailable() const { return true; }
  virtual void setAvailable( bool value ) {}
  virtual void setModeAI( AiMode mode ) {}
  virtual AiMode modeAI() const { return aiCount; }

  virtual unsigned int tradeType() const = 0;
  virtual econ::Treasury& treasury() = 0;
  virtual bool isPaysTaxes() const = 0;
  virtual bool haveOverduePayment() const = 0;
  virtual bool isMovable() const { return false; }
  virtual DateTime lastAttack() const = 0;
  virtual int strength() const = 0;
  virtual PlayerPtr mayor() const = 0;

  virtual void delayTrade( unsigned int month ) = 0;
  virtual void empirePricesChanged( good::Product gtype, const PriceInfo& prices ) = 0;
  virtual const good::Store& sells() const = 0;
  virtual const good::Store& buys() const = 0;
  virtual const city::States& states() const = 0;
};

}//end namespace world

#endif //__CAESARIA_EMPIRECITY_H_INCLUDED__
