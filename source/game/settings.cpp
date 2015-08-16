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

#include "settings.hpp"
#include "vfs/path.hpp"
#include "core/saveadapter.hpp"
#include "vfs/directory.hpp"
#include "core/variant_map.hpp"
#include "core/utils.hpp"
#include "core/foreach.hpp"
#include "core/metric.hpp"

namespace game
{

#define __REG_PROPERTY(a) const char* Settings::a = CAESARIA_STR_EXT(a);
__REG_PROPERTY(localePath)
__REG_PROPERTY(resourcePath )
__REG_PROPERTY(pantheonModel )
__REG_PROPERTY(houseModel )
__REG_PROPERTY(citiesModel)
__REG_PROPERTY(climateModel)
__REG_PROPERTY(constructionModel)
__REG_PROPERTY(ctNamesModel)
__REG_PROPERTY(settingsPath)
__REG_PROPERTY(resolution)
__REG_PROPERTY(fullscreen)
__REG_PROPERTY(language)
__REG_PROPERTY(emigrantSalaryKoeff)
__REG_PROPERTY(savedir)
__REG_PROPERTY(worldModel)
__REG_PROPERTY(minMonthWithFood)
__REG_PROPERTY(langModel)
__REG_PROPERTY(worklessCitizenAway)
__REG_PROPERTY(fastsavePostfix)
__REG_PROPERTY(saveExt)
__REG_PROPERTY(workDir)
__REG_PROPERTY(c3gfx)
__REG_PROPERTY(c3music)
__REG_PROPERTY(c3video)
__REG_PROPERTY(oldgfx)
__REG_PROPERTY(lastTranslation)
__REG_PROPERTY(archivesModel)
__REG_PROPERTY(soundThemesModel)
__REG_PROPERTY(soundVolume )
__REG_PROPERTY(ambientVolume)
__REG_PROPERTY(musicVolume )
__REG_PROPERTY(animationsModel )
__REG_PROPERTY(walkerModel)
__REG_PROPERTY(emblemsModel )
__REG_PROPERTY(remakeModel )
__REG_PROPERTY(screenFitted)
__REG_PROPERTY(needAcceptBuild)
__REG_PROPERTY(sg2model)
__REG_PROPERTY(ranksModel)
__REG_PROPERTY(autosaveInterval)
__REG_PROPERTY(talksArchive)
__REG_PROPERTY(render)
__REG_PROPERTY(empireObjectsModel)
__REG_PROPERTY(pic_offsets)
__REG_PROPERTY(picsArchive)
__REG_PROPERTY(opengl_opts)
__REG_PROPERTY(font)
__REG_PROPERTY(walkerRelations)
__REG_PROPERTY(freeplay_opts)
__REG_PROPERTY(cellw)
__REG_PROPERTY(simpleAnimationModel)
__REG_PROPERTY(hotkeysModel)
__REG_PROPERTY(cartsModel)
__REG_PROPERTY(logoArchive)
__REG_PROPERTY(titleResource)
__REG_PROPERTY(forbidenTile)
__REG_PROPERTY(layersOptsModel)
__REG_PROPERTY(experimental)
__REG_PROPERTY(buildMenuModel)
__REG_PROPERTY(scrollSpeed)
__REG_PROPERTY(borderMoving)
__REG_PROPERTY(mmb_moving)
__REG_PROPERTY(lockInfobox)
__REG_PROPERTY(soundAlias)
__REG_PROPERTY(videoAlias)
__REG_PROPERTY(playerName)
__REG_PROPERTY(lastGame)
__REG_PROPERTY(tooltipEnabled)
__REG_PROPERTY(screenshotDir)
__REG_PROPERTY(showTabletMenu)
__REG_PROPERTY(batchTextures)
__REG_PROPERTY(ccUseAI)
__REG_PROPERTY(metricSystem)
__REG_PROPERTY(defaultFont)
__REG_PROPERTY(celebratesConfig)
#undef __REG_PROPERTY

const vfs::Path defaultSaveDir = "saves";
const vfs::Path defaultResDir = "resources";
const vfs::Path defaultLocaleDir = "resources/locale";

class Settings::Impl
{
public:
  VariantMap options;
};

Settings& Settings::instance()
{
  static Settings inst;
  return inst;
}

Settings::Settings() : _d( new Impl )
{
  std::string application_path = vfs::Directory::applicationDir().toString();
  setwdir( application_path );

  _d->options[ pantheonModel       ] = std::string( "/pantheon.model" );
  _d->options[ sg2model            ] = std::string( "/sg2.model" );
  _d->options[ houseModel          ] = std::string( "/house.model" );
  _d->options[ constructionModel   ] = std::string( "/construction.model" );
  _d->options[ citiesModel         ] = std::string( "/cities.model" );
  _d->options[ ctNamesModel        ] = std::string( "/locale/names." );
  _d->options[ settingsPath        ] = std::string( "/settings.model" );
  _d->options[ langModel           ] = std::string( "/language.model" );
  _d->options[ archivesModel       ] = std::string( "/archives.model" );
  _d->options[ soundThemesModel    ] = std::string( "/sound_themes.model" );
  _d->options[ climateModel        ] = std::string( "/climate.model" );
  _d->options[ language            ] = std::string( "" );
  _d->options[ fastsavePostfix     ] = std::string( "_fastsave");
  _d->options[ saveExt             ] = std::string( ".oc3save");
  _d->options[ walkerModel         ] = std::string( "/walker.model" );
  _d->options[ animationsModel     ] = std::string( "/animations.model" );
  _d->options[ empireObjectsModel  ] = std::string( "/empire_objects.model" );
  _d->options[ emblemsModel        ] = std::string( "/emblems.model" );
  _d->options[ remakeModel         ] = std::string( "/remake.model" );
  _d->options[ ranksModel          ] = std::string( "/ranks.model" );
  _d->options[ pic_offsets         ] = std::string( "/offsets.model" );
  _d->options[ picsArchive         ] = std::string( "/gfx/pics.zip" );
  _d->options[ opengl_opts         ] = std::string( "/opengl.model" );
  _d->options[ freeplay_opts       ] = std::string( "/freeplay.model" );
  _d->options[ walkerRelations     ] = std::string( "/relations.model" );
  _d->options[ font                ] = std::string( "FreeSerif.ttf" );
  _d->options[ defaultFont         ] = std::string( "FreeSerif.ttf" );
  _d->options[ simpleAnimationModel] = std::string( "/basic_animations.model" );
  _d->options[ hotkeysModel        ] = std::string( "/hotkeys.model" );
  _d->options[ cartsModel          ] = std::string( "/carts.model" );
  _d->options[ logoArchive         ] = std::string( "/gfx/pics_wait.zip" );
  _d->options[ titleResource       ] = std::string( "titlerm" );
  _d->options[ forbidenTile        ] = std::string( "oc3_land" );
  _d->options[ layersOptsModel     ] = std::string( "layers_opts.model" );
  _d->options[ buildMenuModel      ] = std::string( "build_menu.model" );
  _d->options[ soundAlias          ] = std::string( "sounds.model" );
  _d->options[ videoAlias          ] = std::string( "videos.model" );
  _d->options[ celebratesConfig    ] = std::string( "romancelebs.model" );
  _d->options[ screenshotDir       ] = vfs::Directory::userDir().toString();
  _d->options[ batchTextures       ] = true;
  _d->options[ experimental        ] = false;
  _d->options[ needAcceptBuild     ] = false;
  _d->options[ borderMoving        ] = false;
  _d->options[ render              ] = std::string( "sdl" );
  _d->options[ scrollSpeed         ] = 30;
  _d->options[ mmb_moving          ] = false;
  _d->options[ tooltipEnabled      ] = true;
  _d->options[ ccUseAI             ] = false;
  _d->options[ c3gfx               ] = std::string( "" );
  _d->options[ c3video             ] = std::string( "" );
  _d->options[ c3music             ] = std::string( "" );
  _d->options[ talksArchive        ] = std::string( ":/audio/wavs_citizen_en.zip" );
  _d->options[ autosaveInterval    ] = 3;
  _d->options[ soundVolume         ] = 100;
  _d->options[ lockInfobox         ] = true;
  _d->options[ metricSystem        ] = metric::Measure::native;
  _d->options[ ambientVolume       ] = 50;
  _d->options[ cellw               ] = 60;
  _d->options[ musicVolume         ] = 25;
  _d->options[ resolution          ] = Size( 1024, 768 );
  _d->options[ fullscreen          ] = false;
  _d->options[ worldModel          ] = std::string( "/worldmap.model" );
  _d->options[ minMonthWithFood    ] = 3;
  _d->options[ worklessCitizenAway ] = 30;
  _d->options[ emigrantSalaryKoeff ] = 5.f;
  _d->options[ oldgfx              ] = 1;
  _d->options[ showTabletMenu      ] = false;

#ifdef CAESARIA_USE_STEAM
  _d->options[ oldgfx              ] = 0;
#endif

#ifdef CAESARIA_PLATFORM_ANDROID
  _d->options[ needAcceptBuild     ] = true;
  _d->options[ showTabletMenu      ] = true;
#endif
}

void Settings::set( const std::string& option, const Variant& value )
{
  instance()._d->options[ option ] = value;
}

Variant Settings::get( const std::string& option )
{
  VariantMap::iterator it = instance()._d->options.find( option );
  return  instance()._d->options.end() == it
              ? Variant()
              : it->second;
}

void Settings::setwdir( const std::string& wdirstr )
{
  vfs::Directory wdir( wdirstr );
  _d->options[ workDir ] = Variant( wdir.toString() );
  _d->options[ resourcePath ] = Variant( (wdir/defaultResDir).toString() );
  _d->options[ localePath ] = Variant( (wdir/defaultLocaleDir).toString() );
  _d->options[ savedir ] = Variant( (wdir/defaultSaveDir).toString() );

  vfs::Directory saveDir;
#ifdef CAESARIA_PLATFORM_LINUX
  vfs::Path dirName = vfs::Path( ".caesaria/" ) + defaultSaveDir;
  saveDir = vfs::Directory::userDir()/dirName;
#elif defined(CAESARIA_PLATFORM_WIN) || defined(CAESARIA_PLATFORM_HAIKU) || defined(CAESARIA_PLATFORM_MACOSX) || defined(CAESARIA_PLATFORM_ANDROID)
  saveDir = wdir/defaultSaveDir;
#endif
  _d->options[ savedir ] = Variant( saveDir.toString() );
}

void Settings::checkwdir(char* argv[], int argc)
{
  for (int i = 0; i < (argc - 1); i++)
  {
    if( !strcmp( argv[i], "-R" ) )
    {
      const char* opts = argv[i+1];
      setwdir( std::string( opts, strlen( opts ) ) );
      i++;
    }
  }
}

void Settings::checkCmdOptions(char* argv[], int argc)
{
  for (int i = 0; i < (argc - 1); i++)
  {
    if( !strcmp( argv[i], "-Lc" ) )
    {
      std::string opts = argv[i+1];
      _d->options[ language ] = Variant( opts ).toString();
      i++;
    }
    else if( !strcmp( argv[i], "-c3gfx" ) )
    {
      std::string opts = argv[i+1];
      _d->options[ c3gfx ] = Variant( opts ).toString();
      i++;
    }
    else if( !strcmp( argv[i], "-oldgfx" ) )
    {
      const char* opts = argv[i+1];
      _d->options[ oldgfx ] = utils::toInt( opts );
      i++;
    }
    else if( !strcmp( argv[i], "-cellw" ) )
    {
      const char* opts = argv[i+1];
      int cellWidth = utils::toInt( opts );
      _d->options[ cellw ] = cellWidth;
      i++;
    }
  }
}

void Settings::checkC3present()
{
  std::string c3path = _d->options[ c3gfx ].toString();
  int useOldGfx = _d->options[ oldgfx ];
  if( !c3path.empty() || useOldGfx )
  {
    _d->options[ houseModel          ] = Variant( std::string( "/house.c3" ) );
    _d->options[ constructionModel   ] = Variant( std::string( "/construction.c3" ) );
    _d->options[ citiesModel         ] = Variant( std::string( "/cities.c3" ) );
    _d->options[ climateModel        ] = Variant( std::string( "/climate.c3" ) );
    _d->options[ walkerModel         ] = Variant( std::string( "/walker.c3" ) );
    _d->options[ animationsModel     ] = Variant( std::string( "/animations.c3" ) );
    _d->options[ empireObjectsModel  ] = Variant( std::string( "/empire_objects.c3" ) );
    _d->options[ simpleAnimationModel] = Variant( std::string( "/basic_animations.c3" ) );
    _d->options[ cartsModel          ] = Variant( std::string( "/carts.c3" ) );
    _d->options[ worldModel          ] = Variant( std::string( "/worldmap.c3" ) );
    _d->options[ buildMenuModel      ] = Variant( std::string( "/build_menu.c3" ) );
    _d->options[ soundAlias          ] = Variant( std::string( "/sounds.c3" ) );
    _d->options[ videoAlias          ] = Variant( std::string( "/videos.c3" ) );
    _d->options[ pic_offsets         ] = Variant( std::string( "/offsets.c3" ) );
    _d->options[ forbidenTile        ] = Variant( std::string( "org_land" ) );
    _d->options[ titleResource       ] = Variant( std::string( "title" ) );
    _d->options[ cellw ] = 30;
  }
}

void Settings::changeSystemLang(const std::string& newLang)
{
  std::string lang = newLang.empty() ? "en" : newLang;
  Settings::set( language, Variant( lang ) );
}

static vfs::Path __concatPath( vfs::Directory dir, vfs::Path fpath )
{
  Variant vr = Settings::get( fpath.toString() );
  if( vr.isNull() )
  {
    return dir/fpath;
  }

  return dir/vfs::Path( vr.toString() );
}

vfs::Path Settings::rcpath( const std::string& option )
{
  std::string rc = get( resourcePath ).toString();

  return __concatPath( rc, option );
}

vfs::Path Settings::rpath( const std::string& option )
{
  std::string wd = get( workDir ).toString();

  return __concatPath( wd, option );
}

void Settings::load()
{
  VariantMap settings = config::load( rcpath( Settings::settingsPath ) );

  foreach( v, settings ) { set( v->first, v->second ); }
}

void Settings::save()
{
  config::save( instance()._d->options, rcpath( Settings::settingsPath ) );
}

}//end namespace game
