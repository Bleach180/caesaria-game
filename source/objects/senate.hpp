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

#ifndef __CAESARIA_SENATE_H_INCLUDED_
#define __CAESARIA_SENATE_H_INCLUDED_

#include "service.hpp"
#include "core/scopedptr.hpp"

class Senate : public ServiceBuilding
{
public:
  typedef enum { culture, prosperity, peace, favour, workless } Status;
  Senate();

  int funds() const;
  unsigned int thisYearTax() const;

  float takeMoney();

  int status( Status status ) const;

  virtual std::string errorDesc() const;

  virtual void deliverService();  

  virtual bool canBuild( const city::AreaInfo& areaInfo )const;

  virtual void applyService(ServiceWalkerPtr walker);

  virtual bool build( const city::AreaInfo& info );

  virtual unsigned int walkerDistance() const;

  virtual void timeStep(const unsigned long time);

  virtual void initialize(const object::Info& mdata);
  virtual Variant getProperty(const std::string& name) const;

  virtual void save( VariantMap& stream ) const;
  virtual void load( const VariantMap& stream );

private:
  void _updateUnemployers();
  void _updateRatings();
  void _updateRating( Status status );

  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //__CAESARIA_SENATE_H_INCLUDED_
