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
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#include "tilemap.hpp"

#include "imgid.hpp"
#include "gfx/tile_config.hpp"
#include "gfx/tilemap_config.hpp"
#include "objects/building.hpp"
#include "core/exception.hpp"
#include "core/position.hpp"
#include "gfx/tilesarray.hpp"
#include "core/saveadapter.hpp"
#include "core/variant_map.hpp"
#include "core/utils.hpp"
#include "core/foreach.hpp"
#include "core/logger.hpp"

using namespace direction;

namespace gfx
{

class SvkBorderConfig
{
public:
  static SvkBorderConfig& instance() { static SvkBorderConfig inst; return inst; }

  void init()
  {
    bordermaps = config::load( ":/svk_borders.model" );
    coast_west = bordermaps.get( "coast_west" ).toMap();
    coast_north = bordermaps.get( "coast_north" ).toMap();
    coast_south = bordermaps.get( "coast_south" ).toMap();
    coast_east = bordermaps.get( "coast_east" ).toMap();
  }

  Tile* addTile(const TilePos& pos, const Tilemap& tmap, int size, bool addGround, bool desert);

  VariantMap bordermaps;
  VariantMap coast_west;
  VariantMap coast_north;
  VariantMap coast_south;
  VariantMap coast_east;
  VariantMap coast_00;
  VariantMap coast_x0;
  VariantMap coast_xx;
  VariantMap coast_0x;
};


class TileRow : public TilesArray
{
public:
  ~TileRow()
  {
    for( auto&& tile : *this )
      delete tile;
  }
};

class TileGrid : public std::vector< TileRow >
{
};

class Tilemap::Impl : public TileGrid
{
public:
  struct TurnInfo {
    Tile* tile;
    Picture pic;
    OverlayPtr overlay;
  };

  typedef std::map<Tile*, TurnInfo> MasterTiles;
  TilesArray mapBorder;
  ClimateType climate;

  struct {
    std::map<int,Tile*> tiles;
    bool addGround = false;
    bool enabled = true;
  } svk;

  int size;
  Direction direction;
  int virtWidth;

  Tile* ate( const TilePos& pos );
  Tile* ate( const int i, const int j );

  Tile& at( const int i, const int j );
  Tile& at( const TilePos& pos ) { return at( pos.i(), pos.j() ); }

  bool isInside( const TilePos& pos ) { return isInside( pos.i(), pos.j() ); }
  inline bool isInside( const int i, const int j ) { return( i >= 0 && j>=0 && i < size && j < size); }

  void resize( const int s );
  void set( int i, int j, Tile* v );
  void saveMasterTiles( MasterTiles& mtiles );
  void checkCoastAfterTurn();
};

Tilemap::Tilemap() : _d( new Impl )
{
  _d->size = 0;
  _d->direction = direction::north;
  _d->virtWidth = config::tilemap.cell.size().width() * 2;
}

void Tilemap::resize( const unsigned int size )
{
  _d->resize( size );
  _d->mapBorder = rect( TilePos( 0, 0), TilePos( size-1, size-1 ) );
}

bool Tilemap::isInside(const TilePos& pos ) const
{
  return _d->isInside( pos );
}

TilePos Tilemap::fit( const TilePos& pos ) const
{
  TilePos ret;
  ret.setI( math::clamp( pos.i(), 0, _d->size ) );
  ret.setJ( math::clamp( pos.j(), 0, _d->size ) );
  return ret;
}

Tile* Tilemap::at( const Point& pos, bool overborder)
{
  // x relative to the left most pixel of the tilemap
  int i = (pos.x() + 2 * pos.y()) / _d->virtWidth;
  int j = (pos.x() - 2 * pos.y()) / _d->virtWidth;
  int s = size();

  if( overborder )
  {
      i = math::clamp( i, 0, s - 1 );
      j = math::clamp( j, 0, s - 1 );
  }

  if (i>=0 && j>=0 && i < s && j < s)
  {
    // valid coordinate
    return &at( i, j );
  }
  else // the pixel is outside the tilemap => no tile here
  {
    return NULL;
  }
}

TilePos Tilemap::p2tp(const Point &pos)
{
  return TilePos( (pos.x() + 2 * pos.y()) / _d->virtWidth,
                  (pos.x() - 2 * pos.y()) / _d->virtWidth );
}

Tile& Tilemap::at(const int i, const int j) {  return _d->at( i, j );}
const Tile& Tilemap::at(const int i, const int j) const  {  return _d->at( i, j ); }
Tile& Tilemap::at( const TilePos& ij ){  return _d->at( ij.i(), ij.j() ); }
void Tilemap::setClimate(ClimateType climate) { _d->climate = climate; }
ClimateType Tilemap::climate() const { return _d->climate; }
OverlayPtr Tilemap::overlay(const TilePos& ij) { return _d->at( ij ).overlay(); }
const Tile& Tilemap::at( const TilePos& ij) const {  return this->at( ij.i(), ij.j() ); }

TilesArray Tilemap::allTiles() const
{
  TilesArray ret;
  TileGrid& tg = *_d;
  for( int row=0; row < _d->size; row++ )
  {
    ret.append( tg[row] );
  }

  return ret;
}

const TilesArray& Tilemap::border() const { return _d->mapBorder; }
void Tilemap::setSvkBorderEnabled(bool enabled)
{
  _d->svk.enabled = enabled;
  clearSvkBorder();
}

int findSvkBorderIndex( const Tile& tile, const VariantMap& items )
{
  const std::string& name = tile.picture().name();
  int limiterIndex = name.find( "_" );
  std::string str = name.substr( limiterIndex+1 );

  return items.get( str ).toInt();
}

void Tilemap::clearSvkBorder()
{
  _d->svk.tiles.clear();
  SvkBorderConfig::instance().init();
}

Tile* Tilemap::svk_at(int i, int j) const
{
  if( !_d->svk.enabled )
    return nullptr;

  TilePos tpos( i, j );
  SvkBorderConfig& svk = SvkBorderConfig::instance();

  auto it = _d->svk.tiles.find( tpos.hash() );
  if( it != _d->svk.tiles.end() )
  {
    return it->second;
  }
  else
  {
    Tile* tl = svk.addTile( tpos, *this, _d->size,
                            _d->svk.addGround,
                            _d->climate == game::climate::desert );
    _d->svk.tiles[ tpos.hash() ] = tl;
    return tl;
  }
}

TilesArray Tilemap::svkTiles() const
{
  TilesArray ret;
  for( const auto& tile : _d->svk.tiles )
    ret.push_back( tile.second );

  return ret;
}

int Tilemap::size() const { return _d->size; }

TilesArray Tilemap::getNeighbors(const TilePos& pos, int type)
{
  TilePos offset(1,1);
  switch (type)
  {
  case AllNeighbors:
    return rect(pos - offset, pos + offset, CheckCorners);

  case FourNeighbors:
    return rect(pos - offset, pos + offset, !CheckCorners);

  default: break;
  }

  Logger::fatal( "Unexpected type {} in Tilemap::getNeighbors", type );
  return TilesArray();
}

TilesArray Tilemap::rect( TilePos start, TilePos stop, const bool corners /*= true*/ )
{
  TilesArray res;

  int mini = math::min( start.i(), stop.i() );
  int minj = math::min( start.j(), stop.j() );
  int maxi = math::max( start.i(), stop.i() );
  int maxj = math::max( start.j(), stop.j() );
  start = TilePos( mini, minj );
  stop = TilePos( maxi, maxj );

  size_t expected = 2 * ((maxi - mini) + (maxj - minj)) + corners;
  res.reserve(expected);

  int delta_corners = 0;
  if(!corners)
  {
    delta_corners = 1;
  }

  int tmpij = start.i();
  //west side
  TilePos location;
  for(int j = start.j() + delta_corners; j <= stop.j() - delta_corners; ++j)
  {
    location.set( tmpij, j );
    if( isInside( location ) )
      res.push_back( &at( location ));
  }

  tmpij = stop.j();
  for(int i = start.i() + 1; i <= stop.i() - delta_corners; ++i)
  {
    location.set( i, tmpij );
    if( isInside( location ) )
      res.push_back( &at( location ));
  }

  tmpij = stop.i();
  for (int j = stop.j() - 1; j >= start.j() + delta_corners; --j)  // corners have been handled already
  {
    location.set( tmpij, j );
    if( isInside( location ))
      res.push_back(&at( location ));
  }

  tmpij = start.j();
  for( int i = stop.i() - 1; i >= start.i() + 1; --i)  // corners have been handled already
  {
    location.set( i, tmpij );
    if( isInside( location ) )
      res.push_back(&at( location ) );
  }

  return res;
}

TilesArray Tilemap::rect( TilePos pos, Size size, const bool corners /*= true */ )
{
  return rect( pos, pos + TilePos( size.width()-1, size.height()-1), corners );
}

TilesArray Tilemap::rect(unsigned int range, TilePos center)
{
  TilePos offset( range, range );
  return rect( center - offset, center + offset );
}

// Get tiles inside of rectangle
TilesArray Tilemap::area(const TilePos& start, const TilePos& stop ) const
{
  TilesArray res;
  int expected = math::min((abs(stop.i() - start.i()) + 1) * (abs(stop.j() - start.j()) + 1), 100);
  res.reserve(expected);

  Rect r( start.i(), start.j(), stop.i(), stop.j() );
  r.repair();

  TilePos location;
  for (int i = r.left(); i <= r.right(); ++i)
  {
    for (int j = r.top(); j <= r.bottom(); ++j)
    {
      location.set( i, j );
      if( isInside( location ))
      {
        res.push_back( &_d->at( location.i(), location.j() ) );
      }
    }
  }

  return res;
}

TilesArray Tilemap::area(const TilePos& start, const Size& size ) const
{
  TilePos stop = start;
  stop.ri() += math::max( size.width()-1, 0 );
  stop.rj() += math::max( size.height()-1, 0 );
  return area( start, stop );
}

TilesArray Tilemap::area(int range, const TilePos& center) const
{
  TilePos offset(range,range);
  return area( center - offset, center + offset );
}

void Tilemap::save( VariantMap& stream ) const
{
  // saves the graphics map
  std::vector<long> bitsetInfo;
  std::vector<short> desInfo;
  std::vector<short> idInfo;

  const TilesArray& tiles = allTiles();
  for( const auto& tile : tiles )
  {
    bitsetInfo.push_back( tile::encode( *tile ) );
    desInfo.push_back( tile->param( Tile::pDesirability ) );
    idInfo.push_back( tile->imgId() );
  }

  ByteArray baBitset;
  ByteArray baDes;
  ByteArray baId;
  baBitset.resize( bitsetInfo.size() * sizeof(long) );
  baDes.resize( desInfo.size() * sizeof(short) );
  baId.resize( idInfo.size() * sizeof(short) );

  memcpy( baBitset.data(), &bitsetInfo[0], baBitset.size() );
  memcpy( baDes.data(), &desInfo[0], baDes.size() );
  memcpy( baId.data(), &idInfo[0], baId.size() );

  stream[ "bitset" ]       = Variant( baBitset.base64() );
  stream[ "desirability" ] = Variant( baDes.base64() );
  stream[ "imgId" ]        = Variant( baId.base64() );
  VARIANT_SAVE_ANY_D( stream, _d, size );
}

void Tilemap::load( const VariantMap& stream )
{
  std::string bitsetInfo = stream.get( "bitset" ).toString();
  std::string desInfo    = stream.get( "desirability" ).toString();
  std::string idInfo     = stream.get( "imgId" ).toString();

  int size;
  VARIANT_LOAD_ANY( size, stream );

  resize( size );

  ByteArray baImgId = ByteArray::fromBase64( idInfo );
  ByteArray baBitset = ByteArray::fromBase64( bitsetInfo );
  ByteArray baDes = ByteArray::fromBase64( desInfo );

  const long* bitsetAr = (long*)baBitset.data();
  const short* imgIdAr = (short*)baImgId.data();
  const short* desAr = (short*)baDes.data();

  if( baBitset.empty() || baImgId.empty() || baDes.empty() )
  {
    Logger::warning( "!!! Tilemap::load data's array is null {0}/{1}/{2}", baBitset.size(), baImgId.size(), baDes.size() );
    return;
  }

  TilesArray tiles = allTiles();
  int index = 0;
  for( auto tile : tiles )
  {
    tile::decode( *tile, bitsetAr[index] );
    tile->setParam( Tile::pDesirability, desAr[index] );

    int imgId = imgIdAr[index];
    if( !tile->master() && imgId != 0 )
    {
      Picture pic = imgid::toPicture( imgId );

      tile->setImgId( imgId );

      int tile_size = (pic.width()+2) / _d->virtWidth;  // size of the multi-tile. the multi-tile is a square.

      tile_size = math::clamp<int>( tile_size, 1, 10 );

      // master is the left-most subtile
      Tile* master = (tile_size == 1) ? NULL : tile;

      for ( int di = 0; di<tile_size; ++di )
      {
        // for each subrow of the multi-tile
        for (int dj = 0; dj<tile_size; ++dj)
        {
          // for each subcol of the multi-tile
          Tile &sub_tile = at( tile->pos() + TilePos( di, dj ) );
          sub_tile.setMaster( master );
          sub_tile.setPicture( pic );
        }
      }
    }
    index++;
  }
}

void Tilemap::turnRight()
{
  switch( _d->direction )
  {
  case north: _d->direction = west; break;
  case west: _d->direction = south; break;
  case south: _d->direction = east; break;
  case east: _d->direction = north; break;

  default:
    Logger::warning( "Tilemap::turnRight wrong direction {0}", _d->direction );
  }

  Impl::MasterTiles masterTiles;
  _d->saveMasterTiles( masterTiles );

  const int& msize = _d->size;
  Tile* mTile;
  for( int i=0; i < msize/2;i++)
  {
    for( int j=i; j < msize-1-i;j++)
    {
      mTile = _d->ate( i, j );
      _d->set( i, j, _d->ate( msize -j-1, i ) );
      _d->set( msize-j-1, i, _d->ate( msize-i-1, msize-j-1 ) );
      _d->set( msize-i-1, msize-j-1, _d->ate( j, msize-i-1 ) );
      _d->set( j, msize-i-1, mTile );
    }
  }

  TilePos mTilePos;
  for( auto&& it : masterTiles )
  {
    const Impl::TurnInfo& ti = it.second;

    int pSize=0;
    if( ti.overlay.isValid() )
    {
      pSize = ti.overlay->size().width();
    }
    else
    {
      const Picture& pic = ti.pic;
      pSize = (pic.width() + 2) / _d->virtWidth;
    }

    pSize = math::clamp<int>( pSize, 1, 10 );

    mTilePos = ti.tile->epos() - TilePos( 0, pSize - 1 );
    mTile = _d->ate( mTilePos );
    for( int i=0; i < pSize; i++ )
    {
      for( int j=0; j < pSize; j++ )
      {
        Tile* apTile = _d->ate( mTilePos + TilePos( i, j ) );
        apTile->setMaster( mTile );
      }
    }

    mTile->setPicture( ti.pic );
    ti.tile->changeDirection( mTile, _d->direction );
  }

  _d->checkCoastAfterTurn();
}

void Tilemap::turnLeft()
{
  switch( _d->direction )
  {
  case north: _d->direction = east; break;
  case east: _d->direction = south; break;
  case south: _d->direction = west; break;
  case west: _d->direction = north; break;

  default:
    Logger::warning( "Tilemap::turnLeft wrong direction {0}", _d->direction );
  }

  Impl::MasterTiles masterTiles;
  _d->saveMasterTiles( masterTiles );

  unsigned int size = _d->size;
  Tile* mTile;
  for( unsigned int i=0;i<size/2;i++)
  {
    for( unsigned int j=i;j<size-1-i;j++)
    {
      mTile = _d->ate( i, j );
      _d->set( i, j, _d->ate( j, size-1-i ) );
      _d->set( j, size-1-i, _d->ate( size-1-i, size-1-j ) );
      _d->set( size-1-i, size-1-j, _d->ate( size-1-j, i ) );
      _d->set( size-1-j, i, mTile );
    }
  }

  TilePos mTilePos;
  for( auto&& it : masterTiles )
  {
    const Impl::TurnInfo& ti = it.second;

    const Picture& pic = ti.overlay.isValid() ? ti.overlay->picture() : ti.pic;
    int pSize = (pic.width() + 2) / _d->virtWidth;

    pSize = math::clamp<int>( pSize, 1, 10);

    mTilePos = ti.tile->epos() - TilePos( pSize - 1, 0 );
    mTile = _d->ate( mTilePos );
    for( int i=0; i < pSize; i++ )
    {
      for( int j=0; j < pSize; j++ )
      {
        Tile* apTile = _d->ate( mTilePos + TilePos( i, j ) );
        apTile->setMaster( mTile );
      }
    }

    mTile->setPicture( ti.pic );
    ti.tile->changeDirection( mTile, _d->direction );
  }

  _d->checkCoastAfterTurn();
}

void Tilemap::setFlag(Flag flag, bool enabled)
{
  switch( flag )
  {
  case fSvkGround: _d->svk.addGround = enabled; break;
  }
}

Direction Tilemap::direction() const { return _d->direction; }

Tilemap::~Tilemap(){}

Tile* Tilemap::Impl::ate(const TilePos& pos )
{
  return ate( pos.i(), pos.j() );
}

Tile* Tilemap::Impl::ate(const int i, const int j)
{
  if( isInside( i, j ) )
  {
    return (*this)[i][j];
  }

  return 0;
}

Tile& Tilemap::Impl::at(const int i, const int j)
{
  if( isInside( i, j ) )
  {
    return *(*this)[i][j];
  }
  return gfx::tile::getInvalidSafe();
}

void Tilemap::Impl::resize(const int s)
{
  size = s;

  // resize the tile array
  TileGrid::resize( size );
  SvkBorderConfig::instance().init();

  for( int i = 0; i < size; ++i )
  {
    (*this)[i].reserve( size );

    for (int j = 0; j < size; ++j)
    {
      (*this)[i].push_back( new Tile( TilePos( i, j ) ));
    }
  }
}

void Tilemap::Impl::set(int i, int j, Tile* v)
{
  v->setEPos( TilePos( i, j ) );
  (*this)[i][j] = v;
}

void Tilemap::Impl::saveMasterTiles(Tilemap::Impl::MasterTiles &mtiles)
{
  Tile* tmp;

  for( int i=0; i < size; i++ )
  {
    for( int j=0; j < size; j++ )
    {
      tmp = ate( i, j );
      Tile* masterTile = tmp->master();

      if( masterTile )
      {
        Impl::MasterTiles::iterator mtFound = mtiles.find( masterTile );

        if( mtFound == mtiles.end() )
        {
          Impl::TurnInfo ti;
          ti.tile = masterTile;
          ti.pic = ti.tile ->picture();
          ti.overlay = tmp->overlay();

          mtiles[ masterTile ] = ti;

          int pSize = (ti.pic.width() + 2) / virtWidth;

          pSize = math::clamp<int>(  pSize, 1, 10 );

          for( int i=0; i < pSize; i++ )
          {
            for( int j=0; j < pSize; j++ )
            {
              Tile* apTile = ate( ti.tile->epos() + TilePos( i, j ) );
              apTile->setMaster( 0 );
              apTile->setPicture( Picture::getInvalid() );
            }
          }
        }
      }
    }
  }
}

void Tilemap::Impl::checkCoastAfterTurn()
{
  for( int i=0; i < size; i++ )
  {
    for( int j=0; j < size; j++ )
    {
      Tile* tmp = ate( i, j );
      if( tmp->getFlag( Tile::tlCoast ) || tmp->getFlag( Tile::tlWater ) )
        tmp->changeDirection( 0, direction );
    }
  }
}

Tile* SvkBorderConfig::addTile(const TilePos& tpos, const Tilemap& tmap, int size,
                               bool addGround, bool desert)
{
  Picture pic;
  int i = tpos.i();
  int j = tpos.j();
  Tile* tl = nullptr;
  Tile::Terrain terrain;
  if( i < 0 && j >= 0 && j < size )
  {
    const Tile& tile = tmap.at( 0, j );
    terrain = tile.terrain();
    tl = new Tile( tpos );

    if( tile.terrain().coast ) pic.load( "land1a", findSvkBorderIndex( tile, coast_west ) );
    else if( tile.terrain().road ) pic = pic.load( "land2a", 84 );
    else if( tile.terrain().water ) pic.load( "land1a", 120 );
    else pic.load( "land1a", math::clamp( math::random(61), 10, 61 ) );
  }
  else if( i >= 0 && i < size && j < 0 )
  {
    const Tile& tile = tmap.at( i, 0 );
    terrain = tile.terrain();
    tl = new Tile( tpos );

    if( tile.terrain().coast ) pic.load( "land1a", findSvkBorderIndex( tile, coast_south ) );
    else if( tile.terrain().road ) pic = pic.load( "land2a", 84 );
    else if( tile.terrain().water ) pic.load( "land1a", 120 );
    else pic.load( "land1a", math::clamp( math::random(61), 10, 61 ) );
  }
  else if( i >= 0 && i < size && j >= size )
  {
    const Tile& tile = tmap.at( i, size-1 );
    terrain = tile.terrain();
    tl = new Tile( tpos );

    if( tile.terrain().coast ) pic.load( "land1a", findSvkBorderIndex( tile, coast_north ) );
    else if( tile.terrain().road ) pic = pic.load( "land2a", 84 );
    else if( tile.terrain().water ) pic.load( "land1a", 120 );
    else pic.load( "land1a", math::clamp( math::random(61), 10, 61 ) );
  }
  else if( i >= size && j >=0 && j < size )
  {
    const Tile& tile = tmap.at( size-1, j );
    terrain = tile.terrain();
    tl = new Tile( tpos );

    if( tile.terrain().coast ) pic.load( "land1a", findSvkBorderIndex( tile, coast_east )  );
    else if( tile.terrain().road ) pic = pic.load( "land2a", 84 );
    else if( tile.terrain().water ) pic.load( "land1a", 120 );
    else pic.load( "land1a", math::clamp( math::random(61), 10, 61 ) );
  }
  else if( i < 0 && j < 0 )
  {
    const Tile& tile = tmap.at( 0, 0 );
    terrain = tile.terrain();
    tl = new Tile( tpos );

    if( tile.terrain().coast ) pic.load( "land1a", findSvkBorderIndex( tile, coast_00 )  );
    else if( tile.terrain().road ) pic = pic.load( "land2a", 84 );
    else if( tile.terrain().water ) pic.load( "land1a", 120 );
    else pic.load( "land1a", math::clamp( math::random(61), 10, 61 ) );
  }
  else if( i < 0 && j >= size )
  {
    const Tile& tile = tmap.at( 0, 0 );
    terrain = tile.terrain();
    tl = new Tile( tpos );

    if( tile.terrain().coast ) pic.load( "land1a", findSvkBorderIndex( tile, coast_0x )  );
    else if( tile.terrain().road ) pic = pic.load( "land2a", 84 );
    else if( tile.terrain().water ) pic.load( "land1a", 120 );
    else pic.load( "land1a", math::clamp( math::random(61), 10, 61 ) );
  }
  else if( i >= size && j >= size )
  {
    const Tile& tile = tmap.at( 0, 0 );
    terrain = tile.terrain();
    tl = new Tile( tpos );

    if( tile.terrain().coast ) pic.load( "land1a", findSvkBorderIndex( tile, coast_xx )  );
    else if( tile.terrain().road ) pic = pic.load( "land2a", 84 );
    else if( tile.terrain().water ) pic.load( "land1a", 120 );
    else pic.load( "land1a", math::clamp( math::random(61), 10, 61 ) );
  }
  else if( i >= size && j < 0 )
  {
    const Tile& tile = tmap.at( 0, 0 );
    terrain = tile.terrain();
    tl = new Tile( tpos );

    if( tile.terrain().coast ) pic.load( "land1a", findSvkBorderIndex( tile, coast_x0 )  );
    else if( tile.terrain().road ) pic = pic.load( "land2a", 84 );
    else if( tile.terrain().water ) pic.load( "land1a", 120 );
    else pic.load( "land1a", math::clamp( math::random(61), 10, 61 ) );
  }

  if( pic.height() > config::tilemap.cell.picSize().height() )
      tl->setFlag( Tile::tlTree, true );

  if( addGround )
  {
    if( !terrain.water )
    {
      Picture ground( "land1a", math::clamp( 62+math::random(56), 62, 118 ) );
      tl->setPicture( ground );

      if( !desert )
      {
        Animation animation;
        animation.addFrame( pic );
        tl->setAnimation( animation );
      }
    }
    else
    {
      tl->setPicture( pic );
    }
  }
  else
  {
    tl->setPicture( pic );
  }

  return tl;
}

}//end namespace gfx
