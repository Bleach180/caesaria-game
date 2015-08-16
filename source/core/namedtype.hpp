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
// Copyright 2012-2013 Dalerank, dalerankn8@gmail.com

#ifndef __CAESARIA_NAMEDTYPE_INCLUDE_HPP__
#define __CAESARIA_NAMEDTYPE_INCLUDE_HPP__

#define BEGIN_NAMEDTYPE(type,start) enum type { start=0

#define APPEND_NAMEDTYPE(name) ,name
#define APPEND_NAMEDTYPE_ID(name,id) ,name=id

#define END_NAMEDTYPE(type) \
  }; \
  inline type& operator++(type& a) { a = type(a+1); return a; }

#define REGISTER_NAMEDTYPE(type,name,id) static const type name = type(id);

#endif //__CAESARIA_NAMEDTYPE_INCLUDE_HPP__
