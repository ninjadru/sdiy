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
// Initialization page class.

#include "midialf/ui_pages/init_page.h"
#include "midialf/storage.h"

namespace midialf {

/* static */
uint8_t InitPage::slot_;

/* static */
uint8_t InitPage::name_[kNameLength];

/* static */
const prog_EventHandlers InitPage::event_handlers_ PROGMEM = {
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
void InitPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  SetSlot(seq.slot());
}

/* static */
void InitPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t InitPage::OnIncrement(uint8_t id, int8_t value) {
  // ENCB changes slot number
  if (id == ENCODER_B) {
    uint8_t slot = Clamp((int16_t)slot_ + value, 0, storage.num_slots() - 1);
    SetSlot(slot);
  }
  return 1;
}

/* static */
uint8_t InitPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  switch (id) {
    case ENCODER_A: break;               // ENCA cancels mode
    case ENCODER_B: InitSlot(); break;   // ENCB inits selected program slot
    case ENCODER_6: InitSeq(); break;    // ENC6 inits current sequence
    case ENCODER_7: InitPgm(); break;    // ENC7 inits current program
    case ENCODER_8: InitAll(); break;    // ENC8 inits all program slots
  }
  Ui::ShowLastPage();
  return 1;
}

/* static */
uint8_t InitPage::OnSwitch(uint8_t id, uint8_t value) {
  // Execute commands
  if (id == SWITCH) {
    return 1;
  }
  return 0;
}

/* static */
uint8_t InitPage::OnIdle() {
  return 0;
}

/* static */
void InitPage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  static const prog_char cmdInit[] PROGMEM = "[Init]";
  static const prog_char cmds[] PROGMEM = "[Seq] [Pgm] [All]";

  memcpy_P(&line1[0], PSTRN("Initialize:"));
  memcpy_P(&line1[kLcdWidth - lengof(cmdInit)], cmdInit, lengof(cmdInit));

  Ui::PrintNumb(&line2[0], 1 + slot_);
  memcpy(&line2[4], name_, kNameLength);
  
  memcpy_P(&line2[kLcdWidth - lengof(cmds)], cmds, lengof(cmds));
}

/* static */
void InitPage::UpdateLeds() {
}

/* static */
void InitPage::SetSlot(uint8_t slot) {
  slot_ = slot;
  storage.ReadSlotName(slot, name_);
}

/* static */
void InitPage::InitSlot() {
  seq.set_slot(slot_);
  InitPgm();
  seq.SaveToStorage(slot_);
}

/* static */
void InitPage::InitSeq() {
  seq.InitSeq();
}

/* static */
void InitPage::InitPgm() {
  seq.InitSeqInfo();
  for (uint8_t n = 0; n < 4; n++) {
    seq.InitSeq(n);
  }
}

/* static */
void InitPage::InitAll() {
  // Confirm full initialization
  Dialog dialog(PSTR("This will delete all your programs!\nAre you sure?"), PSTR("[Yes] [No]"), 2);
  if (dialog.result() != 1)
    return;

  // Draw local page
  Ui::Clear();

  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  
  static const prog_char cmdCancel[] PROGMEM = "[Cancel]";

  memcpy_P(&line1[0], PSTRN("Initializing program:"));
  memcpy_P(&line1[kLcdWidth - lengof(cmdCancel)], cmdCancel, lengof(cmdCancel));

  seq.set_slot(0); InitPgm();

  for (uint16_t n = 0; n < storage.num_slots(); n++) {
    seq.set_slot(n);
    // Update display
    uint8_t name[kNameLength]; storage.ReadSlotName(n, name);
    Ui::PrintNumb(&line2[0], 1 + n);
    memcpy(&line2[4], name, kNameLength);
    Ui::RedrawScreen();

    // Check if cancelled
    if (Ui::GetEncoderState(ENCODER_B) == 0)
      return;

    // Overwrite program
    seq.SaveToStorage(n);
  }

  seq.LoadFromStorage(0);
}

} // namespace midialf
