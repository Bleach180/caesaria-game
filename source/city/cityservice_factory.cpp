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

#include "cityservice_factory.hpp"
#include "core/logger.hpp"
#include "city/city.hpp"
#include "cityservice_timers.hpp"

namespace city
{

typedef SmartList<ServiceCreator> Creators;

class ServiceFactory::Impl
{
public:
  Creators creators;

  ServiceCreatorPtr find( const std::string& name )
  {
    auto it = std::find_if(creators.begin(),
                           creators.end(),
                           [name](ServiceCreatorPtr a) { return a->serviceName() == name; } );
    return it != creators.end() ? *it : ServiceCreatorPtr();
  }
};

SrvcPtr ServiceFactory::create( PlayerCityPtr city, const std::string& name )
{
  auto sharpPos = name.find( "#" );
  const std::string serviceTypename = sharpPos != std::string::npos ? name.substr( sharpPos+1 ) : name;

  Logger::info("CityServiceFactory: try find creator for service " + serviceTypename);

  auto creator = instance()._d->find( serviceTypename );
  if( creator.isValid() )
  {
    city::SrvcPtr srvc = creator->create( city );
    srvc->setName( name );
    return srvc;
  }

  Logger::warning("CityServiceFactory: not found creator for service " + name);
  return SrvcPtr();
}

void ServiceFactory::addCreator( ServiceCreatorPtr creator )
{
  if( creator.isNull() )
    return;

  auto found = _d->find( creator->serviceName() );

  if( found.isValid() )
  {
    Logger::error( "CityServiceFactory: Also have creator for service " + creator->serviceName() );
    return;
  }

  _d->creators.push_back( creator );
}

ServiceFactory::~ServiceFactory()
{

}

ServiceFactory::ServiceFactory() : _d( new Impl )
{
}

}//end namespace city
