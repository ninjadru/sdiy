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
// State saver.

#ifndef MIDIALF_STATE_H_
#define MIDIALF_STATE_H_

#include "midialf/midialf.h"

namespace midialf {

class State {
 public:
  static void Save();
  static void Load();

private:
  static uint16_t crc16_;

  static uint16_t CalcCrc16();
};

extern State state;

} // namespace midialf

#endif // MIDIALF_STATE_H_
