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
// Load page class.

#include "midialf/ui_pages/load_page.h"
#include "midialf/ui_pages/save_page.h"

namespace midialf {

/* static */
uint8_t LoadPage::slot_;

/* static */
uint8_t LoadPage::name_[kNameLength];

/* static */
UiPageIndex LoadPage::prev_page_;

/* static */
const prog_EventHandlers LoadPage::event_handlers_ PROGMEM = {
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
void LoadPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  if (prevPage == PAGE_SAVE) prev_page_ = SavePage::prev_page(); else
  if (prevPage != PAGE_LOAD) prev_page_ = prevPage;
  SetSlot(seq.slot());
}

/* static */
void LoadPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t LoadPage::OnIncrement(uint8_t id, int8_t value) {
  // ENCB changes slot number
  if (id == ENCODER_B) {
    uint8_t slot = Clamp((int16_t)slot_ + value, 0, storage.num_slots() - 1);
    SetSlot(slot);
  }
  return 1;
}

/* static */
uint8_t LoadPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  // ENCA cancels mode
  if (id == ENCODER_A) {
    Ui::ShowPage(prev_page_);
  } else 
  // ENCB loads slot and cancels mode
  if (id == ENCODER_B) {
    LoadSlot();
    Ui::ShowPage(prev_page_);
  }
  return 1;
}

/* static */
uint8_t LoadPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    return 1;
  }
  return 0;
}

/* static */
uint8_t LoadPage::OnIdle() {
  return 0;
}

/* static */
void LoadPage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  static const prog_char cmdLoad[] PROGMEM = "[Load]";

  memcpy_P(&line1[0], PSTRN("Load program from:"));
  memcpy_P(&line1[kLcdWidth - lengof(cmdLoad)], cmdLoad, lengof(cmdLoad));

  Ui::PrintNumb(&line2[0], 1 + slot_);
  memcpy(&line2[4], name_, kNameLength);
}

/* static */
void LoadPage::UpdateLeds() {
}

/* static */
void LoadPage::SetSlot(uint8_t slot) {
  slot_ = slot;
  storage.ReadSlotName(slot, name_);
}

/* static */
void LoadPage::LoadSlot() {
  if (seq.prog_change_flags() & PROGRAM_CHANGE_SEND) {
    seq.Send3(0xb0 | seq.channel(), 0x20, slot_ >> 7);  // Bank Select LSB
    seq.Send3(0xb0 | seq.channel(), 0x00, 0);           // Bank Select MSB
    seq.Send2(0xc0 | seq.channel(), slot_ & 0x7f);      // Program Change
  }
  seq.LoadFromStorage(slot_);
}

} // namespace midialf
