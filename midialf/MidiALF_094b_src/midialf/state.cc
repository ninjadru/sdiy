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

#include "midialf/state.h"
#include "midialf/seq.h"

#include <avr/eeprom.h>

namespace midialf {

State state;

struct StateData {
  static const uint16_t kMagicWord = 0xbad0;
  uint16_t magic_;
  uint16_t crc16_;
  uint8_t seqSave_[Seq::kSeqSaveSize];
  uint8_t prog_change_flags_;
  uint8_t ctrl_change_flags_;
  uint8_t strobe_width_;
};

StateData EEMEM stateData;

/* <static> */
uint16_t State::crc16_;
/* </static> */

/* static */
void State::Save() {
  uint16_t crc16 = CalcCrc16();
  if (crc16_ == crc16)
    return;

#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("State::Save: crc16=%04x size=%d\n", crc16, sizeof(StateData));
#endif

  crc16_ = crc16;

  // This takes 35ms
  eeprom_update_word(&stateData.magic_, stateData.kMagicWord);
  eeprom_update_word(&stateData.crc16_, crc16);
  seq.SaveToIntEeprom(&stateData.seqSave_[0]);
  eeprom_update_byte(&stateData.prog_change_flags_, seq.prog_change_flags());
  eeprom_update_byte(&stateData.ctrl_change_flags_, seq.ctrl_change_flags());
  eeprom_update_byte(&stateData.strobe_width_, seq.strobe_width());
}

/* static */
void State::Load() {
  uint16_t magic = eeprom_read_word(&stateData.magic_);
  if (magic != stateData.kMagicWord)
    return;

  crc16_ = eeprom_read_word(&stateData.crc16_);
  seq.LoadFromIntEeprom(&stateData.seqSave_[0]);

  seq.set_prog_change_flags(
    seq.Verify(eeprom_read_byte(&stateData.prog_change_flags_), 
      PROGRAM_CHANGE_NONE, PROGRAM_CHANGE_BOTH, PROGRAM_CHANGE_NONE));

  seq.set_ctrl_change_flags(
    seq.Verify(eeprom_read_byte(&stateData.ctrl_change_flags_), 
      CONTROL_CHANGE_NONE, CONTROL_CHANGE_BOTH, CONTROL_CHANGE_NONE));

  seq.set_strobe_width(
    seq.Verify(eeprom_read_byte(&stateData.strobe_width_), 
      kMinStrobeWidth, kMaxStrobeWidth, kDefStrobeWidth));

#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("State::Load: crc16=%04x size=%d\n", crc16_, sizeof(StateData));
#endif
}

/* static */
uint16_t State::CalcCrc16() {
  uint16_t crc16 = seq.CalcCrc16();
  return crc16;
}

} // namespace midialf
