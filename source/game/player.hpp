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

#ifndef _CAESARIA_PLAYER_HPP_INCLUDE_
#define _CAESARIA_PLAYER_HPP_INCLUDE_

#include "core/serializer.hpp"
#include "core/scopedptr.hpp"
#include "predefinitions.hpp"
#include "core/referencecounted.hpp"

class NColor;

class Player : public Serializable, public ReferenceCounted
{
public:
  static PlayerPtr create();

  virtual void save( VariantMap& stream) const;
  virtual void load( const VariantMap& stream);

  void setName( const std::string& name );
  std::string name() const;

  void setRank( int rank );
  int rank() const;

  int salary() const;
  void setSalary( const int value );

  void appendMoney( int money );
  int money() const;

  NColor color() const;

  virtual ~Player();
private:
  Player();

  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //_CAESARIA_PLAYER_HPP_INCLUDE_
