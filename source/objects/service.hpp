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
// Copyright 2012-2014 Dalerank, dakerankn8@gmail.com

#ifndef _CAESARIA_SERVICE_BUILDING_H_INCLUDE_
#define _CAESARIA_SERVICE_BUILDING_H_INCLUDE_

#include "objects/working.hpp"
#include "predefinitions.hpp"

class ServiceBuilding : public WorkingBuilding
{
public:
  ServiceBuilding( const Service::Type service,
                   const object::Type type, const Size& size );

  virtual ~ServiceBuilding();

  virtual Service::Type serviceType() const;
  virtual void timeStep(const unsigned long time);
  virtual void destroy();  // handles the walkers

  int serviceRange() const;  // max distance from building to road for road to propose the service
  void setServiceDelay( const int delay );
  virtual int serviceDelay() const;

  virtual int time2NextService() const;
  virtual DateTime lastSendService() const;

  // called when a service man should service the neighborhood
  virtual void deliverService();
  
  virtual void save( VariantMap& stream) const;
  virtual void load( const VariantMap& stream);
  virtual void buildingsServed( const std::set<BuildingPtr>& buildings, ServiceWalkerPtr walker );
  virtual unsigned int walkerDistance() const;

  std::string workersStateDesc() const;
protected:
  virtual int _getWalkerOrders() const;

private:  
  void _setLastSendService( DateTime time );

  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //_CAESARIA_SERVICE_BUILDING_H_INCLUDE_
