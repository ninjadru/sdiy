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
// Send SysEx page class.

#include "midialf/ui_pages/send_sysex_page.h"
#include "midialf/sysex_handler.h"

namespace midialf {

/* static */
uint8_t SendSysExPage::slot_;

/* static */
uint8_t SendSysExPage::name_[kNameLength];

/* static */
const prog_EventHandlers SendSysExPage::event_handlers_ PROGMEM = {
  OnInit,
  OnQuit,
  OnIncrement,
  OnClick,
  OnSwitch,
  OnIdle,
  UpdateScreen,
  UpdateLeds,
};

/* static */
void SendSysExPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  seq.Stop();
  SetSlot(seq.slot());
}

/* static */
void SendSysExPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t SendSysExPage::OnIncrement(uint8_t id, int8_t value) {
  // ENCB changes slot number
  if (id == ENCODER_B) {
    uint8_t slot = Clamp((int16_t)slot_ + value, 0, storage.num_slots() - 1);
    SetSlot(slot);
  }
  return 1;
}

/* static */
uint8_t SendSysExPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  switch (id) {
    case ENCODER_A: Ui::ShowLastPage(); break;  // ENCA cancels mode
    case ENCODER_B: SendSlot(); break;          // ENCB sends selected program slot
    case ENCODER_6: SendSeq(); break;           // ENC6 sends current sequence
    case ENCODER_7: SendPgm(); break;           // ENC7 sends current program
    case ENCODER_8: SendAll(); break;           // ENC8 sends all program slots
  }
  
  return 1;
}

/* static */
uint8_t SendSysExPage::OnSwitch(uint8_t id, uint8_t value) {
  // Execute commands
  if (id == SWITCH) {
    return 1;
  }
  return 0;
}

/* static */
uint8_t SendSysExPage::OnIdle() {
  return 0;
}

/* static */
void SendSysExPage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  static const prog_char cmdSend[] PROGMEM = "[Send]";
  static const prog_char cmds[] PROGMEM = "[Seq] [Pgm] [All]";

  memcpy_P(&line1[0], PSTRN("Send sysex with:"));
  memcpy_P(&line1[kLcdWidth - lengof(cmdSend)], cmdSend, lengof(cmdSend));

  Ui::PrintNumb(&line2[0], 1 + slot_);
  memcpy(&line2[4], name_, kNameLength);
  
  memcpy_P(&line2[kLcdWidth - lengof(cmds)], cmds, lengof(cmds));
}

/* static */
void SendSysExPage::UpdateLeds() {
}

/* static */
void SendSysExPage::SetSlot(uint8_t slot) {
  slot_ = slot;
  storage.ReadSlotName(slot, name_);
}

/* static */
void SendSysExPage::SendSlot() {
  sysex_handler.SendPgm(slot_);
}

/* static */
void SendSysExPage::SendSeq() {
  sysex_handler.SendSeq();
}

/* static */
void SendSysExPage::SendPgm() {
  sysex_handler.SendPgm();
}

/* static */
void SendSysExPage::SendAll() {
  sysex_handler.SendAll();
}

} // namespace midialf
