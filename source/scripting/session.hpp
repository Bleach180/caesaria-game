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

#ifndef _CAESARIA_SCRIPT_SESSION_INCLUDE_H_
#define _CAESARIA_SCRIPT_SESSION_INCLUDE_H_

#include "core/namedtype.hpp"
#include "core/size.hpp"
#include "gui/predefinitions.hpp"
#include "world/predefinitions.hpp"
#include "religion/romedivinity.hpp"
#include "game/predefinitions.hpp"
#include <string>

class Game;
class PlayerCity;
class StringArray;
namespace gfx { class Picture; class Camera; }

namespace script
{

class Session
{
public:
  Session(Game* game) { _game = game; }
  void continuePlay(int years);
  void loadNextMission();
  void setMode(int mode);
  void setOption(const std::string& name,Variant v);
  Variant getOption(std::string name);
  void showSysMessage(std::string title, std::string message);
  void clearUi();
  void save(const std::string& path);
  void createDir(const std::string& dir);
  int getAdvflag(const std::string& flag) const;
  void setAdvflag(const std::string& flag, int value);
  void loadLocalization(const std::string& name);
  void openUrl(const std::string& url);
  int lastChangesNum() const;
  void addWarningMessage(const std::string& message);
  PlayerPtr getPlayer() const;
  PlayerCityPtr getCity() const;
  bool isC3mode() const;
  bool isSteamAchievementReached(int i);
  gfx::Picture getSteamUserImage() const;
  gfx::Picture getSteamAchievementImage(int i) const;
  std::string getSteamAchievementCaption(int id) const;
  world::Emperor* getEmperor() const;
  world::EmpirePtr getEmpire() const;
  void clearHotkeys();
  void setHotkey(const std::string& name, const std::string& config);
  void setRank(int i, const std::string& name, const std::string& pretty, int salary);
  DateTime getGameDate() const;
  StringArray getCredits() const;
  StringArray getFiles(const std::string& dir, const std::string& ext);
  StringArray getFolders(const std::string& dir, bool full);
  int videoModesCount() const;
  void playAudio(const std::string& filename, int volume, int mode);
  Size getVideoMode(int index) const;
  void setResolution(const Size& size);
  void showDlcViewer(const std::string& path);
  StringArray tradableGoods() const;
  Point getCursorPos() const;
  religion::DivinityPtr addGod(const std::string& id);
  std::string getOverlayType(int i) const;
  std::string getWalkerType(int i) const;
  VariantMap getGoodInfo(std::string goodName) const;
  gui::Widget* findWidget(std::string wname) const;
  Size getResolution() const;
  Locations getBuildingLocations(Variant type) const;
  OverlayList getOverlays(Variant type) const;
  OverlayList getWorkingBuildings() const;
  city::RequestPtr getRequest(int index) const;
  uint32_t getOverlaysNumber(Variant var) const;
  gfx::Camera* getCamera() const;
  void setFont(const std::string& fontname);
  void assignFestival(const std::string& name, int size);
  void setLanguage(const std::string& lang,const std::string& audio);

private:
  Game* _game;
};

} //end namespace script

#endif  //_CAESARIA_SCRIPT_SESSION_INCLUDE_H_
