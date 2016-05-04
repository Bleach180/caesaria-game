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

#ifndef __CAESARIA_HEALTHBUILDINGS_H_INCLUDED__
#define __CAESARIA_HEALTHBUILDINGS_H_INCLUDED__

#include "service.hpp"

class HealthBuilding : public ServiceBuilding
{
public:
  virtual ~HealthBuilding();

  virtual unsigned int patientsMax() const;
  virtual unsigned int patientsCurrent() const;

  virtual void buildingsServed(const SmartSet<Building>&, ServiceWalkerPtr);
  virtual unsigned int walkerDistance() const;
  virtual void deliverService();

  virtual void save(VariantMap &stream) const;
  virtual void load(const VariantMap &stream);

  virtual void initialize(const object::Info &mdata);

protected:
  HealthBuilding( const Service::Type service,
                  const object::Type type,
                  const Size& size );

private:
  class Impl;
  ScopedPtr<Impl> _d;
};

#endif //__CAESARIA_HEALTHBUILDINGS_H_INCLUDED__
