// Copyright 2012 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Scale functions.

#ifndef MIDIALF_SCALE_H_
#define MIDIALF_SCALE_H_

#include "midialf/midialf.h"

namespace midialf {

static const uint8_t kScaleNameSize = 20;
static const uint8_t kDefScale = 0; // Major

class Scale {
 public:
  static uint8_t count();
  static uint8_t get_note(uint8_t scale, uint8_t index);
  
  static uint8_t GetScaledNote(uint8_t scale, uint8_t note);
  static uint8_t GetNextScaledNote(uint8_t scale, uint8_t note);
  static uint8_t GetPrevScaledNote(uint8_t scale, uint8_t note);
  static void GetScaleName(uint8_t scale, uint8_t* buffer);

private:

};

} // namespace midialf

#endif // MIDIALF_SCALE_H_
