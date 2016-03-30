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

#ifndef __CAESARIA_INFOBOX_MANAGER_H_INCLUDE_
#define __CAESARIA_INFOBOX_MANAGER_H_INCLUDE_

#include "core/smartptr.hpp"
#include "core/referencecounted.hpp"
#include "core/scopedptr.hpp"
#include "core/predefinitions.hpp"
#include "gfx/tilemap.hpp"
#include "core/singleton.hpp"
#include "gui/info_box.hpp"

namespace gui
{

namespace infobox
{

class Manager;
typedef SmartPtr< Manager > InfoBoxManagerPtr;

class InfoboxCreator
{
public:
  virtual infobox::Infobox* create( PlayerCityPtr, gui::Widget*, TilePos ) = 0;
};

template< class T >
class FreeInfoboxCreator : public InfoboxCreator
{
public:
  Infobox* create(PlayerCityPtr city, gui::Widget* parent, TilePos pos)
  {
    new T(parent, city, city->tilemap().at(pos));
    return nullptr;
  }
};

template< class T >
class BaseInfoboxCreator : public InfoboxCreator
{
public:
  Infobox* create( PlayerCityPtr city, gui::Widget* parent, TilePos pos )
  {
    return new T( parent, city, city->tilemap().at( pos ) );
  }
};

class Manager : public StaticSingleton<Manager>
{
  SET_STATICSINGLETON_FRIEND_FOR(Manager)
public:
  void showHelp( PlayerCityPtr city, gui::Ui* gui, TilePos tile );
  void setShowDebugInfo( const bool showInfo );

  void addInfobox(const object::Type& type, InfoboxCreator* ctor );
  bool canCreate(const object::Type type ) const;

  void setBoxLock(bool lock);
private:
  Manager();
  virtual ~Manager();
   
  class Impl;
  ScopedPtr< Impl > _d;
};

}//end namespace infobox

}//end namespave gui

#define REGISTER_OBJECT_INFOBOX(name,a) \
namespace { \
struct Registrator_##name { Registrator_##name() { Manager::instance().addInfobox( object::name, a ); }}; \
static Registrator_##name rtor_##name; \
}

#define REGISTER_OBJECT_FREEINFOBOX(name,a) \
namespace { \
struct Registrator_##name { Registrator_##name() { Manager::instance().addInfobox( object::name, new FreeInfoboxCreator<a>() ); }}; \
static Registrator_##name rtor_##name; \
}

#define REGISTER_OBJECT_BASEINFOBOX(name,a) \
namespace { \
struct Registrator_##name { Registrator_##name() { Manager::instance().addInfobox( object::name, new BaseInfoboxCreator<a>() ); }}; \
static Registrator_##name rtor_##name; \
}

#endif //__CAESARIA_INFOBOX_MANAGER_H_INCLUDE_
