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
// Copyright 2012-2015 dalerank, dalerankn8@gmail.com

#include "infoboxmanager.hpp"
#include "gui/info_box.hpp"
#include "gui/environment.hpp"
#include "gfx/tile.hpp"
#include "core/utils.hpp"
#include "core/gettext.hpp"
#include "core/logger.hpp"
#include "objects/constants.hpp"
#include "walker/walker.hpp"
#include "gfx/tilemap.hpp"
#include "events/script_event.hpp"
#include "gui/infobox_citizen_mgr.hpp"
#include "gui/infobox_working.hpp"
#include "core/common.hpp"
#include <map>

using namespace gui;
using namespace gfx;

namespace gui
{

namespace infobox
{

REGISTER_OBJECT_STATICINFOBOX(elevation,"", "##elevation_info##" )
REGISTER_OBJECT_STATICINFOBOX(aqueduct,"", "##aqueduct_info##")
REGISTER_OBJECT_STATICINFOBOX(garden,"", "##garden_info##")
REGISTER_OBJECT_STATICINFOBOX(statue_small,"", "##statue_small_info##")
REGISTER_OBJECT_STATICINFOBOX(statue_middle,"", "##statue_middle_info##")
REGISTER_OBJECT_STATICINFOBOX(statue_big,"", "##statue_big_info##")
REGISTER_OBJECT_STATICINFOBOX(native_hut,"", "##nativeHut_info##")
REGISTER_OBJECT_STATICINFOBOX(native_field,"", "##nativeField_info##")
REGISTER_OBJECT_STATICINFOBOX(native_center,"", "##nativeCenter_info##")
REGISTER_OBJECT_STATICINFOBOX(high_bridge,"", "##high_bridge_info##")
REGISTER_OBJECT_STATICINFOBOX(low_bridge,"", "##bridge_extends_city_area##")
REGISTER_OBJECT_STATICINFOBOX(burning_ruins,"", "##this_fire_can_spread##" )
REGISTER_OBJECT_STATICINFOBOX(rift,"", "##these_rift_info##" )
REGISTER_OBJECT_SERVICEINFOBOX(prefecture,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(engineering_post,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(clinic,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(baths,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(barber,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(hospital,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(missionaryPost, "", "")
REGISTER_OBJECT_SERVICEINFOBOX(school,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(academy,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(library,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(actorColony,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(gladiatorSchool,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(lionsNursery,"", "")
REGISTER_OBJECT_SERVICEINFOBOX(hippodrome,"", "" )
REGISTER_OBJECT_SERVICEINFOBOX(chariotSchool, "", "" )
REGISTER_OBJECT_SERVICEINFOBOX(forum, "", "" )
REGISTER_OBJECT_SERVICEINFOBOX(governorHouse, "", "##governor_house_text##")
REGISTER_OBJECT_SERVICEINFOBOX(governorVilla, "", "##governor_villa_text##")
REGISTER_OBJECT_SERVICEINFOBOX(governorPalace,  "", "##governor_palace_text##")

class Manager::Impl
{
public:
  bool showDebugInfo;
  bool boxLocked;

  typedef std::map< object::Type, InfoboxCreator* > InfoboxCreators;
  std::map< std::string, object::Type > name2typeMap;

  InfoboxCreators constructors;
};

Manager::Manager() : _d( new Impl )
{
  _d->showDebugInfo = true;
  _d->boxLocked = true;

  citizen::PManager::instance().loadInfoboxes();
}

Manager::~Manager() {}

void Manager::showHelp( PlayerCityPtr city, Ui* gui, TilePos pos )
{
  Tile& tile = city->tilemap().at( pos );
  OverlayPtr overlay = tile.overlay();
  object::Type type;

  if( _d->showDebugInfo )
  {
    Logger::debug( "Tile debug info: dsrbl={}", tile.param( Tile::pDesirability ) );
  }

  type = object::typeOrDefault(overlay);

  Impl::InfoboxCreators::iterator findConstructor = _d->constructors.find( type );

  Infobox* infoBox = findConstructor != _d->constructors.end()
                                  ? findConstructor->second->create( city, gui->rootWidget(), pos )
                                  : 0;
  
  if( infoBox && infoBox->isAutoPosition() )
  {
    Size rSize = gui->rootWidget()->size();
    int y = ( gui->cursorPos().y() < rSize.height() / 2 )
                ? rSize.height() - infoBox->height() - 5
                : 30;
    Point pos( ( rSize.width() - infoBox->width() ) / 2, y );

    infoBox->setPosition( pos );
    infoBox->setFocus();
    infoBox->setWindowFlag( Window::fdraggable, !_d->boxLocked );
  }

  if (!infoBox && type != object::unknown)
  {
    VariantList vl; vl << pos;
    events::dispatch<events::ScriptFunc>("OnShowOverlayInfobox", vl);
  }
}

void Manager::setShowDebugInfo( const bool showInfo ) {  _d->showDebugInfo = showInfo; }

void Manager::addInfobox( const object::Type& type, InfoboxCreator* ctor )
{
  std::string name = object::toString( type );
  bool alreadyHaveConstructor = _d->name2typeMap.find( name ) != _d->name2typeMap.end();

  if (name == "unknown")
    Logger::debug( "InfoboxManager: added default infobox constructor" );

  if( !alreadyHaveConstructor )
  {
    _d->name2typeMap[ name ] = type;
    _d->constructors[ type ] = ctor;
  }
  else
  {
    Logger::debug("InfoboxManager: already have constructor for type " + name);
  }
}

bool Manager::canCreate(const object::Type type) const
{
  return _d->constructors.find( type ) != _d->constructors.end();
}

void infobox::Manager::setBoxLock(bool lock) { _d->boxLocked = lock; }

StaticInfoboxCreator::StaticInfoboxCreator(const std::string &caption, const std::string &desc):
  title( caption ), text( desc )
{}

Infobox* StaticInfoboxCreator::create(PlayerCityPtr city, Widget *parent, TilePos pos)
{
  Size size = parent->size();
  Infobox& infoBox = parent->add<Infobox>( Infobox::defaultRect );
  infoBox.setPosition( Point( ( size.width() - infoBox.width()) / 2,
                                 size.height() - infoBox.height()) );
  OverlayPtr overlay = city->getOverlay( pos );

  std::string caption = overlay.isValid()
                                  ? overlay->info().prettyName()
                                  : title;

  infoBox.setTitle( _( caption ) );
  infoBox.setText( _( text ) );
  return &infoBox;
}

ServiceInfoboxCreator::ServiceInfoboxCreator(const std::string &caption, const std::string &descr, bool drawWorkers)
  : title( caption ), text( descr ), isDrawWorkers( drawWorkers )
{
}

Infobox* ServiceInfoboxCreator::create(PlayerCityPtr city, Widget *parent, TilePos pos)
{
  Size  size = parent->size();
  auto workBuilding = city->getOverlay( pos ).as<WorkingBuilding>();
  if( workBuilding.isValid() )
  {
    AboutWorkingBuilding& infoBox = parent->add<AboutWorkingBuilding>( workBuilding );
    infoBox.setPosition( Point( (size.width() - infoBox.width()) / 2, size.height() - infoBox.height()) );

    if( !title.empty() ) { infoBox.setTitle( title ); }
    if( !text.empty() ) { infoBox.setText( text ); }
    return &infoBox;
  }

  return 0;
}

}//end namespace infobox

}//end namespave gui
