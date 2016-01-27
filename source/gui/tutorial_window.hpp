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

#ifndef _CAESARIA_TUTORIAL_WINDOW_H_INCLUDE_
#define _CAESARIA_TUTORIAL_WINDOW_H_INCLUDE_

#include "window.hpp"
#include "gameautopause.hpp"
#include "sound/engine.hpp"

namespace gui
{

class TutorialWindow : public Window
{
public:
  TutorialWindow(Widget* parent, vfs::Path tutorial );
  virtual ~TutorialWindow();

private:
  GameAutoPause _locker;
  audio::Muter _muter;
  audio::SampleDeleter _speechDel;
};

}//end namespace gui
#endif //_CAESARIA_TUTORIAL_WINDOW_H_INCLUDE_
