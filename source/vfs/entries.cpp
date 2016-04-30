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
// Copyright 2012-2014 Dalerank, dalerank@gmail.com

#include "entries.hpp"
#include "core/foreach.hpp"
#include "core/logger.hpp"
#include "core/osystem.hpp"
#include "core/utils.hpp"

#include <map>

namespace vfs
{

static const Path emptyFileListEntry( "" );

class Entries::Impl
{
public:
  //! Ignore paths when adding or searching for files
  bool ignorePaths;
  Path::SensType sensType;
  //! Path to the file list
  Path path;
  Entries::Items files;

  typedef std::map< unsigned int, unsigned int> HashedIndex;
  HashedIndex hashedIndex;
  HashedIndex hashedIcIndex;

  Path checkCase(const Path& p );
};

Entries::Entries( const Path& path, Path::SensType type, bool ignorePaths )
 : _d( new Impl )
{
#ifdef _DEBUG
  setDebugName( "FileList" );
#endif
  _d->ignorePaths = ignorePaths;
  _d->path = utils::replace( path.toString(), "\\", "/" );
  _d->sensType = type;
}

Entries::Entries( const Entries& other ) : _d( new Impl )
{
  *this = other;
}

Entries& Entries::operator=( const Entries& other )
{
  _d->sensType = other._d->sensType;
  _d->ignorePaths = other._d->ignorePaths;
  _d->path = other._d->path;
  _d->files.clear();

  for (auto& f : other._d->files) {
    _d->files.push_back(f);
  }

  _updateCache();

  return *this;
}

Entries::ConstItemIt Entries::begin() const {  return _d->files.begin(); }
Entries::ConstItemIt Entries::end() const{  return _d->files.end(); }
Entries::Items& Entries::_items(){  return _d->files; }

void Entries::_updateCache()
{
  _d->hashedIndex.clear();
  _d->hashedIcIndex.clear();
  int k=0;
  for (auto& info : _d->files) {
    info.fphash = info.fullpath.hash();
    info.nhash = info.name.hash();
    info.nihash = info.name.canonical().hash();

    _d->hashedIndex[ info.nhash ] = k;
    _d->hashedIcIndex[ info.nihash ] = k;
    k++;
  }
}

Entries::~Entries(){}

unsigned int Entries::getFileCount() const
{
  return _d->files.size();
}

void Entries::sort()
{
  std::sort( _d->files.begin(), _d->files.end() );

  _updateCache();
}

const Path& Entries::getFileName(unsigned int index) const
{
  if (index >= _d->files.size())
    return emptyFileListEntry;

  return _d->files[index].name;
}


//! Gets the full name of a file in the list, path included, based on an index.
const Path& Entries::getFullFileName(unsigned int index) const
{
  if (index >= _d->files.size())
    return emptyFileListEntry;

  return _d->files[index].fullpath;
}

//! adds a file or folder
unsigned int Entries::addItem( const Path& fullPath, unsigned int offset, unsigned int size, bool isDirectory, unsigned int id )
{
  EntryInfo entry;
  entry.uid   = id ? id : _d->files.size();
  entry.offset = offset;
  entry.size = size;
  entry.isDirectory = isDirectory;

  Path tmpPath = utils::replace( fullPath.toString(), "\\", "/" );

  // remove trailing slash
  if( tmpPath.lastChar() == '/')
  {
    entry.isDirectory = true;
    entry.name = tmpPath.removeEndSlash();
    //entry.name.validate();
  }

  entry.fullpath = _d->checkCase( tmpPath );
  entry.name = tmpPath.baseName();

  if(_d->ignorePaths)
  {
    entry.fullpath = entry.name;
  }

  _d->files.push_back(entry);

  return _d->files.size() - 1;
}

//! Returns the iD of a file in the file list, based on an index.
unsigned int Entries::getID(unsigned int index) const
{
  return index < _d->files.size() ? _d->files[index].uid : 0;
}

bool Entries::isDirectory(unsigned int index) const
{
  bool ret = false;
  if (index < _d->files.size())
    ret = _d->files[index].isDirectory;

  return ret;
}

//! Returns the size of a file
unsigned int Entries::getFileSize(unsigned int index) const
{
  return index < _d->files.size() ? _d->files[index].size : 0;
}

//! Returns the size of a file
unsigned int Entries::getFileOffset(unsigned int index) const
{
  return index < _d->files.size() ? _d->files[index].offset : 0;
}

//! Searches for a file or folder within the list, returns the index
int Entries::findFile(const Path& filename, bool isDirectory) const
{
  EntryInfo entry;
  // we only need fullName to be set for the search
  entry.fullpath = utils::replace( filename.toString(), "\\", "/" );
  entry.isDirectory = isDirectory;

  if( entry.fullpath.lastChar() == '/' )
  {
    entry.isDirectory = true;
  }
  entry.fullpath = entry.fullpath.removeEndSlash();
  entry.fullpath = _d->checkCase( entry.fullpath );

  if( _d->ignorePaths )
  {
    entry.fullpath = entry.fullpath.baseName();
  }

  Path::SensType sType = _d->sensType;
  if( _d->sensType == Path::nativeCase )
  {
#if defined(GAME_PLATFORM_UNIX) || defined(GAME_PLATFORM_UNIXU)
    sType = Path::equaleCase;
#elif defined(GAME_PLATFORM_WIN)
    sType = Path::ignoreCase;
#endif
  }

  std::string fname = filename.baseName().toString();
  unsigned int fnHash = (sType == Path::ignoreCase
                            ? Hash( utils::localeLower( fname ) )
                            : Hash( fname )
                        );

  if( _d->hashedIndex.empty() )
  {
    Logger::warning( "WARNING: Entries::findFile cache not initialized. Used slow linear search" );
    foreach( it, _d->files )
    {
      bool equale = false;
      switch( sType )
      {
      case Path::equaleCase: equale = it->nhash == fnHash; break;
      case Path::ignoreCase: equale = it->nihash == fnHash; break;
      default: break;
      }

      if( equale )
      {
        return std::distance( _d->files.begin(), it );
      }
    }
  }
  else
  {
    switch( sType )
    {
    case Path::equaleCase:
    {
      auto it = _d->hashedIndex.find( fnHash );
      if( it != _d->hashedIndex.end() ) return it->second;
    }
    break;
    case Path::ignoreCase:
    {
      auto it = _d->hashedIcIndex.find( fnHash );
      if( it != _d->hashedIcIndex.end() ) return it->second;
    }
    break;

    default: break;
    }
  }

  return -1;
}


//! Returns the base path of the file list
const Path& Entries::getPath() const
{
  return _d->path;
}

void Entries::setSensType( Path::SensType type )
{
  _d->sensType = type;
}

Entries Entries::filter(int flags, const std::string& options)
{
  Entries ret;
  bool isFile = (flags & Entries::file) > 0;
  bool isDirectory = (flags & Entries::directory) > 0;
  bool checkFileExt = (flags & Entries::extFilter) > 0;

  StringArray exts = utils::split( utils::trim( options ), "," );

  for( const auto& item : _d->files )
  {
    bool mayAdd = true;
    if( isFile ) { mayAdd = !item.isDirectory; }
    if( !mayAdd && isDirectory ) { mayAdd = item.isDirectory; }

    if( mayAdd && !item.isDirectory && checkFileExt )
    {
      if( exts.size() > 1 )
      {
        mayAdd = exts.contains( item.fullpath.extension() );
      }
      else
      {
        mayAdd = item.fullpath.isMyExtension( options );
      }
    }

    if( mayAdd )
    {
      ret._d->files.push_back( item );
    }
  }

  return ret;
}

const Entries::Items& Entries::items() const {  return _d->files; }

const EntryInfo& Entries::item(unsigned int index) const
{
  return _d->files[ index ];
}

Path Entries::Impl::checkCase( const Path& p)
{
  switch( sensType )
  {
  case Path::ignoreCase:
    return p.canonical();
  break;
  case Path::equaleCase: break;
  case Path::nativeCase:
    if( OSystem::isWindows() )
    {
      return p.canonical();
    }
    else if( OSystem::isUnix() )
    {
      return p;
    }
  break;
  }

  return p;
}

StringArray Entries::Items::names() const
{
  StringArray ret;

  for( auto& item : *this )
    ret << item.name.toString();

  return ret;
}

StringArray Entries::Items::fullnames() const
{
  StringArray ret;

  for( auto& item : *this )
    ret << item.fullpath.toString();

  return ret;
}

StringArray Entries::Items::files(const std::string& ext) const
{
  StringArray ret;

  bool any = ext.empty();
  for (auto& item : *this)
  {
    if (any || item.fullpath.isMyExtension( ext ))
      ret << item.fullpath.toString();
  }

  return ret;
}

StringArray Entries::Items::folders(bool full) const
{
  StringArray ret;

  StringArray excludeFolders;
  excludeFolders << vfs::Path::firstEntry << vfs::Path::secondEntry;

  for (const auto& item : *this)
  {
    if (item.isDirectory)
    {
      if (excludeFolders.contains(item.name.toString()))
        continue;

      ret << (full ? item.fullpath.toString() : item.name.toString());
    }
  }

  return ret;
}

} //end namespace io
