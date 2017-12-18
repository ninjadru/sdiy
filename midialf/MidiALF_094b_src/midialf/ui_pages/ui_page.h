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

#ifndef MIDIALF_UI_PAGES_UI_PAGE_H_
#define MIDIALF_UI_PAGES_UI_PAGE_H_

#include "midialf/display.h"
#include "midialf/seq.h"
#include "midialf/ui.h"

#include "avrlib/string.h"

namespace midialf {

class UiPage {
 public:
  UiPage() { }

  static void OnInit(PageInfo* pageInfo, UiPageIndex prevPage);
  static void OnQuit(UiPageIndex nextPage);

  static uint8_t OnIncrement(uint8_t id, int8_t value);
  static uint8_t OnClick(uint8_t id, uint8_t value);
  static uint8_t OnSwitch(uint8_t id, uint8_t value);
  static uint8_t OnIdle();

  static void UpdateScreen();
  static void UpdateLeds();

  static const prog_EventHandlers event_handlers_;
  
 protected:

  static uint8_t cell_pos(uint8_t step);

  static void DrawSeparators();
  static void DrawCells(uint8_t nLine, const prog_char* cells);
  static void DrawSelStr(char* buffer, uint8_t sel, const prog_char* strings);

  static uint8_t Clamp(int16_t value, uint8_t min, uint8_t max);
  static uint8_t Clamp7F(int16_t value) { return Clamp(value, 0, 0x7f); }
  static int16_t Clamp16(int16_t value, int16_t min, int16_t max);

  static uint8_t GetDefSetting(uint8_t value, uint8_t def, uint8_t alt_def) {
    return value == def ? alt_def : def;
  }

  static uint8_t FindLastSpace(char* str, uint8_t count);

  static void SendStep(uint8_t step);

  DISALLOW_COPY_AND_ASSIGN(UiPage);
};

} // namespace midialf

#endif  // MIDIALF_UI_PAGES_UI_PAGE_H_
