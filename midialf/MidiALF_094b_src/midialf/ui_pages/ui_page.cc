// Copyright 2012 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// Based on Ambika code by Olivier Gillet (ol.gillet@gmail.com)
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
// Base UI page class.

#include "midialf/ui_pages/ui_page.h"

namespace midialf {

/* static */
const prog_EventHandlers UiPage::event_handlers_ PROGMEM = {
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
void UiPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void UiPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t UiPage::OnIncrement(uint8_t id, int8_t value) {
  return 0;
}

/* static */
uint8_t UiPage::OnClick(uint8_t id, uint8_t value) {
  return 0;
}

/* static */
uint8_t UiPage::OnSwitch(uint8_t id, uint8_t value) {
  return 0;
}

/* static */
uint8_t UiPage::OnIdle() {
  return 0;
}

/* static */
void UiPage::UpdateScreen() {
}

/* static */
void UiPage::UpdateLeds() {
}

///////////////////////////////////////////////////////////////////////////////
// Miscellaneous helpers

/* static */
uint8_t UiPage::cell_pos(uint8_t step)
{
  // 0000000000111111111122222222223333333333
  // 0123456789012345678901234567890123456789
  // ----|----|----|----||----|----|----|----

  static const prog_uint8_t step_cell_pos[kNumSteps] PROGMEM = {
    0, 5, 10, 15, 21, 26, 31, 36,
  };

  return pgm_read_byte(&step_cell_pos[step]);
}
  
/* static */
void UiPage::DrawSeparators() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  for (uint8_t n = 4; n < kLcdWidth; ) {
    line1[n] = '|'; 
    line2[n] = '|'; 
    if (n == 19) {
      n++;
    } else
      n+= 5;
  }
}

/* static */
void UiPage::DrawCells(uint8_t nLine, const prog_char* cells) {
  char* line = display.line_buffer(nLine);
  for (uint8_t n = 0; n < kNumSteps; n++) {
    uint8_t x = cell_pos(n);
    memcpy_P(&line[x], &cells[n << 2], 4);
  }
}

/* static */
void UiPage::DrawSelStr(char* buffer, uint8_t sel, const prog_char* strings) {
  memcpy_P(buffer, &strings[sel << 2], 4);
}

/* static */
uint8_t UiPage::Clamp(int16_t value, uint8_t min, uint8_t max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return static_cast<uint8_t>(value);
}

/* static */
int16_t UiPage::Clamp16(int16_t value, int16_t min, int16_t max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

/* static */
uint8_t UiPage::FindLastSpace(char* str, uint8_t count) {
  while (count > 0 && str[count - 1] == ' ') {
    count--;
  }
  return count;
}

/* static */
void UiPage::SendStep(uint8_t step) {
  if (!seq.running()) {
    seq.set_step(step);
    seq.set_manual_step_selected(1); // Should be called before SendStep() to ignore step mute and legato
    seq.SendStep(step);
  } else
  if (seq.step_hold()) {
    seq.set_step(step);
  }
}

} // namespace midialf
