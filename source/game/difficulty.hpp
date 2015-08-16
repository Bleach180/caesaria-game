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
// Copyright 2012-2015 Dalerank, dalerankn8@gmail.com

#ifndef _CAESARIA_DIFFICULTY_H_INCLUDE_
#define _CAESARIA_DIFFICULTY_H_INCLUDE_

#include "vfs/directory.hpp"

namespace game
{

namespace difficulty
{
enum Type{ fun=0, easy, simple, usual, nicety, hard, impossible, count };
const char* const name[count] = {
                            CAESARIA_STR_EXT(fun),
                            CAESARIA_STR_EXT(easy),
                            CAESARIA_STR_EXT(simple),
                            CAESARIA_STR_EXT(usual),
                            CAESARIA_STR_EXT(nicety),
                            CAESARIA_STR_EXT(hard),
                            CAESARIA_STR_EXT(impossible)
                          };
}

typedef difficulty::Type Difficulty;

} //end namespace movie

#endif //_CAESARIA_DIFFICULTY_H_INCLUDE_

