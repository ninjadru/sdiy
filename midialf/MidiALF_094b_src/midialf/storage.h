// Copyright 2012 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// Based on Shruthi-1 code by Olivier Gillet (ol.gillet@gmail.com)
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
// Program storage in external EEPROM.

#ifndef MIDIALF_STORAGE_H_
#define MIDIALF_STORAGE_H_

#include <string.h>

#include "avrlib/base.h"
#include "avrlib/devices/external_eeprom.h"

#include "midialf/midialf.h"
#include "midialf/hardware_config.h"
#include "midialf/seq.h"

namespace midialf {
  
enum {
  kSlotSize = 256,
  kMaxNumSlots = 256,
  kSeqDataOffset = 64,  // Accomodates sequence info
};

// Currently SeqInfo is 53 bytes, so existing layout allows 11 bytes for SeqInfo extensions and
// 3 bytes for SeqData extensions. Note that 2 bytes at the end of the last memory block are 
// garbled by eeprom memory availability check!

class Storage {
 public:

  static void Init();

  static uint8_t ReadSlotName(uint8_t slot, uint8_t* name);
  static uint8_t WriteSlotName(uint8_t slot, const uint8_t* name);
   
  static uint8_t ReadSeqInfo(uint8_t slot, SeqInfo* info);
  static uint8_t WriteSeqInfo(uint8_t slot, const SeqInfo* info);
   
  static uint8_t ReadSeqData(uint8_t slot, SeqData* data, uint8_t size);
  static uint8_t WriteSeqData(uint8_t slot, const SeqData* data, uint8_t size);
   
  static uint32_t addressable_space_size() {
    return (uint32_t)num_accessible_banks_ * kBankSize;
  }

  static uint16_t num_slots() {
    return addressable_space_size() / kSlotSize;
  }

 private:
  static uint16_t WriteExternal(const uint8_t* data, uint16_t address, uint8_t size);
  static uint16_t ReadExternal(uint8_t* data, uint16_t address, uint8_t size);

  static uint8_t num_accessible_banks_;
};

extern Storage storage;

} // namespace midialf

#endif // MIDIALF_STORAGE_H_
