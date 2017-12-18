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
// UI page for velocity/skip editing

#include "midialf/ui_pages/velo_page.h"

namespace midialf {

/* static */
const prog_EventHandlers VeloPage::event_handlers_ PROGMEM = {
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
void VeloPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void VeloPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t VeloPage::OnIncrement(uint8_t id, int8_t value) {
  if (id == ENCODER_A) {
    for (uint8_t n = 0; n < kNumSteps; n++) {
      uint8_t velo = seq.velo(n);
      seq.set_velo(n, Clamp7F(static_cast<int16_t>(velo) + value));
    }
    return 1;
  }
  if (id >= ENCODER_1 && id <= ENCODER_8) {
    uint8_t step = id - ENCODER_1;
    uint8_t velo = seq.velo(step);
    seq.set_velo(step, Clamp7F(static_cast<int16_t>(velo) + value));
    SendStep(step);
    return 1;
  }
  return 0;
}

/* static */
uint8_t VeloPage::OnClick(uint8_t id, uint8_t value) {
  return 0;
}

/* static */
uint8_t VeloPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH && value < kNumSteps) {
    uint8_t skip = seq.skip(value);
    seq.set_skip(value, !skip);
    return 1;
  }
  return 0;
}

/* static */
uint8_t VeloPage::OnIdle() {
  return 0;
}

/* static */
void VeloPage::UpdateScreen() {
  DrawSeparators();
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  for (uint8_t n = 0; n < kNumSteps; n++) {
    uint8_t x = cell_pos(n);
    UnsafeItoa(seq.velo(n), 3, &line1[x + 1]);
    if (seq.skip(n)) {
      memcpy_P(&line2[x], PSTRN("skip"));
    }
  }
}

/* static */
void VeloPage::UpdateLeds() {
}

} // namespace midialf
