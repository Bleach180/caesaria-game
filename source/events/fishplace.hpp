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

#ifndef _CAESARIA_FISHPLACE_EVENT_H_INCLUDE_
#define _CAESARIA_FISHPLACE_EVENT_H_INCLUDE_

#include "event.hpp"
#include "core/scopedptr.hpp"

namespace events
{

class ChangeFishery : public GameEvent
{
public:
  typedef enum { unknown, add=1, remove } Mode;
  static GameEventPtr create( TilePos pos, Mode mode );

  virtual void load(const VariantMap &stream);
  virtual VariantMap save() const;

protected:
  virtual void _exec(Game &game, unsigned int time);
  virtual bool _mayExec(Game &game, unsigned int time) const;

private:
  ChangeFishery();

  class Impl;
  ScopedPtr<Impl> _d;
};

}//namespace events

#endif //_CAESARIA_RANDOMFIRE_EVENT_H_INCLUDE_
