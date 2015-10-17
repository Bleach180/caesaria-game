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

#include "layerinfo.hpp"
#include "gfx/camera.hpp"
#include "city/city.hpp"
#include "core/variant_map.hpp"
#include "layers/constants.hpp"
#include "game/resourcegroup.hpp"

using namespace gfx;

namespace citylayer
{  

class Info::Impl
{
public:
  struct
  {
    Picture foot;
    Picture body;
    Picture header;
  } columnPic;

  struct PictureInfo {
    Point pos;
    Picture pic;
  };

  typedef std::vector<PictureInfo> Pictures;

  Pictures pictures;
};

void Info::render(Engine& engine)
{
  Layer::render( engine );
}

void Info::_loadColumnPicture( const char* rc, int picId)
{
  _d->columnPic.foot.load( rc, picId + 2 );
  _d->columnPic.body.load( rc, picId + 1 );
  _d->columnPic.header.load( rc, picId );
}

void Info::_initialize()
{
  Layer::_initialize();

  const VariantMap& vm = citylayer::Helper::getConfig( (citylayer::Type)type() );
  VariantMap columnVm = vm.get( "column" ).toMap();
  if( !columnVm.empty() )
  {
    std::string rc = columnVm.get( "rc" ).toString();
    int footIdx = columnVm.get( "foot" );
    int bodyIdx = columnVm.get( "body" );
    int headerIdx = columnVm.get( "header" );
    Picture test(rc, footIdx);
    if( test.isValid() )
    {
      _d->columnPic.foot.load( rc, footIdx );
      _d->columnPic.body.load( rc, bodyIdx );
      _d->columnPic.header.load( rc, headerIdx );
    }
  }
}

void Info::drawColumn( const RenderInfo& rinfo, const Point& pos, const int percent)
{
  // Column made of tree base parts and contains maximum 10 parts.
  // Header (10)
  // Body (10, max 8 pieces)
  // Foot (10)
  //
  // In original game fire colomn may be in one of 12 (?) states: none, f, f+h, f+b+h, f+2b+h, ... f+8b+h

  int clamped = math::clamp(percent, 0, 100);
  int rounded = (clamped / 10) * 10;
  // [0,  9] -> 0
  // [10,19] -> 10
  // ...
  // [80,89] -> 80
  // [90,99] -> 90
  // [100] -> 100
  // rounded == 0 -> nothing
  // rounded == 10 -> header + footer
  // rounded == 20 -> header + body + footer

  if (percent == 0)
  {
    // Nothing to draw.
    return;
  }

  rinfo.engine.draw( _d->columnPic.foot, pos + Point( 10, -21 ) );

  if(rounded > 10)
  {
    for( int offsetY=7; offsetY < rounded; offsetY += 10 )
    {
      rinfo.engine.draw( _d->columnPic.body, pos - Point( -18, 8 + offsetY ) );
    }

    rinfo.engine.draw(_d->columnPic.header, pos - Point(-7, 25 + rounded));
  }
}

Info::~Info() {  }

void Info::beforeRender(Engine& engine)
{
  _d->pictures.clear();
}

void Info::afterRender(Engine& engine)
{
  Point camOffset = _camera()->offset();

  for( auto& pic : _d->pictures )
  {
    engine.draw( pic.pic, camOffset + pic.pos );
  }

  Layer::afterRender( engine );
}

Info::Info( Camera& camera, PlayerCityPtr city, int columnIndex )
  : Layer( &camera, city ), _d( new Impl )
{
  _loadColumnPicture( ResourceGroup::sprites, columnIndex );
}

void Info::_addPicture(Point pos, const Picture& pic)
{
  Impl::PictureInfo info = { pos, pic };
  _d->pictures.push_back( info );
}

}//end namespace citylayer
