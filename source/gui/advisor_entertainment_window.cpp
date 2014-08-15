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

#include "advisor_entertainment_window.hpp"
#include "gfx/decorator.hpp"
#include "core/gettext.hpp"
#include "texturedbutton.hpp"
#include "label.hpp"
#include "game/resourcegroup.hpp"
#include "core/stringhelper.hpp"
#include "objects/construction.hpp"
#include "gfx/engine.hpp"
#include "core/gettext.hpp"
#include "game/enums.hpp"
#include "city/helper.hpp"
#include "core/foreach.hpp"
#include "objects/house.hpp"
#include "festival_planing_window.hpp"
#include "objects/house_level.hpp"
#include "objects/entertainment.hpp"
#include "city/cityservice_festival.hpp"
#include "religion/romedivinity.hpp"
#include "game/gamedate.hpp"
#include "core/logger.hpp"
#include "objects/constants.hpp"
#include "objects/hippodrome.hpp"
#include "widget_helper.hpp"

using namespace constants;
using namespace gfx;

namespace gui
{

namespace advisorwnd
{

struct InfrastructureInfo
{
  int buildingCount;
  int partlyWork;
  int buildingWork;
  int buildingShow;
  int peoplesServed;
};

class EntertainmentInfoLabel : public Label
{
public:
  EntertainmentInfoLabel( Widget* parent, const Rect& rect,
                          const TileOverlay::Type service, InfrastructureInfo info  )
    : Label( parent, rect ),
      _service( service ),
      _info( info )
  {    
    setFont( Font::create( FONT_1_WHITE ) );
  }

  const InfrastructureInfo& getInfo() const
  {
    return _info;
  }

  virtual void _updateTexture( gfx::Engine& painter )
  {
    Label::_updateTexture( painter );

    std::string buildingStr, peoplesStr;
    switch( _service )
    {
    case building::theater: buildingStr = _("##theaters##"); peoplesStr = _("##peoples##"); break;
    case building::amphitheater: buildingStr = _("##amphitheatres##"); peoplesStr = _("##peoples##"); break;
    case building::colloseum: buildingStr = _("##colloseum##"); peoplesStr = _("##peoples##"); break;
    case building::hippodrome: buildingStr = _("##hippodromes##"); peoplesStr = "-"; break;
    default:
    break;
    }

    PictureRef& texture = _textPictureRef();
    Font rfont = font();
    rfont.draw( *texture, StringHelper::format( 0xff, "%d %s", _info.buildingCount, buildingStr.c_str() ), 0, 0 );
    rfont.draw( *texture, StringHelper::format( 0xff, "%d", _info.buildingWork ), 165, 0 );
    rfont.draw( *texture, StringHelper::format( 0xff, "%d", _info.buildingShow ), 245, 0 );
    rfont.draw( *texture, StringHelper::format( 0xff, "%d %s", _info.peoplesServed, peoplesStr.c_str() ), 305, 0 );
  }

private:
  TileOverlay::Type _service;
  InfrastructureInfo _info;
};

class Entertainment::Impl
{
public:
  PlayerCityPtr city;

  EntertainmentInfoLabel* lbTheatresInfo;
  EntertainmentInfoLabel* lbAmphitheatresInfo;
  EntertainmentInfoLabel* lbColisseumInfo;
  EntertainmentInfoLabel* lbHippodromeInfo;
  Label* lbBlackframe;
  Label* lbTroubleInfo;
  PushButton* btnNewFestival;
  Label* lbInfoAboutLastFestival;
  TexturedButton* btnHelp;
  Label* lbMonthFromLastFestival;
  city::FestivalPtr srvc;

  InfrastructureInfo getInfo( PlayerCityPtr city, const TileOverlay::Type service );
  void assignFestival(int divinityType, int festSize);
  void updateInfo();
  void updateFestivalInfo();
};


Entertainment::Entertainment(PlayerCityPtr city, Widget* parent, int id )
: Window( parent, Rect( 0, 0, 1, 1 ), "", id ), _d( new Impl )
{
  _d->city = city;
  _d->srvc << city->findService( city::Festival::defaultName() );

  setupUI( ":/gui/entertainmentadv.gui" );

  setPosition( Point( (parent->width() - width() )/2, parent->height() / 2 - 242 ) );

  GET_DWIDGET_FROM_UI( _d, lbBlackframe )
  GET_DWIDGET_FROM_UI( _d, lbTroubleInfo )
  GET_DWIDGET_FROM_UI( _d, btnHelp )
  GET_DWIDGET_FROM_UI( _d, btnNewFestival )
  GET_DWIDGET_FROM_UI( _d, lbMonthFromLastFestival )
  GET_DWIDGET_FROM_UI( _d, lbInfoAboutLastFestival )

  Point startPoint( 2, 2 );
  Size labelSize( 550, 20 );
  InfrastructureInfo info;
  info = _d->getInfo( city, building::theater );
  _d->lbTheatresInfo = new EntertainmentInfoLabel( _d->lbBlackframe, Rect( startPoint, labelSize ), building::theater, info );

  info = _d->getInfo( city, building::amphitheater );
  _d->lbAmphitheatresInfo = new EntertainmentInfoLabel( _d->lbBlackframe, Rect( startPoint + Point( 0, 20), labelSize), building::amphitheater,
                                                        info );
  info = _d->getInfo( city, building::colloseum );
  _d->lbColisseumInfo = new EntertainmentInfoLabel( _d->lbBlackframe, Rect( startPoint + Point( 0, 40), labelSize), building::colloseum, info );

  info = _d->getInfo( city, building::hippodrome );
  _d->lbHippodromeInfo = new EntertainmentInfoLabel( _d->lbBlackframe, Rect( startPoint + Point( 0, 60), labelSize), building::hippodrome, info );

  CONNECT( _d->btnNewFestival, onClicked(), this, Entertainment::_showFestivalWindow );

  _d->updateInfo();
  _d->updateFestivalInfo();
}

void Entertainment::draw( Engine& painter )
{
  if( !visible() )
    return;

  Window::draw( painter );
}

void Entertainment::_showFestivalWindow()
{
  FestivalPlaningWindow* wnd = FestivalPlaningWindow::create( this, _d->city, -1 );
  CONNECT( wnd, onFestivalAssign(), _d.data(), Impl::assignFestival );
}

InfrastructureInfo Entertainment::Impl::getInfo(PlayerCityPtr city, const TileOverlay::Type service)
{
  city::Helper helper( city );

  InfrastructureInfo ret;

  ret.buildingWork = 0;
  ret.peoplesServed = 0;
  ret.buildingShow = 0;
  ret.buildingCount = 0;
  ret.partlyWork = 0;

  ServiceBuildingList servBuildings = helper.find<ServiceBuilding>( service );
  foreach( b, servBuildings )
  {
    ServiceBuildingPtr building = *b;
    if( building->numberWorkers() > 0 )
    {
      ret.buildingWork++;

      int maxServing = 0;
      switch( service )
      {
      case building::theater: maxServing = 500; break;
      case building::amphitheater: maxServing = 800; break;
      case building::colloseum: maxServing = 1500; break;
      default:
      break;
      }

      ret.peoplesServed += maxServing * building->numberWorkers() / building->maximumWorkers();
    }
    ret.buildingCount++;
    ret.partlyWork += (building->numberWorkers() != building->maximumWorkers() ? 1 : 0);
  }

  return ret;
}

void Entertainment::Impl::assignFestival(int divinityType, int festSize)
{
  if( srvc.isValid() )
  {
    srvc->assignFestival( (religion::RomeDivinityType)divinityType, festSize );
    updateFestivalInfo();
  }
}

void Entertainment::Impl::updateInfo()
{ 
  StringArray troubles;
  if( !lbTroubleInfo )
    return;

  const InfrastructureInfo& thInfo = lbTheatresInfo->getInfo();
  const InfrastructureInfo& amthInfo = lbAmphitheatresInfo->getInfo();
  const InfrastructureInfo& clsInfo = lbColisseumInfo->getInfo();
  //const InfrastructureInfo& hpdInfo = lbHippodromeInfo->getInfo();

  city::Helper helper( city );
  int theatersNeed = 0, amptNeed = 0, clsNeed = 0, hpdNeed = 0;
  int theatersServed = 0, amptServed = 0, clsServed = 0, hpdServed = 0;
  int nextLevel = 0;

  HouseList houses = helper.find<House>( building::house );
  foreach( it, houses )
  {
    HousePtr house = *it;
    int habitants = house->habitants().count( CitizenGroup::mature );

    const HouseSpecification& lspec = house->spec();

    if( house->isEntertainmentNeed( Service::theater ) )
    {
      theatersNeed +=  habitants;
      theatersServed += (house->hasServiceAccess( Service::theater ) ? habitants : 0);
    }

    if(house->isEntertainmentNeed( Service::amphitheater ))
    {
      amptNeed +=  habitants;
      amptServed += (house->hasServiceAccess( Service::amphitheater ) ? habitants : 0 );
    }

    if(house->isEntertainmentNeed( Service::colloseum ))
    {
      clsNeed += habitants;
      clsServed += (house->hasServiceAccess( Service::colloseum) ? habitants : 0);
    }

    if( house->isEntertainmentNeed( Service::hippodrome ) )
    {
      hpdNeed += habitants;
      hpdServed += (house->hasServiceAccess( Service::hippodrome) ? habitants : 0);
    }

    nextLevel += ((lspec.computeEntertainmentLevel( house ) - lspec.minEntertainmentLevel()) < 0 ? 1 : 0);
  }

  int allNeed = theatersNeed + amptNeed + clsNeed + hpdNeed;
  int allServed = theatersServed + amptServed + clsServed + hpdServed;

  int entertCoverage = math::percentage( allServed, allNeed);

  if( entertCoverage > 80 && entertCoverage <= 100 )     { troubles.push_back( "##entertainment_80_100##" ); }
  else if( entertCoverage > 50 && entertCoverage <= 80 ) { troubles.push_back( "##entertainment_50_80##" ); }
  else if( allNeed > 0 && entertCoverage <= 50 )         { troubles.push_back( "##entertainment_less_50##" ); }

  if( thInfo.partlyWork > 0 ) { troubles.push_back( "" ); }
  if( amthInfo.partlyWork > 0 ) { troubles.push_back( "##some_amphitheaters_no_actors##" ); }
  if( clsInfo.partlyWork > 0 ) { troubles.push_back( "##small_colloseum_show##" ); }

  HippodromeList hippodromes = helper.find<Hippodrome>( building::hippodrome );
  foreach( h, hippodromes )
  {
    if( (*h)->evaluateTrainee( walker::charioteer ) == 100 ) { troubles.push_back( "##no_chariots##" ); }
  }

  if( nextLevel > 0 )
  {
    troubles.push_back( "##entertainment_need_for_upgrade##" );
  }

  if( theatersNeed == 0 )
  {
    troubles.push_back( "##entertainment_not_need##" );
  }

  if( troubles.empty() )
  {
    troubles.push_back( "##entertainment_full##" );
  }

  lbTroubleInfo->setText( _( troubles[ (int)(rand() % troubles.size()) ] ) );
}

void Entertainment::Impl::updateFestivalInfo()
{
  if( srvc.isValid() )
  {
    int monthFromLastFestival = srvc->lastFestivalDate().monthsTo( GameDate::current() );
    std::string text = StringHelper::format( 0xff, "%d %s", monthFromLastFestival, _("##month_from_last_festival##") );

    if( lbMonthFromLastFestival ) { lbMonthFromLastFestival->setText( text ); }

    bool prepare2Festival = srvc->nextFestivalDate() >= GameDate::current();
    btnNewFestival->setText( prepare2Festival ? _("##prepare_to_festival##") : _("##new_festival##") );
    btnNewFestival->setEnabled( !prepare2Festival );

    text = StringHelper::format( 0xff, "##more_%d_month_from_festival##", math::clamp( monthFromLastFestival / 4 * 4, 0, 24) );
    if( lbInfoAboutLastFestival ) { lbInfoAboutLastFestival->setText( _( text.c_str() ) ); }
  }
}

}

}//end namespace gui
