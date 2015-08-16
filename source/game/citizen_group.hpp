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

#ifndef _CAESARIA_CITIZEN_GROUP_INCLUDE_H_
#define _CAESARIA_CITIZEN_GROUP_INCLUDE_H_

#include "core/variant.hpp"

class CitizenGroup
{
public:
  typedef enum { newborn=0, child, scholar, student, mature, aged, longliver=99, any=100 } Age;
  enum { childMin=2, matureMin=21 };

  unsigned int count() const;
  unsigned int count( Age group ) const;
  unsigned int count( unsigned int beginAge, unsigned int endAge ) const;

  CitizenGroup retrieve(unsigned int count );
  CitizenGroup retrieve( Age group, unsigned int count );
  CitizenGroup& include( CitizenGroup& b );
  void exclude( CitizenGroup& group );

  unsigned int& operator [] ( unsigned int age);
  CitizenGroup& operator += ( const CitizenGroup& b );
  CitizenGroup  operator  -  ( const CitizenGroup& b ) const;
  CitizenGroup  operator  +  ( const CitizenGroup& b ) const;

  bool empty() const;
  void clear();
  void set( const CitizenGroup& b );
  void makeOld();

  unsigned int child_n()   const;
  unsigned int mature_n()  const;
  unsigned int aged_n()    const;
  unsigned int scholar_n() const;
  unsigned int student_n() const;

  VariantList save() const;
  void load( const VariantList& stream );

  CitizenGroup();
  CitizenGroup( Age age, int value );
  static CitizenGroup random( int value );
protected:
  typedef std::vector< unsigned int > Peoples;
  Peoples _peoples;
};

#endif //_CAESARIA_CITIZEN_GROUP_INCLUDE_H_
