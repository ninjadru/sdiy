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
// Save page class.

#include "midialf/ui_pages/save_page.h"
#include "midialf/ui_pages/load_page.h"

namespace midialf {

/* static */
uint8_t SavePage::slot_;

/* static */
uint8_t SavePage::name_[kNameLength];

/* static */
uint8_t SavePage::edit_offset_;

/* static */
uint8_t SavePage::original_char_;

/* static */
UiPageIndex SavePage::prev_page_;

/* static */
const prog_EventHandlers SavePage::event_handlers_ PROGMEM = {
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
void SavePage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  if (prevPage == PAGE_LOAD) prev_page_ = LoadPage::prev_page(); else
  if (prevPage != PAGE_SAVE) prev_page_ = prevPage;
  SetSlot(seq.slot());
}

/* static */
void SavePage::OnQuit(UiPageIndex nextPage) {
  display.set_cursor_position(kLcdNoCursor);
}

/* static */
uint8_t SavePage::OnIncrement(uint8_t id, int8_t value) {
  // ENCA shifts name editing left/right
  if (id == ENCODER_A) {
    edit_offset_ = Clamp((int16_t)edit_offset_ + value, 0, kNameLength - 1);
  } else 
  // ENCB changes slot number
  if (id == ENCODER_B) {
    uint8_t slot = Clamp((int16_t)slot_ + value, 0, storage.num_slots() - 1);
    SetSlot(slot);
  } else {
    // Step ENCx increments/decrements character
    uint8_t minChar = kMinNameChar;
    uint8_t maxChar = kMaxNameChar;
    switch (id) {
DoIt: case ENCODER_1: 
        if (name_[edit_offset_] >= minChar && name_[edit_offset_] <= maxChar) {
          name_[edit_offset_] = Clamp((int16_t)name_[edit_offset_] + value, minChar, maxChar);
        } else
          name_[edit_offset_] = minChar;
        break;
      case ENCODER_2: minChar = 'A'; maxChar = 'Z'; goto DoIt;
      case ENCODER_3: minChar = 'a'; maxChar = 'z'; goto DoIt;
      case ENCODER_4: minChar = '0'; maxChar = '9'; goto DoIt;
    }
  }
  return 1;
}

/* static */
uint8_t SavePage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  // ENCA cancels mode
  if (id == ENCODER_A) {
    Ui::ShowPage(prev_page_);
  } else 
  // ENCB saves slot and cancels mode
  if (id == ENCODER_B) {
    SaveSlot();
    Ui::ShowPage(prev_page_);
  } else
  // ENC1 decrements offset
  if (id == ENCODER_1) {
    edit_offset_ = Clamp((int16_t)edit_offset_  - 1, 0, kNameLength - 1);
  } else
  // ENC2 increments offset
  if (id == ENCODER_2) {
    edit_offset_ = Clamp((int16_t)edit_offset_  + 1, 0, kNameLength - 1);
  } else
  // ENCB accepts sequence name
  if (id == ENCODER_8) {
    seq.GetSeqName(name_);
  }
  return 1;
}

/* static */
uint8_t SavePage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    uint8_t chr;
    switch (value) {
      case SWITCH_1: // Toggle space/original char
        { chr = ' ';
DoIt:     if (name_[edit_offset_] != chr) {
            original_char_ = name_[edit_offset_];
            name_[edit_offset_] = chr;
          } else {
            name_[edit_offset_] = original_char_;
          }
        }
        break;
      case SWITCH_2: // Toggle previous/original char
        if (edit_offset_ > 0) {
          chr = name_[edit_offset_ - 1];
          goto DoIt;
        }
        break;
      case SWITCH_3: // Toggle case
        if (name_[edit_offset_] >= 'A' && name_[edit_offset_] <= 'Z') {
          name_[edit_offset_]+= 'a' - 'A';
        } else
        if (name_[edit_offset_] >= 'a' && name_[edit_offset_] <= 'z') {
          name_[edit_offset_]-= 'a' - 'A';
        }
        break;
      case SWITCH_7: // Insert at cursor
        { uint8_t cch = kNameLength - edit_offset_ - 1;
          memmove(&name_[edit_offset_ + 1], &name_[edit_offset_], cch);
          name_[edit_offset_] = ' ';
        }
        break;
      case SWITCH_8: // Delete at cursor
        { uint8_t cch = kNameLength - edit_offset_ - 1;
          memmove(&name_[edit_offset_], &name_[edit_offset_ + 1], cch);
          name_[kNameLength - 1] = ' ';
        }
        break;
    }
    return 1;
  }
  return 0;
}

/* static */
uint8_t SavePage::OnIdle() {
  return 0;
}

/* static */
void SavePage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  static const prog_char prompt[] PROGMEM = "Save program '";
  static const prog_char prompt_end[] PROGMEM = "to:";
  static const prog_char cmdSave[] PROGMEM = "[Save]";
  static const prog_char cmdName[] PROGMEM = "[Name]";

  memcpy_P(&line1[0], prompt, lengof(prompt));
  seq.GetSeqName((uint8_t*)&line1[lengof(prompt)]);
  uint8_t x = FindLastSpace(&line1[lengof(prompt)], kNameLength);
  line1[lengof(prompt) + x] = '\''; x++;
  if (lengof(prompt) + x + lengof(prompt_end) < kLcdWidth - lengof(cmdSave)) x++;
  memcpy_P(&line1[lengof(prompt) + x], PSTRN("to:"));
  memcpy_P(&line1[kLcdWidth - lengof(cmdSave)], cmdSave, lengof(cmdSave));

  Ui::PrintNumb(&line2[0], 1 + slot_);
  memcpy(&line2[4], name_, kNameLength);
  memcpy_P(&line2[kLcdWidth - lengof(cmdName)], cmdName, lengof(cmdName));

  display.set_cursor_position(kLcdWidth + 4 + edit_offset_);
}

/* static */
void SavePage::UpdateLeds() {
}

/* static */
void SavePage::SetSlot(uint8_t slot) {
  slot_ = slot;
  storage.ReadSlotName(slot, name_);
  edit_offset_ = 0;
}

/* static */
void SavePage::SaveSlot() {
  seq.SetSeqName(name_);
  seq.SaveToStorage(slot_);
}

} // namespace midialf
