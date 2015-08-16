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

#ifndef __CAESARIA_BUILDING_ENTERTAINMENT_H_INCLUDED__
#define __CAESARIA_BUILDING_ENTERTAINMENT_H_INCLUDED__

#include "objects/service.hpp"

class EntertainmentBuilding : public ServiceBuilding
{
public:
  typedef std::vector<walker::Type> NecessaryWalkers;

  EntertainmentBuilding( const Service::Type service, const object::Type type,
                         Size size);

  virtual ~EntertainmentBuilding();

  virtual void deliverService();
  virtual int currentVisitors() const;
  virtual int maxVisitors() const;

  virtual unsigned int walkerDistance() const;

  virtual float evaluateTrainee(walker::Type  traineeType);
  virtual bool isShow() const;
  virtual unsigned int showsCount() const;

  virtual void save(VariantMap &stream) const;
  virtual void load(const VariantMap &stream);

  virtual std::string troubleDesc() const;

  virtual NecessaryWalkers necessaryWalkers() const;

protected:
  virtual WalkerList _specificWorkers() const;
  void _addNecessaryWalker( walker::Type type );
  bool _isWalkerReady();

  class Impl;
  ScopedPtr<Impl> _d;
};

#endif //__CAESARIA_BUILDING_ENTERTAINMENT_H_INCLUDED__
