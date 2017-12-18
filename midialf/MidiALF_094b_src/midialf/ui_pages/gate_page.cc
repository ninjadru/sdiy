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
// UI page for gate/legato editing

#include "midialf/ui_pages/gate_page.h"
#include "midialf/note_duration.h"

namespace midialf {

/* static */
const prog_EventHandlers GatePage::event_handlers_ PROGMEM = {
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
void GatePage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void GatePage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t GatePage::OnIncrement(uint8_t id, int8_t value) {
  if (id == ENCODER_A) {
    for (uint8_t n = 0; n < kNumSteps; n++) {
      uint8_t gate = seq.gate(n);
      seq.set_gate(n, Clamp(static_cast<int16_t>(gate) + value, 0, kNoteDurationCount - 1));
    }
    return 1;
  }
  if (id >= ENCODER_1 && id <= ENCODER_8) {
    uint8_t step = id - ENCODER_1;
    uint8_t gate = seq.gate(step);
    seq.set_gate(step, Clamp(static_cast<int16_t>(gate) + value, 0, kNoteDurationCount - 1));
    SendStep(step);
    return 1;
  }
  return 0;
}

/* static */
uint8_t GatePage::OnClick(uint8_t id, uint8_t value) {
  return 0;
}

/* static */
uint8_t GatePage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH && value < kNumSteps) {
    uint8_t lega = seq.lega(value);
    seq.set_lega(value, !lega);
    return 1;
  }
  return 0;
}

/* static */
uint8_t GatePage::OnIdle() {
  return 0;
}

/* static */
void GatePage::UpdateScreen() {
  DrawSeparators();
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  for (uint8_t n = 0; n < kNumSteps; n++) {
    uint8_t x = cell_pos(n);
    DrawSelStr(&line1[x], seq.gate(n), midi_clock_ticks_per_note_str);
    if (seq.lega(n)) {
      memcpy_P(&line2[x], PSTRN("lega"));
    }
  }
}

/* static */
void GatePage::UpdateLeds() {
}

} // namespace midialf
