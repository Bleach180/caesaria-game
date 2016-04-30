#include "packager.hpp"

#include "constants.hpp"
#include "util.hpp"
#include "inifile.hpp"
#include "core/utils.hpp"
#include "core/format.hpp"
#include "vfs/file.hpp"
#include "vfs/directory.hpp"
#include "vfs/entries.hpp"
#include "vfs/entryinfo.hpp"

namespace updater
{

Packager::Packager(std::string baseset, std::string currentVersion)
  : _baseset( baseset ), _crver( currentVersion )
{
}

typedef std::vector<vfs::Path> FilePathList;

static void __gartherFiles( vfs::Directory basedir, vfs::Directory dir, FilePathList& files )
{
  vfs::Entries entries = dir.entries();
  for (auto& i : entries) {
    if( i.name.isDirectoryEntry() )
      continue;

    if( i.fullpath.isFolder() )
    {
      __gartherFiles( basedir, i.fullpath, files );
    }
    else
    {
      files.push_back( basedir.relativePathTo( i.fullpath ) );
    }
  }
}

std::string getFilePlatforms(vfs::Path &path) {
  std::string extension = path.extension();
  if (extension.empty()) return "";
  if ("dll" == extension) return "win32,win64";
  if ("exe" == extension) return "win32,win64";
  if ("linux" == extension) return "linux";
  if ("linux64" == extension) return "linux,linux64";
  if ("macos" == extension) return "macosx";
  if ("haiku" == extension) return "haiku";
  return "";
}

void Packager::createUpdate( bool release )
{
  FilePathList allFiles;

  vfs::Directory::switchTo( _baseset );
  vfs::Directory dir = vfs::Directory::current();

  __gartherFiles( dir, dir, allFiles );

  IniFilePtr vinfo = IniFile::Create();
  vinfo->SetValue( "version", "version", _crver );
  for( FilePathList::iterator i=allFiles.begin(); i != allFiles.end(); i++ )
  {
    std::string baseName = (*i).toString();
    if( baseName == STABLE_VERSION_FILE ||
        baseName == UPDATE_VERSION_FILE )
    {
        continue;
    }

    std::string sectionName;
    if( release )
    {
      sectionName = fmt::format( "File {}", baseName );
    }
    else
    {
      sectionName = fmt::format( "Version{} File {}", _crver, baseName );
    }

    ByteArray data = vfs::NFile::open( (*i).absolutePath() ).readAll();

    unsigned int crc = data.crc32( 0 );
    vinfo->SetValue( sectionName, "crc", utils::format( 0xff, "%x", crc ) );
    vinfo->SetValue( sectionName, "filesize", utils::i2str( data.size() ) );
    // Add platforms information info release file
    std::string platforms = getFilePlatforms(*i);
    if (!platforms.empty()) {
      vinfo->SetValue( sectionName, "platforms", platforms );
    }
  }

  vinfo->ExportToFile( release
                        ? dir/STABLE_VERSION_FILE
                        : dir/UPDATE_VERSION_FILE );
}


}//end namespace updater
