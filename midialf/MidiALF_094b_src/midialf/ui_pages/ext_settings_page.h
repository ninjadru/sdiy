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
// CV extension settings page class.

#ifndef MIDIALF_UI_PAGES_EXT_SETTINGS_PAGE_H_
#define MIDIALF_UI_PAGES_EXT_SETTINGS_PAGE_H_

#include "midialf/ui_pages/ui_page.h"

namespace midialf {

class ExtSettingsPage : public UiPage {
 public:
  ExtSettingsPage() { }
  
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
  static void UpdateCvMode(int8_t index, int8_t value);
  static void UpdateCvModeOffset(int8_t index, int8_t value);
  static void UpdateGateMode(int8_t index, int8_t value);
  static void UpdateGateModeInvert(int8_t index, int8_t value);

  DISALLOW_COPY_AND_ASSIGN(ExtSettingsPage);
};

} // namespace midialf

#endif  // MIDIALF_UI_PAGES_EXT_SETTINGS_PAGE_H_
