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
// UI page for note cv tuning

#ifdef ENABLE_CV_OUTPUT

#include "midialf/ui_pages/tune_page.h"
#include "midialf/cv/cv.h"

namespace midialf {

/* static */
uint8_t TunePage::note_ = 60; // C4
uint8_t TunePage::sent_note_ = -1;

/* static */
const prog_EventHandlers TunePage::event_handlers_ PROGMEM = {
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
void TunePage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  sent_note_ = -1;
}

/* static */
void TunePage::OnQuit(UiPageIndex nextPage) {
  SendNoteOff();
  cv.SaveTune();
}

/* static */
uint8_t TunePage::OnIncrement(uint8_t id, int8_t value) {
  // ENCA shifts alls tuning up/down
  if (id == ENCODER_A) {
    AdjustTune(value);
    return 1;
  }
  // ENCB shifts alls notes up/down
  if (id == ENCODER_B) {
    value = Ui::FixOctaveIncrement(value);
    note_ = Clamp(static_cast<int16_t>(note_) + value, 0, 0x80 - kNumSteps);
    return 1;
  }
  // ENCx changes note tuning up/down
  if (id >= ENCODER_1 && id <= ENCODER_8) {
    uint8_t note = note_ + (id - ENCODER_1);
    cv.AdjustTune(note, value);
    SendNote(note);
    return 1;
  }
  return 0;
}

/* static */
uint8_t TunePage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  // Clear tuning
  if (id == ENCODER_A) {
    ClearTune();
    return 1;
  }
  // Save tuning
  if (id == ENCODER_B) {
    SendNoteOff();
    cv.SaveTune();
    return 1;
  }
  // Send clicked note
  if (id >= ENCODER_1 && id <= ENCODER_8) {
    SendNote(note_ + (id - ENCODER_1));
    return 1;
  }
  return 0;
}

/* static */
uint8_t TunePage::OnSwitch(uint8_t id, uint8_t value) {
  // Reset/Interpolate clicked note tuning
  if (id == SWITCH && value < kNumSteps) {
    uint8_t note = note_ + value;
    int16_t tune = cv.tune(note);
    if (tune) {
      cv.set_tune(note, 0);
    } else
      cv.InterpolateTune(note);
    SendNote(note);
    return 1;
  }
  return 0;
}

/* static */
uint8_t TunePage::OnIdle() {
  return 0;
}

/* static */
void TunePage::UpdateScreen() {
  DrawSeparators();
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  for (uint8_t n = 0; n < kNumSteps; n++) {
    uint8_t x = cell_pos(n);
    Ui::PrintNote(&line1[x], note_ + n);
    char numb[6] = { 0 };
    UnsafeItoa(cv.tune(note_ + n), 5, numb);
    uint8_t cch = strlen(numb);
    memcpy(&line2[x + 4 - cch], numb, cch);
  }
}

/* static */
void TunePage::UpdateLeds() {
}

/* static */
void TunePage::SendNote(int8_t note) {
  SendNoteOff();
  note = Clip(static_cast<int16_t>(note), 0, 0x7f);
  seq.SendNote(note, 100);
  sent_note_ = note;
}

/* static */
void TunePage::SendNoteOff() {
  if (sent_note_ != -1) {
    seq.SendNoteOff(sent_note_, 0);
    sent_note_ = -1;
  }
}

/* static */
void TunePage::AdjustTune(int8_t value) {
  // Move all tuning up/down
  for (uint8_t n = 0; n <= 0x7f; n++) {
    cv.AdjustTune(n, value);
  }
  // Resend sent note
  if (sent_note_ != -1) {
    SendNote(sent_note_);
  }
}

/* static */
void TunePage::ClearTune() {
  // Confirm clear tune
  Dialog dialog(PSTR("This will reset all tuning!\nAre you sure?"), PSTR("[Yes] [No]"), 2);
  if (dialog.result() != 1)
    return;

  cv.ClearTune();
}

} // namespace midialf

#endif // #ifdef ENABLE_CV_OUTPUT
