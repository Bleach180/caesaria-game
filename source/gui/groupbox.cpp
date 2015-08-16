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

#include "groupbox.hpp"
#include "gfx/decorator.hpp"
#include "gfx/picture.hpp"
#include "gfx/engine.hpp"
#include "core/variant_map.hpp"
#include "widget_factory.hpp"

using namespace gfx;

namespace gui
{

REGISTER_CLASS_IN_WIDGETFACTORY(GroupBox)

class GroupBox::Impl
{
public:
  Batch background;
  Pictures backgroundNb;
  Picture backgroundImage;
	bool scaleImage;
  GroupBox::Style style;
  bool needUpdateTexture;
};

//! constructor
GroupBox::GroupBox(Widget *parent)
  : Widget( parent, -1, Rect( 0, 0, 1, 1 ) ), _d( new Impl )
{
  #ifdef _DEBUG
      setDebugName("GroupBox");
  #endif

  _d->scaleImage = true;
  _d->needUpdateTexture = true;
  _d->style = blackFrame;
}

GroupBox::GroupBox( Widget* parent, const Rect& rectangle, int id, Style style)
: Widget( parent, id, rectangle ), _d( new Impl )
{
	#ifdef _DEBUG
    	setDebugName("GroupBox");
	#endif

  _d->scaleImage = true;	
  _d->needUpdateTexture = true;
  _d->style = style;
}

//! destructor
GroupBox::~GroupBox() {}

//! draws the element and its children
void GroupBox::draw(gfx::Engine& painter )
{
  if (!visible())
      return;

  if( _d->backgroundImage.isValid() )
  {
    painter.draw( _d->backgroundImage, absoluteRect().lefttop(), &absoluteClippingRectRef() );
  }
  else
  {
    if( _d->background.valid() )
      painter.draw( _d->background, &absoluteClippingRectRef() );
    else
      painter.draw( _d->backgroundNb, absoluteRect().lefttop(), &absoluteClippingRectRef() );
  }

  Widget::draw( painter );
}

//! Returns true if the image is scaled to fit, false if not
bool GroupBox::isBackgroundImageScaled() const {	return _d->scaleImage; }

void GroupBox::setBackgroundImage( const Picture& image )
{
  _d->backgroundImage = image;
  _d->needUpdateTexture = true;
}

const Picture& GroupBox::backgroundImage() const {  return _d->backgroundImage; }
void GroupBox::setScaleBackgroundImage( bool scale ) { _d->scaleImage = scale; }

void GroupBox::setStyle( Style style )
{
  _d->style = style;
  _d->needUpdateTexture = true;
}

void GroupBox::beforeDraw(gfx::Engine& painter )
{
  if( _d->needUpdateTexture )
  {
    _d->needUpdateTexture = false;

    if( !_d->backgroundImage.isValid() )
    {
      Decorator::Mode styles[] = { Decorator::whiteFrame, Decorator::blackFrame, Decorator::pure };

      Pictures pics;
      Decorator::draw( pics, Rect( Point( 0, 0 ), size() ),
                       Decorator::Mode( styles[ math::clamp<int>( _d->style, 0, count ) ] ), Decorator::normalY );

      bool batchOk = _d->background.load( pics, absoluteRect().lefttop() );
      if( !batchOk )
      {
        _d->background.destroy();
        Decorator::reverseYoffset( pics );
        _d->backgroundNb = pics;
      }
    }
  }

  Widget::beforeDraw( painter );
}

void GroupBox::setupUI(const VariantMap &ui)
{
  Widget::setupUI( ui );

  std::string style = ui.get( "bgtype" ).toString();
  if( style == "blackFrame" ) _d->style = blackFrame;
  else if( style == "whiteFrame" ) _d->style = whiteFrame;
  else if( style == "none" ) _d->style = none;
  _d->needUpdateTexture = true;
}

}//end namespace gui
