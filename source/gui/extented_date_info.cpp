#include "extented_date_info.hpp"
#include "core/event.hpp"
#include "widget_factory.hpp"
#include "game/gamedate.hpp"
#include "core/utils.hpp"
#include "core/gettext.hpp"
#include "game/roman_celebrates.hpp"

namespace gui
{

REGISTER_CLASS_IN_WIDGETFACTORY(ExtentedDateInfo)
const std::string romDatePrefix = "##rd_";
const std::string romDatePostfix = "##";

ExtentedDateInfo::ExtentedDateInfo(Widget *parent, const Rect& rect, int id)
  : Label( parent, rect, "", false, bgSimpleWhite, id )
{
  setGeometry( Rect( 0, 0, parent->width(), parent->height()) );
  setWordwrap( true );
  setFont( Font::create( FONT_1 ));
}

ExtentedDateInfo::~ExtentedDateInfo() {}

bool ExtentedDateInfo::onEvent(const NEvent& e)
{
  if( e.EventType == sEventGui && e.gui.type == guiEditboxChanged )
  {

  }

  return Label::onEvent( e );
}

void ExtentedDateInfo::draw(gfx::Engine& painter)
{
  if( !visible() )
    return;

  if( game::Date::isDayChanged() )
  {
    _update();
  }

  Label::draw( painter );
}

void ExtentedDateInfo::_update()
{
  RomanDate date( game::Date::current() );
  std::string dayStr   = romDatePrefix + RomanDate::dayName( date.dayOfWeek() ) + romDatePostfix;
  std::string monthStr = romDatePrefix + RomanDate::monthName( date.month()-1 ) + romDatePostfix;
  std::string dayDescription = game::Celebrates::instance().getDescription( date.day(), date.month() );

  std::string text = utils::format( 0xff, "%s %s %d\n %s",
                                    _(dayStr), _(monthStr), date.year(),
                                    dayDescription.c_str() );
  setText( text );
}

}//end namespace gui
