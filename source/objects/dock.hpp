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
// Copyright 2012-2015 Dalerank, dalerankn8@gmail.com

#ifndef _CAESARIA_DOCK_INCLUDE_H_
#define _CAESARIA_DOCK_INCLUDE_H_

#include "working.hpp"
#include "core/direction.hpp"

class Dock : public WorkingBuilding
{
public:
  Dock();
  virtual ~Dock();

  virtual bool canBuild(const city::AreaInfo& areaInfo) const;  // returns true if it can be built there
  virtual Construction::BuildArea buildArea(const city::AreaInfo& areaInfo) const;
  virtual bool build(const city::AreaInfo& info);
  virtual void destroy();

  virtual void timeStep(const unsigned long time);
  virtual void save(VariantMap& stream) const;
  virtual void load(const VariantMap& stream);
  virtual void reinit();
  virtual std::string workersProblemDesc() const;

  bool isBusy() const;

  const gfx::Tile& landingTile() const;
  const gfx::Tile& queueTile() const;

  int queueSize() const;

  const good::Store& exportStore() const;

  void requestGoods( good::Stock& stock );

  int importingGoods( good::Stock& stock );
  int exportingGoods( good::Stock& stock, int qty );
  void storeGoods( good::Stock& stock, const int amount);

private:
  void _setDirection( Direction direction );
  virtual void _updatePicture( Direction direction );
  void _tryReceiveGoods();
  void _tryDeliverGoods();

private:
  Dock( PlayerCityPtr city );

  class Impl;
  ScopedPtr<Impl> _d;
};

#endif //_CAESARIA_DOCK_INCLUDE_H_
