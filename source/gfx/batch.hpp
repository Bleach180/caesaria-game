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

#ifndef __CAESARIA_BATCH_H_INCLUDED__
#define __CAESARIA_BATCH_H_INCLUDED__

#include "core/rect_array.hpp"

struct SDL_Batch;
class Point;

namespace gfx
{
class Picture;
class Pictures;

class Batch
{
public:
  Batch();
  Batch( SDL_Batch* batch );
  Batch( const Batch& other );
  Batch& operator=(const Batch& other);
  inline SDL_Batch* native() const { return _batch; }
  inline bool valid() const { return _batch != 0; }
  inline void init( SDL_Batch* batch ) { _batch = batch; }
  void destroy();
  bool load(const Pictures& pics, const Rects& dstrects);
  bool load(const Pictures& pics, const Point& pos);
  void load(const Picture& pics, const Rects& srcrects, const Rects& dstrects);
private:

  SDL_Batch* _batch;
};

}//end namespace gfx

#endif //__CAESARIA_BATCH_H_INCLUDED__
