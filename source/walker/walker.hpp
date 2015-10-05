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
// Copyright 2012-2015 Dalerank, dalerankn8@gmail.com

#ifndef _CAESARIA_WALKER_H_INCLUDE_
#define _CAESARIA_WALKER_H_INCLUDE_

#include <string>
#include <memory>

#include "walker/action.hpp"
#include "gfx/picturesarray.hpp"
#include "core/serializer.hpp"
#include "core/scopedptr.hpp"
#include "pathway/predefinitions.hpp"
#include "core/referencecounted.hpp"
#include "city/predefinitions.hpp"
#include "constants.hpp"
#include "core/debug_queue.hpp"
#include "world/nation.hpp"
#include "core/smartptr.hpp"
#include "predefinitions.hpp"

class Pathway;
namespace gfx { class Tile; class Animation; class Tilemap; }

class Walker : public Serializable, public ReferenceCounted
{
public:
  typedef unsigned int UniqueId;
  typedef enum { acNone=0, acMove, acFight, acDie, acWork, acMax } Action;
  typedef enum { infiniteWait=-1, showDebugInfo=1, vividly, showPath, userFlag=0x80, count=0xff } Flag;
  typedef enum { thCurrent, thAction, thCount } Thought;
  typedef enum { plOrigin, plDestination, pcCount } Place;

  Walker( PlayerCityPtr city );
  virtual ~Walker();

  virtual void timeStep(const unsigned long time);  // performs one simulation step
  virtual walker::Type type() const;

  // position and movement

  TilePos pos() const;
  void setPos( const TilePos& pos );

  virtual const Point& mappos() const;
  Point tilesubpos() const;

  const gfx::Tile& tile() const;

  virtual void setPathway(const Pathway& pathway);
  const Pathway& pathway() const;

  virtual void turn( TilePos pos );

  float speed() const;
  void setSpeed(const float speed);

  float speedMultiplier() const;
  void setSpeedMultiplier( float koeff );

  void setUniqueId( const UniqueId uid );
  UniqueId uniqueId() const;

  void setFlag( Flag flag, bool value );
  bool getFlag( Flag flag ) const;

  Direction direction() const;
  Walker::Action action() const;

  virtual double health() const;
  virtual void updateHealth(double value);
  virtual void acceptAction( Action action, TilePos pos );

  virtual void setName( const std::string& name );
  virtual const std::string& name() const;

  virtual std::string thoughts( Thought about ) const;
  virtual void setThinks( std::string newThinks );

  virtual TilePos places( Place type ) const;

  virtual void save( VariantMap& stream) const;
  virtual void load( const VariantMap& stream);

  virtual void addAbility( AbilityPtr ability );

  virtual void go( float speed = 1.0 );
  virtual void wait( int ticks = 0 );
  virtual int  waitInterval() const;
  virtual bool die();

  virtual void getPictures( gfx::Pictures& oPics);

  bool isDeleted() const;  // returns true if the walker should be forgotten
  void deleteLater();

  virtual void initialize( const VariantMap& options );
  virtual int agressive() const;

  virtual world::Nation nation() const;

  void attach();
  Point wpos() const;

protected:
  void _walk();
  void _updateMappos();
  void _computeDirection();
  const gfx::Tile& _nextTile() const;

  virtual void _changeTile();  // called when the walker is on a new tile
  virtual void _centerTile();  // called when the walker is on the middle of a tile
  virtual void _reachedPathway();  // called when the walker is at his destination
  virtual void _changeDirection(); // called when the walker changes direction
  virtual void _brokePathway(TilePos pos);
  virtual void _noWay();
  virtual void _waitFinished();
  virtual const gfx::Picture& getMainPicture();
  virtual void _setAction( Walker::Action action );
  virtual void _updatePathway(const Pathway& pathway );
  virtual Point& _rndOffset();
  virtual void _updateThoughts();

  Pathway& _pathway();

  gfx::Animation& _animation();
  const gfx::Animation &_animation() const;
  void _setDirection( Direction direction );
  void _setNation( world::Nation nation );
  void _setLocation( gfx::Tile* tile );
  void _setType( walker::Type type );
  PlayerCityPtr _city() const;
  gfx::Tilemap& _map() const;
  void _setHealth( double value );
  void _updateAnimation(const unsigned int time);
  void _setWpos(const Point &pos );

private:
  class Impl;
  ScopedPtr<Impl> _d;
};

#ifdef DEBUG
class WalkerDebugQueue : public DebugQueue<Walker>
{
public:
  static void print();
};
#endif

#endif //_CAESARIA_WALKER_H_INCLUDE_
