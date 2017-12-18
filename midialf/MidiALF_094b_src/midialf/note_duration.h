// Copyright 2013 Peter Kvitek.
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

#ifndef MIDIALF_NOTE_DURATION_H_
#define MIDIALF_NOTE_DURATION_H_

#include "avrlib/base.h"
#include "midialf/resources.h"

namespace midialf {

enum NoteDurations {
  k64tNote,
  k32tNote,
  k32ndNote,
  k16tNote,
  k16thNote,
  k8tNote,
  k16dNote,
  k8thNote,
  k4tNote,
  k8dNote,
  k4thNote,
  k2tNote,
  k4dNote,
  k2ndNote,
  k1btNote,
  k2dNote,
  k1bNote,
  k2btNote,
  k15bNote,
  k2bNote,
  kNoteDurationCount
};

class NoteDuration {
 public:
  NoteDuration() {}

  static uint8_t GetMidiClockTicks(uint8_t interval);

private:

  DISALLOW_COPY_AND_ASSIGN(NoteDuration);
};

extern const prog_uint8_t midi_clock_ticks_per_note[kNoteDurationCount];
extern const prog_char midi_clock_ticks_per_note_str[kNoteDurationCount * 4 + 1];

}  // namespace midialf

#endif  // MIDIALF_NOTE_DURATION_H_
