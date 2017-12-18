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
// UI page for CC1/send editing

#include "midialf/ui_pages/cc1_page.h"

namespace midialf {

/* static */
const prog_EventHandlers CC1Page::event_handlers_ PROGMEM = {
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
void CC1Page::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void CC1Page::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t CC1Page::OnIncrement(uint8_t id, int8_t value) {
  if (id == ENCODER_A) {
    for (uint8_t n = 0; n < kNumSteps; n++) {
      uint8_t cc1 = seq.cc1(n);
      seq.set_cc1(n, Clamp7F(static_cast<int16_t>(cc1) + value));
    }
    return 1;
  }
  if (id >= ENCODER_1 && id <= ENCODER_8) {
    uint8_t step = id - ENCODER_1;
    uint8_t cc1 = seq.cc1(step);
    seq.set_cc1(step, Clamp7F(static_cast<int16_t>(cc1) + value));
    SendStep(step);
    return 1;
  }
  return 0;
}

/* static */
uint8_t CC1Page::OnClick(uint8_t id, uint8_t value) {
  return 0;
}

/* static */
uint8_t CC1Page::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH && value < kNumSteps) {
    uint8_t cc1send = seq.cc1send(value);
    seq.set_cc1send(value, !cc1send);
    return 1;
  }
  return 0;
}

/* static */
uint8_t CC1Page::OnIdle() {
  return 0;
}

/* static */
void CC1Page::UpdateScreen() {
  DrawSeparators();
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  uint8_t noCC1 = true;
  for (uint8_t n = 0; n < kNumSteps; n++) {
    uint8_t x = cell_pos(n);
    UnsafeItoa(seq.cc1(n), 3, &line1[x + 1]);
    if (seq.cc1send(n)) {
      memcpy_P(&line2[x], PSTRN(" cc1"));
      noCC1 = false;
    }
  }
  if (noCC1) {
    memcpy_P(&line2[kLcdWidth / 2 - 4], PSTRN("CC1 muted"));
  }
}

/* static */
void CC1Page::UpdateLeds() {
}

} // namespace midialf
