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

#ifndef __CAESARIA_WIDGETANIMATOR_H_INCLUDE_
#define __CAESARIA_WIDGETANIMATOR_H_INCLUDE_

#include "gui/widget.hpp"
#include "core/flagholder.hpp"
#include "core/signals.hpp"

#define ANIMATOR_UNUSE_VALUE -9999

namespace gui
{

class WidgetAnimator : public Widget, public FlagHolder<int>
{
public:
	enum { showParent=0x1,
         removeSelf=0x2, removeParent=0x4,
         debug=0x8,
         isActive=0x10 };

  WidgetAnimator(Widget* parent, int flags);

  void beforeDraw(gfx::Engine& painter);
  void setFlag(const std::string& flagname, bool enabled);

public signals:
  Signal0<>& onFinish() { return _onFinishSignal; }
  Signal1<Widget*>& onFinishEx() { return _onFinishExSignal; }

protected:
  virtual void _afterFinished();
  Signal0<> _onFinishSignal;
  Signal1<Widget*> _onFinishExSignal;
};

}//end namespace gui
#endif //__CAESARIA_WIDGETANIMATOR_H_INCLUDE_
