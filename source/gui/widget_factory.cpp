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

#include "widget_factory.hpp"
#include "core/utils.hpp"
#include "core/requirements.hpp"
#include "core/logger.hpp"
#include <map>

namespace gui
{

class WidgetFactory::Impl
{
public:
  typedef std::map< std::string, WidgetCreator* > BuildingCreators;
  BuildingCreators constructors;
};

Widget* WidgetFactory::create(const std::string& type, Widget* parent ) const
{
  Impl::BuildingCreators::iterator findConstructor = _d->constructors.find( type );

  if( findConstructor != _d->constructors.end() )
  {
    return findConstructor->second->create( parent );
  }

  return 0;
}

WidgetFactory::WidgetFactory() : _d( new Impl )
{
}

WidgetFactory::~WidgetFactory() {}

void WidgetFactory::addCreator( const std::string& typeName, WidgetCreator* ctor )
{
  bool alreadyHaveConstructor = _d->constructors.find( typeName ) != _d->constructors.end();

  if( !alreadyHaveConstructor )
  {
    _d->constructors[ typeName ] = ctor;
  }
  else
  {
    Logger::warning( "already have constructor for this widget type" );
  }
}

bool WidgetFactory::canCreate( const std::string& type ) const
{
  return _d->constructors.find( type ) != _d->constructors.end();
}

}//end namespace gui
