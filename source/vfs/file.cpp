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

#include "file.hpp"
#include "filesystem.hpp"
#include "path.hpp"
#include "core/logger.hpp"
#include "core/platform_specific.hpp"

#ifdef GAME_PLATFORM_WIN
  #define getline_def getline_win
#elif defined(GAME_PLATFORM_UNIX)
  #define getline_def getline
#endif

namespace vfs
{

static const Path purePath = "";

NFile NFile::open(const Path& fileName, Entity::Mode mode)
{
  return FileSystem::instance().createAndOpenFile( fileName, mode );
}

NFile::NFile()
{
  #ifdef _DEBUG
    setDebugName("NFile");
  #endif

	_entity = 0;
}

NFile::NFile( FSEntityPtr file )
{
#ifdef _DEBUG
  setDebugName("NFile");
#endif
  _entity = file;
}

NFile::NFile(const NFile& nfile)
{
  *this = nfile;
}

NFile::~NFile()
{
}

bool NFile::isEof() const
{
  return _entity.isValid() ? _entity->isEof() : false;
}

//! returns how much was read
int NFile::read(void* buffer, unsigned int sizeToRead)
{
  return _entity.isValid() ? _entity->read( buffer, sizeToRead ) : 0;
}

//! changes position in file, returns true if successful
//! if relativeMovement==true, the pos is changed relative to current pos,
//! otherwise from begin of file
bool NFile::seek(long finalPos, bool relativeMovement)
{
  return _entity.isValid() ? _entity->seek( finalPos, relativeMovement ) : 0;
}


//! returns size of file
long NFile::size() const
{
  return _entity.isValid() ? _entity->size() : 0;
}


//! returns where in the file we are.
long NFile::getPos() const
{
	return _entity.isValid() ? _entity->getPos() : 0;
}

ByteArray NFile::readLine()
{
  return _entity.isValid() ? _entity->readLine() : ByteArray();
}

ByteArray NFile::read( unsigned int sizeToRead)
{
  return _entity.isValid() ? _entity->read( sizeToRead ) : ByteArray();
}

ByteArray NFile::readAll()
{
  if( _entity.isValid() )
  {
    seek( 0 );
    return read( size() );
  }

  return ByteArray();
}

//! returns name of file
const Path& NFile::path() const
{
	return _entity.isValid() ? _entity->path() : purePath;
}

//! returns how much was read
int NFile::write(const void* buffer, unsigned int sizeToWrite)
{
  return _entity.isValid() ? _entity->write( buffer, sizeToWrite ) : 0;
}

int NFile::write(const std::string& str )
{
  return write( str.c_str(), str.size() );
}

int NFile::write( const ByteArray& bArray )
{
  return _entity.isValid() ? _entity->write( bArray ) : 0;
}

bool NFile::isOpen() const
{
  return _entity.isValid() ? _entity->isOpen() : false;
}

void NFile::flush()
{
  if( _entity.isValid() )
  {
	  _entity->flush();
  }
}

size_t NFile::lastModify() const
{
  return _entity.isValid() ? _entity->lastModify() : 0;
}

NFile& NFile::operator=( const NFile& other )
{
  _entity = other._entity;

  return *this;
}

unsigned long NFile::size(vfs::Path filename)
{
  NFile file = NFile::open( filename );
  return file.size();
}

bool NFile::remove( Path filename )
{
#ifdef GAME_PLATFORM_WIN
  BOOL result = DeleteFileA( filename.toCString() );
  if( !result )
  {
    /*int error = */GetLastError();
    //Logger::warning( "Error[%d] on removed file %s", error, filename.toString().c_str() );
  }
  return result;
#elif defined(GAME_PLATFORM_UNIX) || defined(GAME_PLATFORM_HAIKU)
  int result = ::remove( filename.toCString() );
  return (result == 0);
#endif
}

bool NFile::rename(Path oldpath, Path newpath)
{
#ifdef GAME_PLATFORM_WIN
  bool result = MoveFileExA( oldpath.toCString(), newpath.toCString(), MOVEFILE_REPLACE_EXISTING );

  if( !result )
  {
    /*int error = */GetLastError();
    //Logger::warning( "Error[%d] on renamed file %s to %s", error, oldpath.toString().c_str(), newpath.toString().c_str() );
  }
  return result;
#else
  int result = ::rename( oldpath.toCString(), newpath.toCString() );

  if( result != 0 )
  {
    Logger::warning( "Error[{}] on renamed file {} to {}", result, oldpath.toCString(), newpath.toCString() );
  }
  return (result == 0);
#endif
}

} //end namespace io
