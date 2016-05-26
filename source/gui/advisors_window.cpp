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

#include "advisors_window.hpp"

#include "gfx/picture.hpp"
#include "gfx/pictureconverter.hpp"
#include "gfx/engine.hpp"
#include "core/event.hpp"
#include "texturedbutton.hpp"
#include "game/resourcegroup.hpp"
#include "gfx/decorator.hpp"
#include "label.hpp"
#include "city/city.hpp"
#include "core/utils.hpp"
#include "core/gettext.hpp"
#include "core/logger.hpp"
#include "advisor_employers_window.hpp"
#include "advisor_legion_window.hpp"
#include "advisor_emperor_window.hpp"
#include "advisor_trade_window.hpp"
#include "advisor_finance_window.hpp"
#include "game/funds.hpp"
#include "events/event.hpp"
#include "city/requestdispatcher.hpp"
#include "image.hpp"
#include "events/script_event.hpp"
#include "gameautopause.hpp"
#include "events/fundissue.hpp"
#include "core/smartlist.hpp"
#include "objects/military.hpp"
#include "widgetescapecloser.hpp"
#include "events/showempiremapwindow.hpp"
#include "advisor_population_window.hpp"
#include "widget_helper.hpp"
#include "world/empire.hpp"
#include "city/statistic.hpp"

using namespace gfx;
using namespace events;

namespace gui
{

namespace advisorwnd
{

PushButton& Parlor::_addButton( Advisor advisorName, int picId, std::string tooltip )
{
  Point tabButtonPos( (width() - 636) / 2 + 10, height() / 2 + 192 + 10);

  auto& btn = add<TexturedButton>( tabButtonPos + Point( 48, 0 ) * (advisorName-1), Size(40, 40),
                                   advisorName, TexturedButton::States( picId, picId, picId + 13 ) );
  btn.setIsPushButton( true );
  btn.setTooltipText( tooltip );
  return btn;
}

void Parlor::_initButtons()
{
  if( _model.isNull() )
  {
    Logger::warning( "Parlor model is null. Cant init buttons" );
    return;
  }

  auto buttons = children().select<PushButton>();
  for( auto& btn : buttons )
    btn->deleteLater();

  for( auto& item : _model->items() )
    _addButton( item.type, item.pic, _( fmt::format( "##visit_{}_advisor##", item.tooltip ) ) );

  auto& btn = _addButton( advisor::unknown, 609 );
  btn.setIsPushButton( false );

  CONNECT_LOCAL( &btn, onClicked(), Parlor::deleteLater );
}

Parlor::Parlor( Widget* parent, int id )
  : Window( parent, Rect( Point(0, 0), parent->size() ), "", id )
{
  setupUI( ":/gui/advisors.gui" );

  GameAutoPauseWidget::insertTo( this );
  WidgetClosers::insertTo( this );

  INIT_WIDGET_FROM_UI( Image*, imgBgButtons )
  if( imgBgButtons )
    imgBgButtons->setPosition( Point( (width() - 636) / 2, height() / 2 + 192) );

  setInternalName("ParlorWindow");
}

void Parlor::setModel(ParlorModel* model)
{
  _model.reset( model );
  model->setParent( this );
  _initButtons();

  CONNECT( this, onSwitchAdvisor, model, ParlorModel::switchAdvisor );
}

void Parlor::draw(gfx::Engine& engine )
{
  if( !visible() )
    return;

  Window::draw( engine );
}

bool Parlor::onEvent( const NEvent& event )
{
  if( event.EventType == sEventMouse && event.mouse.type == NEvent::Mouse::mouseRbtnRelease )
  {
    deleteLater();
    return true;
  }

  if( event.EventType == sEventGui && event.gui.type == guiButtonClicked )
  {
    int id = event.gui.caller->ID();
    if( id >= 0 && id < advisor::unknown )
    {
      emit onSwitchAdvisor( Advisor( event.gui.caller->ID() ) );
    }
  }

  return Widget::onEvent( event );
}

void Parlor::showAdvisor(Advisor type) { emit onSwitchAdvisor(type); }

Parlor* Parlor::create(Widget* parent, int id, const Advisor type, ParlorModel* model )
{
  Parlor* ret = new Parlor( parent, id );
  ret->setModel( model );
  ret->showAdvisor( type );

  return ret;
}

class ParlorModel::Impl
{
public:
  Widget* parent;
  Base* advisorPanel;

  Point offset;
  PlayerCityPtr city;
};

ParlorModel::ParlorModel(PlayerCityPtr city)
  : __INIT_IMPL(ParlorModel)
{
  __D_REF(d,ParlorModel)
  d.city = city;
  d.advisorPanel = nullptr;
}

void ParlorModel::sendMoney2City(int money)
{
  events::dispatch<Payment>( econ::Issue::donation, money );
}

void ParlorModel::showEmpireMapWindow()
{
  __D_REF(d,ParlorModel)
  d.advisorPanel->parent()->deleteLater();
  events::dispatch<ShowEmpireMap>( true );
}

void ParlorModel::setParent(Widget* parlor)
{
  _dfunc()->parent = parlor;
}

Parlor::Items ParlorModel::items()
{
  Parlor::Items items = { { advisor::employers,     255, "labor" },
                          { advisor::military,      256, "military"},
                          { advisor::empire,        257, "imperial"},
                          { advisor::ratings,       258, "rating"},
                          { advisor::trading,       259, "trade"},
                          { advisor::population,    260, "population"},
                          { advisor::health,        261, "health"},
                          { advisor::education,     262, "education"},
                          { advisor::entertainment, 263, "entertainment"},
                          { advisor::religion,      264, "religion"},
                          { advisor::finance,       265, "financial"},
                          { advisor::main,          266, "chief"} };

  return items;
}

void ParlorModel::switchAdvisor(Advisor type)
{
  __D_REF(d,ParlorModel)
  if( type >= advisor::unknown )
    return;

  auto buttons = d.parent->children().select<PushButton>();
  for( auto btn : buttons )
  {
    btn->setPressed( btn->ID() == type );
  }

  if( d.advisorPanel )
  {
    if( type == d.advisorPanel->type() )
      return;

    d.advisorPanel->deleteLater();
    d.advisorPanel = nullptr;
  }

  switch( type )
  {
  case advisor::employers:
    d.advisorPanel = new advisorwnd::Employer( d.city, d.parent );
  break;
  case advisor::military:
  {
    auto forts = d.city->statistic().objects.find<Fort>();
    d.advisorPanel = new advisorwnd::Legion( d.parent, d.city, forts );
  }
  break;
  case advisor::population:
    d.advisorPanel = new advisorwnd::Population( d.city, d.parent );
  break;
  case advisor::empire:
    d.advisorPanel = new advisorwnd::Emperor( d.city, d.parent );
  break;
  case advisor::trading:
    d.advisorPanel = &d.parent->add<advisorwnd::Trade>( d.city );
  break;
  case advisor::finance:
    d.advisorPanel = new advisorwnd::Finance( d.city, d.parent );
  break;

  default:
  break;
  }

  VariantList vl; vl << (int)type;
  events::dispatch<events::ScriptFunc>( "OnShowAdvisorWindow", vl );
}

}//end namespace advisorwnd

}//end namespace gui
