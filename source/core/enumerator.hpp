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

#ifndef __CAESARIA_ENUMS_HELPER_H_INCLUDED__
#define __CAESARIA_ENUMS_HELPER_H_INCLUDED__

#include "core/requirements.hpp"
#include <map>
#include <string>

template< class T >
class EnumsHelper
{
public:
  const std::string noText;
  T findType(const std::string& name) const {
    for (auto& it : _equales) {
      if (name == it.second) {
        return it.first;
      }
    }

    return getInvalid();
  }

  const std::string& findName(const T& type) const {
    auto it = _equales.find( type );
    return it != _equales.end() ? it->second : noText;
  }

  bool empty() const { return _equales.empty(); }

  void append(const T& key, const std::string& name) {
    _equales[ key ] = name;
  }

  T getInvalid() const { return _invalid; }

  ~EnumsHelper() {}
  EnumsHelper(const T& invalid) : _invalid(invalid) {}

protected:
  typedef std::pair< T, std::string > TypeEquale;
  typedef std::map< T, std::string > Equales;

  Equales _equales;
  T _invalid;
};

#endif //__CAESARIA_ENUMS_HELPER_H_INCLUDED__
