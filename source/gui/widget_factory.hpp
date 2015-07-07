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

#ifndef _CAESARIA_WIDGET_FACTORY_H_INCLUDE_
#define _CAESARIA_WIDGET_FACTORY_H_INCLUDE_

#include "core/scopedptr.hpp"
#include "core/singleton.hpp"
#include <string>

namespace gui
{

class Widget;

class WidgetCreator
{
public:
  virtual Widget* create( Widget* parent ) = 0;
};

template< class T >
class BaseWidgetCreator : public WidgetCreator
{
public:
  Widget* create( Widget* parent )
  {
    return new T( parent );
  }
};

class WidgetFactory : public StaticSingleton<WidgetFactory>
{
  friend class StaticSingleton;
public:
  ~WidgetFactory();

  Widget* create(const std::string& type, Widget* parent ) const;

  bool canCreate( const std::string& type ) const;

  void addCreator( const std::string& type, WidgetCreator* ctor );
private:
  WidgetFactory();

  class Impl;
  ScopedPtr< Impl > _d;
};

#define REGISTER_CLASS_IN_WIDGETFACTORY(a) \
namespace { \
struct Registrator_##a { Registrator_##a() { WidgetFactory::instance().addCreator( CAESARIA_STR_A(a), new BaseWidgetCreator<a>() ); }}; \
static Registrator_##a rtor_##a; \
}

#define REGISTER_CLASSNAME_IN_WIDGETFACTORY(a, name) \
namespace { \
struct Registrator_##a { Registrator_##a() { WidgetFactory::instance().addCreator( name, new BaseWidgetCreator<a>() ); }}; \
static Registrator_##a rtor_##a; \
}


}//end namespace gui
#endif //_CAESARIA_WIDGET_FACTORY_H_INCLUDE_
