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

#ifndef _CAESARIA_FISHPLACE_H_INCLUDE_
#define _CAESARIA_FISHPLACE_H_INCLUDE_

#include "animals.hpp"

class FishPlace : public Fish
{
  WALKER_MUST_INITIALIZE_FROM_FACTORY
public:
  virtual ~FishPlace();

  virtual void timeStep(const unsigned long time);

  virtual void save(VariantMap &stream) const;
  virtual void load(const VariantMap &stream);

  void send2city( TilePos pos, TilePos dst );

protected:
  virtual void _reachedPathway();
  virtual const gfx::Picture& getMainPicture();

private:
  FishPlace( PlayerCityPtr city );
  void _findway( TilePos start, TilePos end );

  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //_CAESARIA_FISHPLACE_H_INCLUDE_
