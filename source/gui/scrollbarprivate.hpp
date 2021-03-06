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

#ifndef _CAESARIA_SCROLLBAR_PRIVATE_H_INCLUDE_
#define _CAESARIA_SCROLLBAR_PRIVATE_H_INCLUDE_

#include "gfx/picture.hpp"
#include "core/signals.hpp"
#include "core/rectangle.hpp"
#include "gui/pushbutton.hpp"

namespace gui
{

class ScrollBar::Impl
{
signals public:
	Signal1<int> onPositionChanged;

public:
  Rect sliderTextureRect;
  Rect backgroundRect;
  NColor  overrideBgColor;
  int  value;
  Rect filledAreaRect;
  bool needRecalculateParams;
  Point cursorPos;
  unsigned int lastTimeChange;

  struct {
    gfx::Picture texture;
    Rect rect;
  } background;

  struct {
    struct _ButtonInfo{
      PushButton* widget = nullptr;
      bool visible() const { return widget && widget->visible(); }
      void setEnabled(bool enable) { if(widget) widget->setEnabled( enable); }
    };

    _ButtonInfo down;
    _ButtonInfo up;
  } button;

  struct {
    gfx::Picture texture;
    Rect         rect;
    gfx::Picture pressed;
    gfx::Picture normal;
  } slider;
};

}//end namespace gui

#endif //_CAESARIA_SCROLLBAR_PRIVATE_H_INCLUDE_
