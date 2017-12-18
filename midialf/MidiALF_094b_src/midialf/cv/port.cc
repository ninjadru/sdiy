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

#include "midialf/cv/port.h"

#ifdef ENABLE_CV_OUTPUT

namespace midialf {

using namespace avrlib;

/* extern */
Port port;

/* <static> */
uint8_t Port::value_;
uint8_t Port::latched_value_;
uint8_t Port::strobe_count_[4];
uint8_t Port::strobe_ticks_;
uint8_t Port::strobe_mask_;
uint8_t Port::inv_gate_;
/* </static> */

#define SS_LOW  { SSEnable::High(); SSSelector::Write(3); SSEnable::Low(); }
#define SS_HIGH { SSEnable::High(); SSSelector::Write(0); }

static SPIInterface spi;

const prog_uint8_t strobe_ticks_from_width[kMaxStrobeWidth + 1] PROGMEM = {
  5,  // 1ms
  10, // 2ms
  15, // 3ms
  20, // 4ms
  25, // 5ms
};

/* static */
void Port::Init() {
  spi.Init();
  SSSelector::set_mode(DIGITAL_OUTPUT);
  SSEnable::set_mode(DIGITAL_OUTPUT);
  SSEnable::High();

  SS_LOW;
  spi.Send(0x40);
  spi.Send(0x00); // IODIR
  spi.Send(0x00); // output
  SS_HIGH;

  Write(latched_value_);
}

/* static */
void Port::Write(uint8_t value) {
  DisableInterrupts di;
  SS_LOW;
  spi.Send(0x40);
  spi.Send(0x0a); // GPIO
  spi.Send(value ^ inv_gate_);
  SS_HIGH;
}

/* static */
void Port::Tick() {
  // This takes 0.8 or 4us
  if (value_ != latched_value_) {
    Write(value_);
    latched_value_ = value_;
  }

  // Update strobe state
  if (strobe_mask_) {
    uint8_t mask = 1<<4;
    for (uint8_t n = 0; n < 4; n++, mask<<= 1) {
      if (strobe_mask_ & mask) {
        if (!strobe_count_[n] || --strobe_count_[n] == 0) {
          CLRFLAG(strobe_mask_, mask);
          CLRFLAG(value_, mask);
        }
      }
    }
  }
}

/* static */
void Port::SetState(uint8_t index, uint8_t on) {
  uint8_t mask = 1<<4;
  for (uint8_t n = 0; n < 4; n++, mask<<= 1) {
    if (seq.gate_mode(n) == index) {
      SETFLAGTO(value_, mask, on);
    }
  }
}

/* static */
void Port::SetPulse(uint8_t index) {
  uint8_t mask = 1<<4;
  for (uint8_t n = 0; n < 4; n++, mask<<= 1) {
    if (seq.gate_mode(n) == index) {
      SETFLAG(value_, mask);
      SETFLAG(strobe_mask_, mask);
      strobe_count_[n] = strobe_ticks_;
    }
  }
}

/* static */
void Port::UpdateCvOffset() {
  uint8_t mask = 1;
  for (uint8_t n = 0; n < 4; n++, mask<<= 1) {
    SETFLAGTO(value_, mask, seq.cv_mode_offset(n));
  }
}

/* static */
void Port::UpdateGateInvert() {
  uint8_t old_inv_gate_ = inv_gate_; inv_gate_ = 0;

  uint8_t mask = 1<<4;
  for (uint8_t n = 0; n < 4; n++, mask<<= 1) {
    SETFLAGTO(inv_gate_, mask, seq.gate_mode_invert(n));
  }

  // Force update
  if (inv_gate_ != old_inv_gate_) {
    latched_value_^= 0xff;
  }
}

/* static */
void Port::UpdateStrobeWidth() {
  strobe_ticks_ = ResourcesManager::Lookup<uint8_t, uint8_t>(
        strobe_ticks_from_width, seq.strobe_width());
}

} // namespace midialf

#endif // #ifdef ENABLE_CV_OUTPUT
