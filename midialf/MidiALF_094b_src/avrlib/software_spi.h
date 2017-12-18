// Copyright 2013 Peter Kvitek
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
// Software SPI implementation

#ifndef AVRLIB_SOFTWARE_SPI_H_
#define AVRLIB_SOFTWARE_SPI_H_

#include <avr/io.h>

#include "avrlib/gpio.h"

namespace avrlib {

template<typename SS, typename SCK, typename MOSI>
class SoftwareSpiMasterOutput {
 public:

  static void Init() {
    SS::set_mode(DIGITAL_OUTPUT); SS::High();
    SCK::set_mode(DIGITAL_OUTPUT); SCK::Low();
    MOSI::set_mode(DIGITAL_OUTPUT); MOSI::High();
  }

  static inline void Strobe() {
    SS::High();
    SS::Low();
  }

#if 0
  static void Send(uint8_t byte) {
    byte & 0x80 ? MOSI::High() : MOSI::Low(); SCK::High(); SCK::Low();
    byte & 0x40 ? MOSI::High() : MOSI::Low(); SCK::High(); SCK::Low(); 
    byte & 0x20 ? MOSI::High() : MOSI::Low(); SCK::High(); SCK::Low();
    byte & 0x10 ? MOSI::High() : MOSI::Low(); SCK::High(); SCK::Low();
    byte & 0x08 ? MOSI::High() : MOSI::Low(); SCK::High(); SCK::Low();
    byte & 0x04 ? MOSI::High() : MOSI::Low(); SCK::High(); SCK::Low();
    byte & 0x02 ? MOSI::High() : MOSI::Low(); SCK::High(); SCK::Low();
    byte & 0x01 ? MOSI::High() : MOSI::Low(); SCK::High(); SCK::Low();
    MOSI::High();
  }
#else
  static void Send(uint8_t byte) {
    for (uint8_t n = 8; n--; ) {
      SCK::Low();

      if (byte & 0x80) {
        MOSI::High();
      } else
        MOSI::Low();

      byte<<= 1;

      SCK::High();
    }
    
    SCK::Low();
    MOSI::High();
  }
#endif

  static void Write(uint8_t byte) {
    uint8_t oldSREG = SREG;
    cli();

    Send(byte);

    SREG = oldSREG;
  }

  static void WriteWord(uint8_t byte1, uint8_t byte2) {
    uint8_t oldSREG = SREG;
    cli();

    Send(byte1);
    Send(byte2);

    SREG = oldSREG;
  }

  static inline void Wait() { 
    __asm__ volatile (
      "nop" // 50ns @20MHz
      ); 
  }
};

}  // namespace avrlib

#endif   // AVRLIB_SOFTWARE_SPI_H_
