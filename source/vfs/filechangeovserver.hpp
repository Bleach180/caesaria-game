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
// Copyright 2012-2016 Dalerank, dalerankn8@gmail.com

#ifndef __CAESARIA_FILECHANGEOBSERVER_H_INCLUDED__
#define __CAESARIA_FILECHANGEOBSERVER_H_INCLUDED__

#include <GameCore>
#include <GameVfs>

namespace vfs
{

class FileChangeObserver
{
public:
  FileChangeObserver();
  ~FileChangeObserver();

  void watch( vfs::Directory dir );
  void watch( const std::string& dir );
  void run( bool& continues );

  Signal1<vfs::Path>& onFileChange();
private:
  class Impl;
  ScopedPtr<Impl> _d;
};

}//end namespace vfs

#endif //__CAESARIA_FILECHANGEOBSERVER_H_INCLUDED__
