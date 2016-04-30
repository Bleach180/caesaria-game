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

#ifndef __CAESARIA_BYTEARRAY_H_INCLUDED__
#define __CAESARIA_BYTEARRAY_H_INCLUDED__

#include <vector>
#include <string>
#include <cstring>

class ByteArray
{
public:
  ByteArray();

  explicit ByteArray( unsigned int cap );
  explicit ByteArray( const std::string& str );

  ByteArray& operator=( const std::string& str );

  ByteArray copy( unsigned int start, int length=-1) const;

  const char* data() const;

  char* data();

  std::string toString() const;
  bool empty() const;
  char& operator[](size_t index);

  ByteArray& push_back(char c);
  char& back();

  ByteArray& operator=(const ByteArray& other);
  bool operator == (const ByteArray& other) const;

  void clear();
  size_t size() const;
  void resize(size_t value);

  unsigned long crc32(unsigned long crc);

  static ByteArray fromBase64(const std::string &encoded_string);
  std::string base64() const;

  static unsigned long CRC32(unsigned long crc, const char* data, size_t length);
private:
  std::vector<char> _data;
};

#endif //__CAESARIA_BYTEARRAY_H_INCLUDED__
