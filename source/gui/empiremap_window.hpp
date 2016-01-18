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

#ifndef __CAESARIA_EMPIREMAP_WINDOW_H_INCLUDED__
#define __CAESARIA_EMPIREMAP_WINDOW_H_INCLUDED__

#include "widget.hpp"
#include "game/predefinitions.hpp"

namespace gui
{

class EmpireMapWindow : public Widget
{
public:
  typedef enum { showCityInfo=0x1 } Flag;

  EmpireMapWindow( Widget* parent, int id, PlayerCityPtr city );
  static EmpireMapWindow* create( PlayerCityPtr city, Widget* parent, int id );

  virtual ~EmpireMapWindow();

  // draw on screen
  virtual void draw( gfx::Engine& engine );
  virtual void beforeDraw(gfx::Engine &painter);

  void setFlag(Flag flag, bool value);

  //resolve event
  virtual bool onEvent(const NEvent& event);

protected:
  void _changePosition();
  virtual bool _onMousePressed( const NEvent::Mouse& event);
  const Point& _offset() const;

  Widget* _resetInfoPanel();
  void _toggleAi();

  class Impl;
  ScopedPtr< Impl > _d;
};

}//end namespace gui
#endif //__CAESARIA_EMPIREMAP_WINDOW_H_INCLUDED__
