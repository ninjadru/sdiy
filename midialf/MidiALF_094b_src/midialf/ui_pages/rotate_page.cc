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
// Rotate sequence page.

#include "midialf/ui_pages/rotate_page.h"

namespace midialf {

/* static */
const prog_EventHandlers RotatePage::event_handlers_ PROGMEM = {
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
void RotatePage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void RotatePage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t RotatePage::OnIncrement(uint8_t id, int8_t value) {
  // ENCA moves alls notes up/down
  if (id == ENCODER_A) {
    value = Ui::FixOctaveIncrement(value);
    for (uint8_t n = 0; n < kNumSteps; n++) {
      uint8_t note = seq.note(n);
      seq.set_note(n, Clamp7F(static_cast<int16_t>(note) + value));
    }
    return 1;
  }
  // ENCB rotates sequence left/right
  if (id == ENCODER_B) {
    if (value < 0) {
      seq.RotLSeq();
    } else 
    if (value > 0) {
      seq.RotRSeq();
    }
    return 1;
  }
  // ENCx moves note up/down
  if (id >= ENCODER_1 && id <= ENCODER_8) {
    uint8_t step = id - ENCODER_1;
    uint8_t note = seq.note(step);
    value = Ui::FixOctaveIncrement(value);
    seq.set_note(step, Clamp7F(static_cast<int16_t>(note) + value));
    return 1;
  }
  return 0;
}

/* static */
uint8_t RotatePage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  // ENCA cancels mode
  if (id == ENCODER_A) {
    Ui::ShowLastPage();
    return 1;
  }
  return 0;
}

/* static */
uint8_t RotatePage::OnSwitch(uint8_t id, uint8_t value) {
  // Step switch makes the clicked step first
  if (id == SWITCH && value < kNumSteps) {
    seq.SetFirstStep(value);
    return 1;
  }
  return 0;
}

/* static */
uint8_t RotatePage::OnIdle() {
  return 0;
}

/* static */
void RotatePage::UpdateScreen() {
  DrawSeparators();
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  for (uint8_t n = 0; n < kNumSteps; n++) {
    uint8_t x = cell_pos(n);
    Ui::PrintNote(&line1[x], seq.note(n));
  }
  memcpy_P(&line2[0], PSTRN("Press switch to make it the first step"));
}

/* static */
void RotatePage::UpdateLeds() {
}

} // namespace midialf
