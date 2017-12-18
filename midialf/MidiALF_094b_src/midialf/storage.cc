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

#include "midialf/storage.h"

using namespace avrlib;

namespace midialf {

ExternalEeprom<kMaxNumBanks * kBankSize, I2cMaster<8, 64, 400000> > external_eeprom;

/* extern */
Storage storage;

/* static */
uint8_t Storage::num_accessible_banks_;

/* static */
void Storage::Init() {
  uint16_t data;
  external_eeprom.Init();
  uint16_t address = kMaxNumBanks * kBankSize - 2;
  // Write the sequence 0xfadN at the end of each bank, where N is the
  // bank number.
  for (uint8_t i = kMaxNumBanks; i > 0; --i) {
    data = 0xfad0 + i;
    WriteExternal((uint8_t*)&data, address, 2);
    address -= kBankSize;
  }
  // Try to read back this data to figure out the actual number of banks.
  address = kMaxNumBanks * kBankSize - 2;
  num_accessible_banks_ = kMaxNumBanks;
  while (num_accessible_banks_ > 0) {
    ReadExternal((uint8_t*)&data, address, 2);
    if (data == 0xfad0 + num_accessible_banks_) {
      break;
    }
    --num_accessible_banks_;
    address -= kBankSize;
  }
#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("Storage::Init: num_accessible_banks=%u\n", num_accessible_banks_);
#endif
}

/* static */
uint16_t Storage::WriteExternal(const uint8_t* data, uint16_t address, uint8_t size) {
  return external_eeprom.Write(address, data, size);
}

/* static */
uint16_t Storage::ReadExternal(uint8_t* data, uint16_t address, uint8_t size) {
  return external_eeprom.Read(address, size, data);
}

/* static */
uint8_t Storage::ReadSlotName(uint8_t slot, uint8_t* name) {
  uint16_t address = kSlotSize * slot;
  if (address + kSlotSize > addressable_space_size())
    return 0;

  uint16_t read = ReadExternal(name, address, kNameLength);
  if (read != kNameLength)
    return 0;

  Seq::FixSeqName(name);

  return read;
}

/* static */
uint8_t Storage::WriteSlotName(uint8_t slot, const uint8_t* name) {
  uint16_t address = kSlotSize * slot;
  if (address + kSlotSize > addressable_space_size())
    return 0;

  uint16_t written = WriteExternal(name, address, kNameLength);
  if (written != kNameLength)
    return 0;

  return written;
}

/* static */
uint8_t Storage::ReadSeqInfo(uint8_t slot, SeqInfo* info) {
  uint16_t address = kSlotSize * slot;
  if (address + kSlotSize > addressable_space_size())
    return 0;

  uint16_t read = ReadExternal((uint8_t*)info, address, sizeof(SeqInfo));
  if (read != sizeof(SeqInfo))
    return 0;

  Seq::FixSeqName(info->name_);

  return read;
}

/* static */
uint8_t Storage::WriteSeqInfo(uint8_t slot, const SeqInfo* info) {
  uint16_t address = kSlotSize * slot;
  if (address + kSlotSize > addressable_space_size())
    return 0;

  // This takes about 7ms
  uint16_t written = WriteExternal((uint8_t*)info, address, sizeof(SeqInfo));
  if (written != sizeof(SeqInfo))
    return 0;

  return written;
}
 
/* static */
uint8_t Storage::ReadSeqData(uint8_t slot, SeqData* data, uint8_t size) {
  uint16_t address = kSlotSize * slot;
  if (address + kSlotSize > addressable_space_size())
    return 0;

  uint16_t read = ReadExternal((uint8_t*)data, address + kSeqDataOffset, size);
  if (read != size)
    return 0;

  return read;
}

/* static */
uint8_t Storage::WriteSeqData(uint8_t slot, const SeqData* data, uint8_t size) {
  uint16_t address = kSlotSize * slot;
  if (address + kSlotSize > addressable_space_size())
    return 0;

  uint16_t written = WriteExternal((uint8_t*)data, address + kSeqDataOffset, size);
  if (written != size)
    return 0;

  return written;
}
 
}  // midialf
