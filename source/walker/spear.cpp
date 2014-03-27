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

#include "spear.hpp"
#include "core/gettext.hpp"
#include "city/city.hpp"
#include "game/resourcegroup.hpp"
#include "gfx/tilemap.hpp"
#include "core/foreach.hpp"

using namespace constants;

SpearPtr Spear::create(PlayerCityPtr city)
{
  SpearPtr ret( new Spear( city ) );
  ret->drop();

  return ret;
}

void Spear::_onTarget()
{
  WalkerList walkers = _getCity()->getWalkers( walker::any, dstPos() );
  foreach( w, walkers )
  {
    (*w)->updateHealth( -10 );
    (*w)->acceptAction( Walker::acFight, startPos() );
  }

  TileOverlayPtr overlay = _getCity()->getOverlay( dstPos() );

  ConstructionPtr c = ptr_cast<Construction>( overlay );
  if( c.isValid() )
  {
    c->updateState( Construction::damage, 5 );
  }
}

const char* Spear::rcGroup() const {  return ResourceGroup::sprites; }
int Spear::rcStartIndex() const { return 114; }

Spear::Spear(PlayerCityPtr city) : ThrowingWeapon( city )
{
  _setType( walker::spear );
  _setAnimation( gfx::unknown );

  setName( _("##spear##") );
}
