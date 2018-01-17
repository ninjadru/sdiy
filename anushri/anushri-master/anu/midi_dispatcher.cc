// Copyright 2009 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
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
// Instance of the midi out filter class.

#include "anu/midi_dispatcher.h"

#include "anu/hardware_config.h"

namespace anu {

/* static */
bool MidiDispatcher::learning_midi_channel_ = false;

/* static */
bool MidiDispatcher::seen_midi_drum_events_ = false;

/* static */
void MidiDispatcher::Send(uint8_t status, uint8_t* data, uint8_t size) {
  OutputBufferLowPriority::Overwrite(status);
  if (size) {
    OutputBufferLowPriority::Overwrite(*data++);
    --size;
  }
  if (size) {
    OutputBufferLowPriority::Overwrite(*data++);
    --size;
  }
}

/* static */
void MidiDispatcher::SendBlocking(uint8_t byte) {
  OutputBufferLowPriority::Write(byte);
}

/* static */
void MidiDispatcher::SendNow(uint8_t byte) {
  OutputBufferHighPriority::Overwrite(byte);
}

/* static */
void MidiDispatcher::Send3(uint8_t status, uint8_t a, uint8_t b) {
  OutputBufferLowPriority::Overwrite(status);
  OutputBufferLowPriority::Overwrite(a);
  OutputBufferLowPriority::Overwrite(b);
}

/* extern */
MidiDispatcher midi_dispatcher;

}  // namespace anu
