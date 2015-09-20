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

#ifndef _CAESARIA_BUILDING_H_INCLUDE_
#define _CAESARIA_BUILDING_H_INCLUDE_

#include "construction.hpp"
#include "good/good.hpp"
#include "core/scopedptr.hpp"
#include "gfx/animation.hpp"
#include "core/referencecounted.hpp"
#include "predefinitions.hpp"
#include "game/service.hpp"
#include "walker/constants.hpp"
#include "gfx/tilesarray.hpp"

class Building : public Construction
{
public:
  Building(const object::Type type, const Size& size=Size(1) );
  virtual ~Building();
  virtual void initTerrain( gfx::Tile& terrain);

  virtual void timeStep(const unsigned long time);
  virtual void storeGoods(good::Stock& stock, const int amount = -1);

  // evaluate the given service
  virtual bool build(const city::AreaInfo& info);

  // handle service reservation
  void reserveService(const Service::Type service);
  bool isServiceReserved(const Service::Type service);
  void cancelService(const Service::Type service);
  virtual float evaluateService( ServiceWalkerPtr walker);
  virtual void applyService( ServiceWalkerPtr walker);

  // evaluate the need for the given trainee
  virtual float evaluateTrainee( walker::Type traineeType);  // returns >0 if trainee is needed
  void reserveTrainee( walker::Type traineeType ); // trainee will come
  void cancelTrainee( walker::Type traineeType );  // trainee will not come
  int traineeValue( walker::Type traineeType ) const;

  virtual void updateTrainee( TraineeWalkerPtr walker ); // trainee arrives
  virtual void setTraineeValue( walker::Type type, int value ); // trainee arrives
  virtual void initialize(const MetaData& mdata);

  virtual void save(VariantMap &stream) const;
  virtual void load(const VariantMap &stream);

  virtual gfx::Renderer::PassQueue passQueue() const;

protected:
  void _updateBalanceKoeffs();

  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //_CAESARIA_BUILDING_H_INCLUDE_
