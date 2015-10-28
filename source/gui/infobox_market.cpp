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

#include <cstdio>

#include "infobox_market.hpp"
#include "label.hpp"
#include "objects/market.hpp"
#include "good/store.hpp"
#include "core/gettext.hpp"
#include "good/helper.hpp"
#include "core/utils.hpp"
#include "core/metric.hpp"
#include "core/logger.hpp"
#include "game/infoboxmanager.hpp"

using namespace gfx;

namespace gui
{

namespace infobox
{

REGISTER_OBJECT_BASEINFOBOX(market,AboutMarket)

AboutMarket::AboutMarket(Widget* parent, PlayerCityPtr city, const Tile& tile )
  : AboutConstruction( parent, Rect( 0, 0, 510, 256 ), Rect( 16, 155, 510 - 16, 155 + 50) )
{
  setupUI( ":/gui/infoboxmarket.gui" );

  MarketPtr market = tile.overlay<Market>();

  if( !market.isValid() )
  {
    Logger::warning( "AboutMarket: market is null tile at [{0},{1}]", tile.i(), tile.j() );
    return;
  }

  setBase( market );
  _setWorkingVisible( true );

  Label& lbAbout = add<Label>( Rect( 15, 30, width() - 15, 50) );
  lbAbout.setWordwrap( true );
  lbAbout.setFont( Font::create( FONT_1 ) );
  lbAbout.setTextAlignment( align::upperLeft, align::upperLeft );

  setTitle( _( market->info().prettyName() ) );

  if( market->numberWorkers() > 0 )
  {
    good::Store& goods = market->goodStore();
    int furageSum = 0;
    // for all furage types of good
    for( good::Product pr=good::none; pr<good::olive; ++pr )
    {
      furageSum += goods.qty( pr );
    }

    int paintY = 100;
    if( 0 < furageSum )
    {
      drawGood( market, good::wheat, 0, paintY );
      drawGood( market, good::fish, 1, paintY);
      drawGood( market, good::meat, 2, paintY);
      drawGood( market, good::fruit, 3, paintY);
      drawGood( market, good::vegetable, 4, paintY);
      lbAbout.setHeight( 60 );
    }
    else
    {
      lbAbout.setHeight( 90 );
      lbAbout.setWordwrap( true );
      lbAbout.setTextAlignment( align::upperLeft, align::center );
    }

    paintY += 24;
    drawGood( market, good::pottery, 0, paintY);
    drawGood( market, good::furniture, 1, paintY);
    drawGood( market, good::oil, 2, paintY);
    drawGood( market, good::wine, 3, paintY);

    lbAbout.setText( 0 == furageSum ? _("##market_search_food_source##") : _("##market_about##"));
  }
  else
  {
    lbAbout.setHeight( 50 );
    lbAbout.setText( _("##market_no_workers##") );
  }

  _updateWorkersLabel( Point( 32, 8 ), 542, market->maximumWorkers(), market->numberWorkers() );
}

AboutMarket::~AboutMarket() {}

void AboutMarket::drawGood( MarketPtr market, const good::Product &goodType, int index, int paintY )
{
  int startOffset = 25;

  int offset = ( width() - startOffset * 2 ) / 5;
  //std::string goodName = good::Helper::name( goodType );
  int qty = market->goodStore().qty( goodType );
  std::string outText = utils::i2str( metric::Measure::convQty( qty ) );

  // pictures of goods
  Picture pic = good::Helper::picture( goodType );
  Point pos( index * offset + startOffset, paintY );

  Label* lb = new Label( this, Rect( pos, pos + Point( 100, 24 )) );
  lb->setFont( Font::create( FONT_2 ) );
  lb->setIcon( pic );
  lb->setText( outText );
  lb->setTextOffset( Point( 30, 0 ) );
}

}

}//end namespace gui
