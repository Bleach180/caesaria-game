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

#ifndef _CAESARIA_EVENT_SHOWTILEINFO_H_INCLUDE_
#define _CAESARIA_EVENT_SHOWTILEINFO_H_INCLUDE_

#include "event.hpp"
#include "core/position.hpp"

namespace events
{

class ShowTileInfo : public GameEvent
{
public:
  typedef enum { current=0, next, prew, count } Mode;
  static GameEventPtr create( TilePos pos, Mode mode=current );

protected:
  virtual void _exec( Game& game, unsigned int );
  virtual bool _mayExec(Game&, unsigned int) const;

private:
  ShowTileInfo();

  TilePos _pos;
  Mode _mode;
};

}

#endif //_CAESARIA_EVENT_SHOWTILEINFO_H_INCLUDE_
