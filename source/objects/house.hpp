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
// Copyright 2012-2015 Dalerank, dalerankn8@gmail.com

#ifndef __CAESARIA_HOUSE_H_INCLUDED__
#define __CAESARIA_HOUSE_H_INCLUDED__

#include "objects/building.hpp"
#include "core/scopedptr.hpp"
#include "house_level.hpp"
#include "game/citizen_group.hpp"
#include "good/good.hpp"

class HouseSpecification;

class House : public Building
{
  friend class HouseSpecification;
public:
  House( HouseLevel::ID level=HouseLevel::vacantLot );

  virtual void timeStep(const unsigned long time);

  virtual good::Store& store();
  virtual const good::Store& store() const;

  // return the current house level
  const HouseSpecification& spec() const;
  int level() const;

  virtual void applyService(ServiceWalkerPtr walker);
  virtual float evaluateService(ServiceWalkerPtr walker);
  virtual void buyMarket(ServiceWalkerPtr walker);

  virtual void appendServiceValue(Service::Type srvc, float value );
  virtual bool hasServiceAccess( Service::Type service );
  virtual float getServiceValue( Service::Type service );
  virtual void setServiceValue(Service::Type service, float value );
  virtual gfx::TilesArray enterArea() const;
  virtual bool build( const city::AreaInfo& info );
  virtual bool getMinimapColor(int& color1, int& color2) const;
  virtual const gfx::Pictures& pictures( gfx::Renderer::Pass pass ) const;
  virtual Variant getProperty(const std::string& name) const;

  virtual float state(int param) const;

  unsigned int hired() const;
  unsigned int unemployed() const;

  float isEvolveEducationNeed(Service::Type type);

  const Desirability& desirability() const;

  virtual void destroy();

  virtual void save(VariantMap& stream) const;
  virtual void load(const VariantMap& stream);

  virtual void collapse();
  virtual void burn();

  unsigned int capacity() const;
  void addHabitants( CitizenGroup& habitants );
  CitizenGroup removeHabitants( int paramCount );
  void removeHabitants( CitizenGroup& group );
  const CitizenGroup& habitants() const;

  float collectTaxes();
  float taxesThisYear() const;
  void appendMoney( float money );
  DateTime lastTaxationDate() const;

  std::string evolveInfo() const;
  std::string levelName() const;

  virtual int roadsideDistance() const;
  virtual bool isWalkable() const;
  virtual bool isFlat() const;

  virtual std::string sound() const;
  virtual std::string troubleDesc() const;

  bool isCheckedDesirability() const;
  void addWalker(WalkerPtr walker);
  bool ready4evolve(object::Type type) const;
  const WalkerList& walkers() const;

  void __debugChangeLevel( int change );
  void __debugMakeGeneration();
private:
  void _updateHealthLevel();
  void _levelUp();
  void _levelDown();
  void _disaster();
  void _update(bool needChangeTexture);
  void _updateGround();
  bool _tryEvolve_1_to_12_lvl(int level, int growSize, const char desirability );
  bool _tryEvolve_12_to_20_lvl(int level4grow, int minSize, const char desirability);
  void _tryDegrage_12_to_2_lvl( const char desirability );
  void _tryDegrade_20_to_12_lvl(int size, const char desirability);
  void _setServiceMaxValue( Service::Type type, unsigned int value );
  void _checkEvolve();
  void _checkPatricianDeals();
  void _updateTax();
  void _updateCrime();
  void _updateHappiness();
  void _updateHomeless();
  void _settleVacantLotIfNeed();
  void _updateConsumptions(const unsigned long time);

  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //__CAEARIA_HOUSE_H_INCLUDED__
