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
//
// -----------------------------------------------------------------------------
//
// CV extention board port.

#ifndef MIDIALF_PORT_H_
#define MIDIALF_PORT_H_

#ifdef ENABLE_CV_OUTPUT

#include "midialf/midialf.h"
#include "midialf/seq.h"

namespace midialf {

class Port {
 public:
  Port() {}
  static void Init();
  static void Write(uint8_t value);
  static void Tick();

  static void set(uint8_t value) {
    value_ = value;
  }

  static uint8_t get() {
    return value_;
  }

  static void SetState(uint8_t index, uint8_t on);
  static void SetPulse(uint8_t index);

  static void SendGate(uint8_t on) { SetState(GATEMODE_GATE, on); }
  static void SendStart(uint8_t on) { SetState(GATEMODE_START, on); }

  static void SendStrobe() { SetPulse(GATEMODE_STROBE); }
  static void SendClock() { SetPulse(GATEMODE_CLOCK); }
  static void SendSeq() { SetPulse(GATEMODE_SEQ); }
  static void SendLFO(uint8_t lfo) { SetPulse(GATEMODE_LFO1 + lfo); }

  static void UpdateCvOffset();
  static void UpdateGateInvert();
  static void UpdateStrobeWidth();

 private:

  static uint8_t value_;
  static uint8_t latched_value_;
  static uint8_t strobe_count_[4];
  static uint8_t strobe_ticks_;
  static uint8_t strobe_mask_;
  static uint8_t inv_gate_;

  DISALLOW_COPY_AND_ASSIGN(Port);
};

extern Port port;

} // namespace midialf

#endif // #ifdef ENABLE_CV_OUTPUT

#endif // MIDIALF_PORT_H_
