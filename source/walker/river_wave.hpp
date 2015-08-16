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

#ifndef __CAESARIA_RIVER_WAVE_H_INCLUDED__
#define __CAESARIA_RIVER_WAVE_H_INCLUDED__

#include "walker.hpp"
#include "predefinitions.hpp"
#include "gfx/animation.hpp"

class RiverWave : public Walker
{
public:
  static RiverWavePtr create( PlayerCityPtr city );
  virtual ~RiverWave();

  virtual void send2City(const TilePos& location);
  virtual void timeStep(const unsigned long time);
  virtual void save(VariantMap& stream) const;

protected:
  const gfx::Picture& getMainPicture();

private:
  RiverWave( PlayerCityPtr city );
  gfx::Animation _animation;
  unsigned int _delay;
};

#endif //__CAESARIA_RIVER_WAVE_H_INCLUDED__
