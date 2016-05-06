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

#include "farm.hpp"
#include "core/position.hpp"
#include "core/exception.hpp"
#include "core/variant_map.hpp"
#include "game/resourcegroup.hpp"
#include "good/stock.hpp"
#include "good/helper.hpp"
#include "city/city.hpp"
#include "core/utils.hpp"
#include "gfx/tilemap.hpp"
#include "core/gettext.hpp"
#include "core/logger.hpp"
#include "constants.hpp"
#include "walker/locust.hpp"
#include "city/statistic.hpp"
#include "core/tilepos_array.hpp"
#include "game/gamedate.hpp"
#include "events/clearland.hpp"
#include "objects/config.hpp"
#include "objects_factory.hpp"
#include "city/states.hpp"

using namespace gfx;
using namespace events;

REGISTER_CLASS_IN_OVERLAYFACTORY(object::fig_farm,       FarmFruit    )
REGISTER_CLASS_IN_OVERLAYFACTORY(object::wheat_farm,     FarmWheat    )
REGISTER_CLASS_IN_OVERLAYFACTORY(object::vinard,         FarmGrape    )
REGISTER_CLASS_IN_OVERLAYFACTORY(object::meat_farm,      FarmMeat     )
REGISTER_CLASS_IN_OVERLAYFACTORY(object::olive_farm,     FarmOlive    )
REGISTER_CLASS_IN_OVERLAYFACTORY(object::vegetable_farm, FarmVegetable)

class FarmTile : public Construction
{
public:
  FarmTile() : Construction( object::farmtile, Size(1,1) ) {}
  FarmTile(const good::Product outGood, const TilePos& farmpos);
  virtual ~FarmTile() {}
  Picture& getPicture();
  virtual void initTerrain(gfx::Tile&) {}
  virtual bool isFlat() const { return false; }
  virtual bool build(const city::AreaInfo &info);
  virtual void save(VariantMap &stream) const;
  virtual void load(const VariantMap &stream);
  static Picture computePicture( const good::Product outGood, const int percent);

private:
  TilePos _farmpos;
};

REGISTER_CLASS_IN_OVERLAYFACTORY(object::farmtile, FarmTile)

FarmTile::FarmTile( const good::Product outGood, const TilePos& farmpos )
 : Construction( object::farmtile, Size(1,1) )
{
  _farmpos = farmpos;
  setPicture( computePicture( outGood, 0 ) );
}

Picture FarmTile::computePicture( const good::Product outGood, const int percent)
{
  int picIdx = 0;
  int sequenceSize = 5;

  std::map<good::Product, int> good2pics;
  good2pics[ good::wheat     ] = 13;
  good2pics[ good::vegetable ] = 18;
  good2pics[ good::fruit     ] = 23;
  good2pics[ good::olive     ] = 28;
  good2pics[ good::grape     ] = 33;
  good2pics[ good::meat      ] = 38;

  auto rIt = good2pics.find( outGood );
  if( rIt != good2pics.end() )
  {
    picIdx = rIt->second;
  }
  else
  {
    Logger::warning( "Unexpected farmType in farm" + good::Helper::name( outGood ) );
  }

  picIdx += math::clamp<int>( (percent * sequenceSize) / 100, 0, sequenceSize-1);
  return Picture( ResourceGroup::commerce, picIdx );
}

bool FarmTile::build(const city::AreaInfo &info)
{
  return Construction::build( info );
}

void FarmTile::save(VariantMap &stream) const
{
  Construction::save( stream );
  VARIANT_SAVE_ANY( stream, _farmpos )
}

void FarmTile::load(const VariantMap &stream)
{
  Construction::load( stream );
  VARIANT_LOAD_ANY( _farmpos, stream )
}

class Farm::Impl
{
public:
  Locations sublocs;
  float meadowsCoverage;
  int lastProgress;
};

Farm::Farm(const good::Product outGood, const object::Type farmType )
  : Factory( good::none, outGood, farmType, Size(3,3) ), _d( new Impl )
{
  outStock().setCapacity( 100 );

  _d->lastProgress = 0;
  _d->meadowsCoverage = 1.f;
  _d->sublocs.append(0, 0)
             .append(2, 2)
             .append(2, 1)
             .append(1, 0)
             .append(2, 0);

  Picture mainPic = _getMainPicture();
  mainPic.addOffset( TilePos( 0, 1 ).toScreenCoordinates() );
  _fgPicture(config::fgpic::idxMainPic) = mainPic;  // farm building

  for( auto& pos : _d->sublocs )
  {
    Picture tPic = FarmTile::computePicture( outGood, 0 );
    tPic.addOffset( pos.toScreenCoordinates() );
    _fgPictures().push_back( tPic );
  }
  setPicture( Picture::getInvalid() );
}

bool Farm::canBuild( const city::AreaInfo& areaInfo ) const
{
  bool is_constructible = Construction::canBuild( areaInfo );

  TilesArea area( areaInfo.city->tilemap(), areaInfo.pos, size() );
  bool on_meadow = area.count( Tile::tlMeadow ) > 0;

  Farm* non_const_this = const_cast< Farm* >( this );
  non_const_this->_setError( on_meadow ? "" : _("##farm_need_farmland##") );

  return (is_constructible && on_meadow);
}

void Farm::burn()
{
  Factory::burn();
  for( auto& pos : _d->sublocs )
  {
    OverlayPtr ov = _map().overlay( pos );
    if( ov.isValid() )
      ov->burn();
  }
}

void Farm::collapse()
{
  Factory::collapse();
  for( auto& pos : _d->sublocs )
  {
    OverlayPtr ov = _map().overlay( pos );
    if( ov.isValid() )
      ov->collapse();
  }
}

void Farm::destroy()
{
  for( auto& pos : _d->sublocs )
  {
    OverlayPtr ov = _map().overlay( pos );
    if( ov.isValid() && ov->type() == object::farmtile )
    {
      events::dispatch<ClearTile>( ov->pos() );
    }
  }

  Factory::destroy();
}

math::Percent Farm::productivity() const
{
  return Factory::productivity() * _d->meadowsCoverage;
}

void Farm::computeRoadside()
{
  Factory::computeRoadside();

  for( auto& pos : _d->sublocs )
  {
    auto construction = _map().overlay<Construction>( pos );
    if( object::typeOrDefault( construction ) == object::farmtile )
    {
      _roadside().append( construction->roadside() );
    }
  }
}

void Farm::computePictures()
{
  int amount = progress();
  int percentTile;

  for(unsigned int n = 0; n<_d->sublocs.size(); ++n)
  {
    if (amount >= 20)   // 20 = 100 / nbSubTiles
    {
      // this subtile is at maximum
      percentTile = 100;  // 100%
      amount -= 20;  // for next subTiles
    }
    else
    {
      // this subtile is not at maximum
      percentTile = 5 * amount;
      amount = 0;  // for next subTiles
    }

    auto farmTile = _map().overlay<FarmTile>( _d->sublocs[n] );
    if( farmTile.isValid() )
      farmTile->setPicture( FarmTile::computePicture( produce().type(), percentTile ));
  }
}

void Farm::assignTile(const TilePos &pos)
{
  _d->sublocs.addUnique( pos );
}

void Farm::timeStep(const unsigned long time)
{
  Factory::timeStep(time);

  if( game::Date::isDayChanged() && mayWork()
      && _d->lastProgress != progress() )
  {
    _d->lastProgress = progress();
    computePictures();
  }
}

void Farm::_updateMeadowsCoverage()
{
  if (_cityOpt(PlayerCity::farmUseMeadows)>0) {
    auto tiles = area();
    _d->meadowsCoverage = (float)tiles.count(Tile::tlMeadow) / (float)tiles.size();
  } else {
    _d->meadowsCoverage = 1.f;
  }
}

bool Farm::build(const city::AreaInfo& info)
{
  setSize(Size(2,2));
  city::AreaInfo upInfo = info;
  if (!info.onload) //it flag use real build
  {
    upInfo.pos += TilePos(0,1);
    _buildFarmTiles(info, upInfo.pos);
  }

  _fgPictures().clear();
  Factory::build(upInfo);

  _fgPictures().resize(config::fgpic::idxFactoryMax);

  setPicture(_getMainPicture());
  computePictures();
  _updateMeadowsCoverage();

  return true;
}

void Farm::save( VariantMap& stream ) const
{
  Factory::save( stream );
  stream[ "locations" ] = _d->sublocs.save();
}

void Farm::load( const VariantMap& stream )
{
  Factory::load( stream );
  _d->sublocs.load( stream.get( "locations").toList() );

  //el muleta for broken farmtiles
  if( !_d->sublocs.empty() && _d->sublocs[ 0 ] == TilePos(0,0) )
    _d->sublocs.clear();

  if( _d->sublocs.empty() )
  {
    Logger::warning( "!!! WARNING: Farm [{0},{1}] lost tiles. Will add default locations", pos().i(), pos().j() );
    _d->sublocs << TilePos(0, 0) << TilePos( 1, 0 )
                << TilePos(2, 0) << TilePos( 2, 1 ) << TilePos( 2, 2);
    for( auto& location : _d->sublocs )
      location += pos() - TilePos( 0, 1 );
  }

  computePictures();
  _updateMeadowsCoverage();
}

Variant Farm::getProperty(const std::string& name) const
{
  if (name == "meadowCoverage") {
    return _d->meadowsCoverage;
  }

  return Factory::getProperty(name);
}

TilesArray Farm::meadows() const
{
  return area().select( Tile::tlMeadow );
}

TilesArray Farm::area() const
{
  TilesArray ret;
  ret.append( Factory::area() );
  for (const auto& st : _d->sublocs)
    ret.append( &_map().at( st ) );

  return ret;
}

unsigned int Farm::produceQty() const
{
  return productRate() * getFinishedQty() * numberWorkers() / maximumWorkers();
}

void Farm::initialize(const object::Info& mdata)
{
  Factory::initialize( mdata );
  //picture will be setting on build
  setPicture( Picture::getInvalid() );
}

Picture Farm::_getMainPicture()
{
  return info().randomPicture()
               .withFallback( ResourceGroup::commerce, 12 );
}

Farm::~Farm() {}

FarmWheat::FarmWheat() : Farm(good::wheat, object::wheat_farm)
{
}

std::string FarmWheat::troubleDesc() const
{
  int locust_n = _city()->statistic().walkers.count( walker::locust, pos() );
  if( locust_n > 0 )
  {
    return "##trouble_farm_was_blighted_by_locust##";
  }

  return Factory::troubleDesc();
}

bool FarmWheat::build( const city::AreaInfo& info )
{
  bool ret = Farm::build( info );
  if( info.city->climate() == game::climate::central )
  {
    setProductRate( productRate() * 2 );
  }

  return ret;
}

FarmOlive::FarmOlive() : Farm(good::olive, object::olive_farm)
{
}

FarmGrape::FarmGrape() : Farm(good::grape, object::vinard)
{
}

FarmMeat::FarmMeat() : Farm(good::meat, object::meat_farm)
{
}

FarmFruit::FarmFruit() : Farm(good::fruit, object::fig_farm)
{
}

FarmVegetable::FarmVegetable() : Farm(good::vegetable, object::vegetable_farm)
{
}

OverlayPtr Farm::_buildFarmTile(const city::AreaInfo &info, const TilePos &ppos)
{
  OverlayPtr farmtile( new FarmTile( produce().type(), ppos ) );
  farmtile->drop();
  farmtile->build( info );
  info.city->addOverlay( farmtile );
  return farmtile;
}

void Farm::_buildFarmTiles(const city::AreaInfo& info, const TilePos& ppos )
{
  for( auto& location : _d->sublocs )
  {
    city::AreaInfo tInfo = info;
    tInfo.pos += location;
    _buildFarmTile( tInfo, ppos );
    location = tInfo.pos;
  }
}
