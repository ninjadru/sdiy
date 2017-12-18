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

#ifndef MIDIALF_LEDS_H_
#define MIDIALF_LEDS_H_

#include "avrlib/devices/led_array.h"

#include "midialf/hardware_config.h"

namespace midialf {
  
enum LedNumber {
  LED_SEQA,
  LED_SEQB,
  LED_SEQC,
  LED_SEQD,
  LED_RUN,
  LED_SEQ,
  LED_CLK,
  LED_SEL,
  LED_1,
  LED_2,
  LED_3,
  LED_4,
  LED_5,
  LED_6,
  LED_7,
  LED_8,
};
  
using avrlib::LedArray;

typedef LedArray<Led_Load, IOClock, Led_Serial, 2> Leds;

extern Leds leds;

} // namespace midialf

#endif  // MIDIALF_LEDS_H_
