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

#ifndef __CAESARIA_APPCONFIG_H_INCLUDED__
#define __CAESARIA_APPCONFIG_H_INCLUDED__

#include "core/scopedptr.hpp"
#include "core/variant.hpp"
#include "vfs/path.hpp"

namespace game
{

class Settings
{
public:
#define __GS_PROPERTY(a) static const char* a;
  __GS_PROPERTY(ctNamesModel)
  __GS_PROPERTY(localePath)
  __GS_PROPERTY(resourcePath)
  __GS_PROPERTY(pantheonModel)
  __GS_PROPERTY(houseModel)
  __GS_PROPERTY(citiesModel)
  __GS_PROPERTY(empireObjectsModel)
  __GS_PROPERTY(constructionModel)
  __GS_PROPERTY(settingsPath)
  __GS_PROPERTY(language)
  __GS_PROPERTY(resolution)
  __GS_PROPERTY(fullscreen)
  __GS_PROPERTY(climateModel)
  __GS_PROPERTY(savedir)
  __GS_PROPERTY(emigrantSalaryKoeff)
  __GS_PROPERTY(minMonthWithFood)
  __GS_PROPERTY(worldModel)
  __GS_PROPERTY(worklessCitizenAway)
  __GS_PROPERTY(langModel)
  __GS_PROPERTY(fastsavePostfix)
  __GS_PROPERTY(saveExt)
  __GS_PROPERTY(workDir)
  __GS_PROPERTY(lastTranslation)
  __GS_PROPERTY(c3gfx)
  __GS_PROPERTY(c3music)
  __GS_PROPERTY(c3video)
  __GS_PROPERTY(oldgfx)
  __GS_PROPERTY(archivesModel)
  __GS_PROPERTY(soundThemesModel)
  __GS_PROPERTY(soundVolume)
  __GS_PROPERTY(ambientVolume)
  __GS_PROPERTY(musicVolume)
  __GS_PROPERTY(animationsModel)
  __GS_PROPERTY(walkerModel)
  __GS_PROPERTY(emblemsModel)
  __GS_PROPERTY(remakeModel)
  __GS_PROPERTY(screenFitted)
  __GS_PROPERTY(needAcceptBuild)
  __GS_PROPERTY(sg2model)
  __GS_PROPERTY(ranksModel)
  __GS_PROPERTY(autosaveInterval)
  __GS_PROPERTY(talksArchive)
  __GS_PROPERTY(render)
  __GS_PROPERTY(pic_offsets)
  __GS_PROPERTY(picsArchive)
  __GS_PROPERTY(opengl_opts)
  __GS_PROPERTY(font)
  __GS_PROPERTY(walkerRelations)
  __GS_PROPERTY(freeplay_opts)
  __GS_PROPERTY(cellw)
  __GS_PROPERTY(simpleAnimationModel)
  __GS_PROPERTY(hotkeysModel)
  __GS_PROPERTY(cartsModel)
  __GS_PROPERTY(logoArchive)
  __GS_PROPERTY(titleResource)
  __GS_PROPERTY(forbidenTile)
  __GS_PROPERTY(layersOptsModel)
  __GS_PROPERTY(experimental)
  __GS_PROPERTY(scrollSpeed)
  __GS_PROPERTY(buildMenuModel)
  __GS_PROPERTY(borderMoving)
  __GS_PROPERTY(mmb_moving)
  __GS_PROPERTY(lockInfobox)
  __GS_PROPERTY(soundAlias)
  __GS_PROPERTY(videoAlias)
  __GS_PROPERTY(playerName)
  __GS_PROPERTY(lastGame)
  __GS_PROPERTY(tooltipEnabled)
  __GS_PROPERTY(screenshotDir)
  __GS_PROPERTY(showTabletMenu)
  __GS_PROPERTY(batchTextures)
  __GS_PROPERTY(ccUseAI)
  __GS_PROPERTY(metricSystem)
  __GS_PROPERTY(defaultFont)
  __GS_PROPERTY(celebratesConfig)
#undef __GS_PROPERTY

  static Settings& instance();

  static void set( const std::string& option, const Variant& value );
  static Variant get( const std::string& option );

  static vfs::Path rcpath( const std::string& option="" );
  static vfs::Path rpath( const std::string& option="" );

  static void load();
  static void save();

  void setwdir( const std::string& wdir );
  void checkwdir( char* argv[], int argc );
  void checkCmdOptions( char* argv[], int argc );
  void checkC3present();
  void changeSystemLang( const std::string& newLang );
private:
  Settings();

  class Impl;
  ScopedPtr< Impl > _d;
};

}//end namespace game

#define SETTINGS_RC_PATH(a) game::Settings::rcpath( game::Settings::a )
#define SETTINGS_VALUE(a) game::Settings::get( game::Settings::a )
#define KILLSWITCH(a) game::Settings::get( game::Settings::a ).toBool()
#define SETTINGS_SET_VALUE(a, value) game::Settings::set( game::Settings::a, value )

#endif //__CAESARIA_APPCONFIG_H_INCLUDED__
