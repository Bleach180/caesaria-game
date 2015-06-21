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

#ifdef _MSC_VER
#include <cmath>
#endif

#include "widget.hpp"
#include "widgetprivate.hpp"
#include "environment.hpp"
#include "core/event.hpp"
#include "core/variant_map.hpp"
#include "core/variant_list.hpp"
#include "core/foreach.hpp"
#include "core/saveadapter.hpp"
#include "core/utils.hpp"
#include "core/logger.hpp"
#include "core/gettext.hpp"
#include "rect_calc.hpp"

namespace gui
{

static const Variant invalidVariant;
CAESARIA_LITERALCONST(vars)

void Widget::beforeDraw(gfx::Engine& painter )
{
  __D_IMPL(d,Widget)
  if( !d->parent )
    return;
  //"Parent must be exists";

  foreach( widget, d->children ) { (*widget)->beforeDraw( painter ); }
}

Ui* Widget::ui() {  return _environment; }

void Widget::setTextAlignment(align::Type horizontal, align::Type vertical )
{
  if( !(horizontal >= align::upperLeft && horizontal <= align::automatic)
     || !(vertical >= align::upperLeft && vertical <= align::automatic) )
  {
    Logger::warning( "Unknown align in SetTextAlignment" );
    return;
  }

  __D_IMPL(d,Widget)
  d->textHorzAlign = horizontal;
  d->textVertAlign = vertical;
}

void Widget::setMaxWidth( unsigned int width ) { __D_IMPL(d,Widget) d->maxSize.setWidth( width );}
unsigned int Widget::height() const{    return relativeRect().height(); }

Widget::Widget( Widget* parent, int id, const Rect& rectangle )
: __INIT_IMPL(Widget),
  _environment( parent ? parent->ui() : 0 )
{
  __D_IMPL(_d,Widget)
  _d->alignLeft = align::upperLeft;
  _d->alignRight = align::upperLeft;
  _d->alignTop = align::upperLeft;
  _d->alignBottom = align::upperLeft;
  _d->isVisible = true;
  _d->maxSize = Size(0,0);
  _d->minSize = Size(1,1);
  _d->parent = parent;
  _d->id = id;
  _d->isEnabled = true;
  _d->isSubElement = false;
  _d->noClip = false;
  _d->tabOrder = -1;
  _d->isTabGroup = false;
  _d->relativeRect = rectangle;
  _d->absoluteRect = rectangle;
  _d->absoluteClippingRect = rectangle;
  _d->desiredRect = rectangle;
  _d->isTabStop = false;

#ifdef _DEBUG
  setDebugName( "AbstractWidget" );
#endif

  // if we were given a parent to attach to
  if( parent )
  {
    parent->_addChild(this);
    _recalculateAbsolutePosition(true);
    drop();
  }

  setTextAlignment( align::upperLeft, align::center );
}

Widget::~Widget()
{
  // delete all children
  foreach( widget, _dfunc()->children )
  {
    (*widget)->setParent( 0 );
    (*widget)->drop();
  }
}

void Widget::setGeometry( const Rect& r, GeometryType mode )
{
  __D_IMPL(_d,Widget)
  if( parent() )
  {
    const Rect& r2 = parent()->absoluteRect();
    SizeF d = r2.size().toSizeF();

    if( _d->alignLeft == align::scale)
      _d->scaleRect.setLeft( (float)r.left() / d.width() );
    if (_d->alignRight == align::scale)
      _d->scaleRect.setRight( (float)r.right() / d.width() );
    if (_d->alignTop == align::scale)
      _d->scaleRect.setTop( (float)r.top() / d.height() );
    if (_d->alignBottom == align::scale)
      _d->scaleRect.setBottom(  (float)r.bottom() / d.height() );
  }

  _d->desiredRect = r;
  updateAbsolutePosition();
}

void Widget::_finalizeResize() {}
void Widget::_finalizeMove() {}

Widget::Widgets& Widget::_getChildren() {  return _dfunc()->children;}

void Widget::setPosition( const Point& position )
{
	const Rect rectangle( position, size() );
	setGeometry( rectangle );
}

void Widget::setGeometry( const RectF& r, GeometryType mode )
{
  if( !parent() )
    return;

  __D_IMPL(_d,Widget)
  const Size& d = parent()->size();

  switch( mode )
  {
  case ProportionalGeometry:
    _d->desiredRect = Rect(
          floor( d.width() * r.left() ),
          floor( d.height() * r.top() ),
          floor( d.width() * r.right() ),
          floor( d.height() * r.bottom() ));

    _d->scaleRect = r;
  break;

  default:
  break;
  }

  updateAbsolutePosition();
}

Rect Widget::absoluteRect() const { __D_IMPL_CONST(_d,Widget)   return _d->absoluteRect;}
Rect Widget::absoluteClippingRect() const{ __D_IMPL_CONST(_d,Widget)   return _d->absoluteClippingRect;}

void Widget::setNotClipped( bool noClip )
{
  __D_IMPL(_d,Widget)
  _d->noClip = noClip;
  updateAbsolutePosition();
}

void Widget::setMaxSize( const Size& size )
{
  __D_IMPL(_d,Widget)
  _d->maxSize = size;
  updateAbsolutePosition();
}

void Widget::setMinSize( const Size& size )
{
  __D_IMPL(_d,Widget)
  _d->minSize = size;
  if( _d->minSize.width() < 1)
      _d->minSize.setWidth( 1 );

  if( _d->minSize.height() < 1)
      _d->minSize.setHeight( 1 );

  updateAbsolutePosition();
}

void Widget::setAlignment( Alignment left, Alignment right, Alignment top, Alignment bottom )
{
  __D_IMPL(_d,Widget)
  _d->alignLeft = left;
  _d->alignRight = right;
  _d->alignTop = top;
  _d->alignBottom = bottom;

  if( parent() )
  {
    Rect r( parent()->absoluteRect() );

    SizeF d = r.size().toSizeF();

    RectF dRect = _d->desiredRect.toRectF();
    if( _d->alignLeft == align::scale)
      _d->scaleRect.setLeft( dRect.left() / d.width() );
    if(_d->alignRight == align::scale)
      _d->scaleRect.setRight( dRect.right() / d.width() );
    if( _d->alignTop  == align::scale)
      _d->scaleRect.setTop( dRect.top() / d.height() );
    if (_d->alignBottom == align::scale)
      _d->scaleRect.setBottom( dRect.bottom() / d.height() );
  }
}

void Widget::updateAbsolutePosition()
{
  __D_IMPL(_d,Widget)
  _recalculateAbsolutePosition(false);

  // update all children
  foreach( widget, _d->children ) { (*widget)->updateAbsolutePosition(); }
}

Widget* Widget::getElementFromPoint( const Point& point )
{
  Widget* target = 0;
  __D_IMPL(_d,Widget)
  // we have to search from back to front, because later children
  // might be drawn over the top of earlier ones.

  ChildIterator it = _d->children.getLast();

  if (visible())
  {
    while(it != _d->children.end())
    {
      target = (*it)->getElementFromPoint(point);
      if( target )
      {
        return target;
      }

      --it;
    }
  }

  if( visible() && isPointInside(point) )
  {
    target = this;
  }

  return target;
}

bool Widget::isPointInside( const Point& point ) const
{
  return absoluteClippingRect().isPointInside(point);
}

void Widget::addChild( Widget* child )
{
  _addChild(child);
  if (child)
  {
    child->updateAbsolutePosition();
  }
}

void Widget::removeChild( Widget* child )
{
  __D_IMPL(_d,Widget)
  ChildIterator it = _d->children.begin();
  for (; it != _d->children.end(); ++it)
    if ((*it) == child)
    {
      (*it)->setParent( 0 );
      (*it)->drop();
      _d->children.erase(it);
      return;
    }
}

void Widget::draw(gfx::Engine& painter )
{
  if ( visible() )
  {
    Widgets& children = _getChildren();
    foreach( widget, children ) { (*widget)->draw( painter ); }
  }
}

void Widget::setTabOrder( int index )
{
  __D_IMPL(_d,Widget)
  // negative = autonumber
  if (index < 0)
  {
    _d->tabOrder = 0;
    Widget *el = tabgroup();
    while( _d->isTabGroup && el && el->parent() )
        el = el->parent();

    Widget *first=0, *closest=0;
    if (el)
    {
        // find the highest element number
        el->next(-1, true, _d->isTabGroup, first, closest, true);
        if (first)
        {
            _d->tabOrder = first->tabOrder() + 1;
        }
    }
  }
  else
  {
    _d->tabOrder = index;
  }
}

int Widget::tabOrder() const{  return _dfunc()->tabOrder;}

Widget* Widget::tabgroup()
{
  Widget *ret=this;

  while (ret && !ret->hasTabgroup())
      ret = ret->parent();

  return ret;
}

bool Widget::enabled() const
{
  __D_IMPL_CONST(_d,Widget)
  if ( isSubElement() && _d->isEnabled && parent() )
  {
    return parent()->enabled();
  }

  return _d->isEnabled;
}

bool Widget::bringToFront()
{
	if( parent() )
	{
		return parent()->bringChildToFront( this );
	}

	return false;
}

bool Widget::bringChildToFront( Widget* element )
{
  Widgets& children = _getChildren();
  foreach( it, children )
  {
    if (element == (*it))
    {
      children.erase(it);
      children.push_back(element);
      return true;
    }
  }

  return false;
}

bool Widget::sendChildToBack( Widget* child )
{
  Widgets& children = _getChildren();
  ChildIterator it = children.begin();
  if (child == (*it))	// already there
      return true;

  for (; it != children.end(); ++it)
  {
    if (child == (*it))
    {
      children.erase(it);
      children.push_front(child);
      return true;
    }
  }

  return false;
}

bool Widget::sendToBack()
{
	if( parent() )
	{
		return parent()->sendChildToBack( this );
	}

	return false;
}

Widget* Widget::findChild( int id, bool searchchildren/*=false*/ ) const
{
  Widget* e = 0;

  __D_IMPL_CONST(_d,Widget)
  foreach( widget, _d->children )
  {
    if( (*widget)->ID() == id)
    {
      return *widget;
    }

    if( searchchildren )
    {
      e = (*widget)->findChild(id, true);
    }

    if( e )
    {
      return e;
    }
  }

  return e;
}

bool Widget::next( int startOrder, bool reverse, bool group, Widget*& first, Widget*& closest, bool includeInvisible/*=false*/ ) const
{
    // we'll stop searching if we find this number
    int wanted = startOrder + ( reverse ? -1 : 1 );
    if (wanted==-2)
        wanted = 1073741824; // maximum int

    __D_IMPL_CONST(_d,Widget)
    ConstChildIterator it = _d->children.begin();

    int closestOrder, currentOrder;

    while(it != _d->children.end())
    {
        // ignore invisible elements and their children
        if ( ( (*it)->visible() || includeInvisible ) &&
             (group == true || (*it)->hasTabgroup() == false) )
        {
            // only check tab stops and those with the same group status
            if ((*it)->isTabStop() && ((*it)->hasTabgroup() == group))
            {
                currentOrder = (*it)->tabOrder();

                // is this what we're looking for?
                if (currentOrder == wanted)
                {
                  closest = *it;
                  return true;
                }

                // is it closer than the current closest?
                if (closest)
                {
                  closestOrder = closest->tabOrder();
                  if ( ( reverse && currentOrder > closestOrder && currentOrder < startOrder)
                      ||(!reverse && currentOrder < closestOrder && currentOrder > startOrder))
                  {
                    closest = *it;
                  }
                }
                else if ( (reverse && currentOrder < startOrder) || (!reverse && currentOrder > startOrder) )
                {
                  closest = *it;
                }

                // is it before the current first?
                if (first)
                {
                  closestOrder = first->tabOrder();

                  if ( (reverse && closestOrder < currentOrder) || (!reverse && closestOrder > currentOrder) )
                  {
                      first = *it;
                  }
                }
                else
                {
                  first = *it;
                }
            }
            // search within children
            if ((*it)->next(startOrder, reverse, group, first, closest))
            {
              return true;
            }
        }
        ++it;
    }
    return false;
}

void Widget::setParent(Widget* p) {  _dfunc()->parent = p; }

static int __convStr2RelPos( Widget* w, const VariantMap& vars, std::string s )
{
  s = utils::trim( s );

  WidgetCalc wcalc( *w, vars );
  return wcalc.eval( s );
}

void Widget::setupUI( const VariantMap& options )
{
  __D_IMPL(_d,Widget)
  _d->internalName = options.get( "name" ).toString();
  align::Helper ahelper;
  VariantList textAlign = options.get( "textAlign" ).toList();
  VariantList altAlign = options.get( "text.align" ).toList();
  if( !altAlign.empty() )
  {
    textAlign = altAlign;
  }

  if( textAlign.size() > 1 )
  {
    setTextAlignment( ahelper.findType( textAlign.front().toString() ),
                      ahelper.findType( textAlign.back().toString() ) );
  }

  Variant tmp;
  setID( (int)options.get( "id", _d->id ) );
  setText( _( options.get( "text" ).toString() ) );
  setTooltipText( options.get( "tooltip" ).toString() );
  setVisible( options.get( "visible", true ).toBool() );
  setEnabled( options.get( "enabled", true ).toBool() );
  _d->isTabStop = options.get( "tabStop", false ).toBool();
  _d->isTabGroup = options.get( "tabGroup", -1 ).toInt();
  _d->tabOrder = options.get( "tabOrder", -1 ).toInt();
  setMaxSize( options.get( "maximumSize", Size( 0 ) ).toSize() );
  setMinSize( options.get( "minimumSize", Size( 1 ) ).toSize() );
  VariantMap vars = options.get( literals::vars ).toMap();

  VariantList aRectList = options.get( "geometry" ).toList();
  if( !aRectList.empty() )
  {
    Rect cRect(
       __convStr2RelPos( this, vars, aRectList.get( 0 ).toString() ),
       __convStr2RelPos( this, vars, aRectList.get( 1 ).toString() ),
       __convStr2RelPos( this, vars, aRectList.get( 2 ).toString() ),
       __convStr2RelPos( this, vars, aRectList.get( 3 ).toString() ) );

    setGeometry( cRect );
  }

  tmp = options.get( "geometryf" );
  if( tmp.isValid() )
  {
    RectF r = tmp.toRectf();
    if( r.width() > 1 && r.height() > 1)
    {
      r = RectF( 0, 0, 1, 1 );
      Logger::warning( "Incorrect geometryf values [%f, %f, %f, %f]",
                       r.left(), r.top(), r.right(), r.bottom() );
    }

    setGeometry( r );
  }

  tmp = options.get( "size" );
  if( tmp.isValid() )
  {
    Size s = tmp.toSize();
    if( s.width() > 0 ) { setWidth( s.width()); }
    if( s.height() > 0 ) { setHeight( s.height() ); }
  }

  setNotClipped( options.get( "noclipped", false ).toBool() );

  foreach( it, options )
  {
    if( it->second.type() != Variant::Map )
      continue;

    VariantMap tmp = it->second.toMap();
    tmp[ literals::vars ] = vars;

    std::string widgetName = it->first;
    std::string widgetType;
    std::string::size_type delimPos = widgetName.find( '#' );
    if( delimPos != std::string::npos )
    {
      widgetType = widgetName.substr( delimPos+1 );
      widgetName = widgetName.substr( 0, delimPos );
    }
    else
    {
      widgetType = tmp.get( "type" ).toString();
    }

    if( !widgetType.empty() )
    {
      Widget* child = ui()->createWidget( widgetType, this );
      if( child )
      {
        child->setupUI( tmp );
        if( child->internalName().empty() )
        {
          child->setInternalName( widgetName );
        }
      }
    }
  }

  Variant positionV = options.get( "position" );
  if( positionV.isValid() )
    move( positionV.toPoint() );

  _d->properties = options.get( "properties" ).toMap();
}

void Widget::setupUI(const vfs::Path& filename)
{
  Logger::warning( "Widget: load gui model from " + filename.toString() );
  setupUI( config::load( filename ) );
}

void Widget::_addChild( Widget* child )
{
  if (child)
  {
    child->grab(); // prevent destruction when removed
    child->remove(); // remove from old parent
    child->_dfunc()->lastParentRect = absoluteRect();
    child->setParent( this );
    _dfunc()->children.push_back(child);
  }
}

void Widget::_recalculateAbsolutePosition( bool recursive )
{
  Rect parentAbsolute(0,0,0,0);
  Rect parentAbsoluteClip;
  float fw=0.f, fh=0.f;

  __D_IMPL(_d,Widget)
  if ( parent() )
  {
    parentAbsolute = parent()->absoluteRect();

    if( _d->noClip )
    {
      Widget* p=this;
      while( p && p->parent() )
          p = p->parent();

      parentAbsoluteClip = p->absoluteClippingRect();
    }
    else
      parentAbsoluteClip = parent()->absoluteClippingRect();
  }

  const int diffx = parentAbsolute.width() - _d->lastParentRect.width();
  const int diffy = parentAbsolute.height() - _d->lastParentRect.height();


  if( _d->alignLeft == align::scale || _d->alignRight == align::scale)
      fw = (float)parentAbsolute.width();

  if( _d->alignTop == align::scale || _d->alignBottom == align::scale)
      fh = (float)parentAbsolute.height();

  switch( _d->alignLeft)
  {
  case align::automatic:
  case align::upperLeft: break;
  case align::lowerRight: _d->desiredRect._lefttop += Point( diffx, 0 ); break;
  case align::center: _d->desiredRect._lefttop += Point( diffx/2, 0 ); break;
  case align::scale: _d->desiredRect.setLeft( _d->scaleRect.left() * fw ); break;
  }

  switch( _d->alignRight)
  {
  case align::automatic:
  case align::upperLeft:   break;
  case align::lowerRight: _d->desiredRect._bottomright += Point( diffx, 0 ); break;
  case align::center: _d->desiredRect._bottomright += Point( diffx/2, 0 ); break;
  case align::scale: _d->desiredRect.setRight( roundf( _d->scaleRect.right() * fw ) ); break;
  }

  switch( _d->alignTop)
  {
  case align::automatic:
  case align::upperLeft: break;
  case align::lowerRight: _d->desiredRect._lefttop += Point( 0, diffy ); break;
  case align::center: _d->desiredRect._lefttop += Point( 0, diffy/2 ); break;
  case align::scale: _d->desiredRect.setTop( roundf(_d->scaleRect.top() * fh) ); break;
  }

  switch( _d->alignBottom)
  {
  case align::automatic:
  case align::upperLeft:  break;
  case align::lowerRight: _d->desiredRect._bottomright += Point( 0, diffy );  break;
  case align::center:  _d->desiredRect._bottomright += Point( 0, diffy/2 );  break;
  case align::scale: _d->desiredRect.setBottom( roundf(_d->scaleRect.bottom() * fh) );  break;
  }

  _d->relativeRect = _d->desiredRect;

  const int w = _d->relativeRect.width();
  const int h = _d->relativeRect.height();

  // make sure the desired rectangle is allowed
  if (w < (int)_d->minSize.width() )
      _d->relativeRect.setRight( _d->relativeRect.left() + _d->minSize.width() );
  if (h < (int)_d->minSize.height() )
      _d->relativeRect.setBottom( _d->relativeRect.top() + _d->minSize.height() );
  if (_d->maxSize.width() > 0 && w > (int)_d->maxSize.width() )
      _d->relativeRect.setRight( _d->relativeRect.left() + _d->maxSize.width() );
  if (_d->maxSize.height() > 0 && h > (int)_d->maxSize.height() )
      _d->relativeRect.setBottom( _d->relativeRect.left() + _d->maxSize.height() );

  _d->relativeRect.repair();

  _d->absoluteRect = _d->relativeRect + parentAbsolute.lefttop();

  if (!parent())
      parentAbsoluteClip = absoluteRect();

  _d->absoluteClippingRect = absoluteRect();
  _d->absoluteClippingRect.clipAgainst(parentAbsoluteClip);

  _d->lastParentRect = parentAbsolute;

  if ( recursive )
  {
    // update all children
    foreach( it, _d->children )
    {
        (*it)->_recalculateAbsolutePosition(recursive);
    }
  }

  _finalizeResize();
}

void Widget::animate( unsigned int timeMs )
{
  if( !visible() )
    return;

  foreach( widget, _getChildren() ) { (*widget)->animate( timeMs ); }
}

void Widget::remove()
{
  //"parent must be exist for element"
  if( parent() )
      parent()->removeChild( this );
}

bool Widget::onEvent( const NEvent& event )
{
  foreach( item, _dfunc()->eventHandlers )
  {
    bool handled = (*item)->onEvent( event );
    if( handled )
      return true;
  }

  if (event.EventType == sEventMouse)
    if (parent() && (parent()->parent() == NULL))
      return true;

  return parent() ? parent()->onEvent(event) : false;
}

bool Widget::isMyChild( Widget* child ) const
{
  if (!child)
    return false;

  do
  {
    if( child->parent() )
     child = child->parent();

  } while (child->parent() && child != this);

  return child == this;
}

void Widget::setWidth( unsigned int width )
{
  const Rect rectangle( relativeRect().lefttop(), Size( width, height() ) );
  setGeometry( rectangle );
}

void Widget::setHeight( unsigned int height )
{
  const Rect rectangle( relativeRect().lefttop(), Size( width(), height ) );
  setGeometry( rectangle );
}

void Widget::setEnabled(bool enabled){  _dfunc()->isEnabled = enabled;}
std::string Widget::internalName() const{    return _dfunc()->internalName;}
void Widget::setInternalName( const std::string& name ){    _dfunc()->internalName = name;}
Widget* Widget::parent() const {    return _dfunc()->parent;}
Rect Widget::relativeRect() const{  return _dfunc()->relativeRect;}
bool Widget::isNotClipped() const{  return _dfunc()->noClip;}
void Widget::setVisible( bool visible ){  _dfunc()->isVisible = visible;}
bool Widget::isTabStop() const{  return _dfunc()->isTabStop;}
bool Widget::hasTabgroup() const{  return _dfunc()->isTabGroup;}
void Widget::setText( const std::string& text ){  _dfunc()->text = text;}
void Widget::setTooltipText( const std::string& text ) {  _dfunc()->toolTipText = text;}
std::string Widget::text() const{  return _dfunc()->text;}
std::string Widget::tooltipText() const{  return _dfunc()->toolTipText;}
int Widget::ID() const{  return _dfunc()->id;}
void Widget::setID( int id ) {  _dfunc()->id = id; }
const Widget::Widgets& Widget::children() const{  return _dfunc()->children;}
Size Widget::maxSize() const{    return _dfunc()->maxSize;}
Size Widget::minSize() const{    return _dfunc()->minSize;}
bool Widget::isHovered() const{  return _environment->isHovered( this );}
bool Widget::isFocused() const{  return _environment->hasFocus( this );}
Rect Widget::clientRect() const{  return Rect( 0, 0, width(), height() );}
void Widget::setFocus(){  ui()->setFocus( this );}
void Widget::removeFocus(){  ui()->removeFocus( this );}
Rect& Widget::absoluteClippingRectRef() const { return _dfunc()->absoluteClippingRect; }
unsigned int Widget::width() const{  return relativeRect().width();}
Size Widget::size() const{  return Size( _dfunc()->relativeRect.width(), _dfunc()->relativeRect.height() );}
int Widget::screenTop() const { return absoluteRect().top(); }
int Widget::screenLeft() const { return absoluteRect().left(); }
int Widget::screenBottom() const { return absoluteRect().bottom(); }
int Widget::screenRight() const { return absoluteRect().right(); }
Point Widget::lefttop() const { return Point( left(), top() ); }
Point Widget::leftbottom() const { return Point( left(), bottom() ); }
Point Widget::righttop() const { return Point( right(), top() ); }
Point Widget::rightbottom() const { return Point( right(), bottom() ); }
Point Widget::localToScreen( const Point& localPoint ) const{  return localPoint + _dfunc()->absoluteRect.lefttop();}
Rect Widget::localToScreen( const Rect& localRect ) const{  return localRect + _dfunc()->absoluteRect.lefttop();}
Point Widget::screenToLocal(const Point &screenPoint) const { return screenPoint - _dfunc()->absoluteRect.lefttop(); }
void Widget::move( const Point& relativeMovement ){  setGeometry( _dfunc()->relativeRect + relativeMovement );}
int Widget::bottom() const{  return _dfunc()->relativeRect.bottom(); }
Point Widget::center() const { return (_dfunc()->relativeRect.rightbottom() + _dfunc()->relativeRect.lefttop()) / 2; }
void Widget::setTabgroup( bool isGroup ) { _dfunc()->isTabGroup = isGroup; }
bool Widget::visible() const{  return _dfunc()->isVisible;}
bool Widget::isSubElement() const{  return _dfunc()->isSubElement;}
void Widget::setSubElement( bool subElement ){  _dfunc()->isSubElement = subElement;}
void Widget::setTabStop( bool enable ){  _dfunc()->isTabStop = enable;}
void Widget::setLeft( int newLeft ) { setPosition( Point( newLeft, top() ) ); }
void Widget::setTop( int newTop ) { setPosition( Point( left(), newTop ) );  }
int Widget::top() const { return relativeRect().top(); }
int Widget::left() const { return relativeRect().left(); }
int Widget::right() const { return relativeRect().right(); }
void Widget::hide() { setVisible( false ); }
void Widget::show() {  setVisible( true ); }
Alignment Widget::horizontalTextAlign() const{  return _dfunc()->textHorzAlign; }
Alignment Widget::verticalTextAlign() const{  return _dfunc()->textVertAlign;}
void Widget::deleteLater(){  _environment->deleteLater( this ); }

void Widget::setRight( int newRight )
{
  Rect r = relativeRect();
  r.rright() = newRight;
  setGeometry( r );
}

void Widget::addProperty(const std::string& name, const Variant& value)
{
  _dfunc()->properties[ name ] = value;
}

const Variant& Widget::getProperty(const std::string& name) const
{
  VariantMap::const_iterator it = _dfunc()->properties.find( name );
  return it != _dfunc()->properties.end() ? it->second : invalidVariant;
}

void Widget::installEventHandler( Widget* elementHandler )
{
  _dfunc()->eventHandlers.insert( elementHandler );
}

void Widget::setCenter(Point center)
{
  Rect newRect( Point( center.x() - width() / 2, center.y() - height() / 2), size() );
  setGeometry( newRect );
}

void Widget::setBottom( int b )
{
  Rect r = _dfunc()->relativeRect;
  r.setBottom(  b );
  setGeometry( r );
}

}//end namespace gui
