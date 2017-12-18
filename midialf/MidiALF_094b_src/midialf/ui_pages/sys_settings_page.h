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
// System settings page class.

#ifndef MIDIALF_UI_PAGES_SYS_SETTINGS_PAGE_H_
#define MIDIALF_UI_PAGES_SYS_SETTINGS_PAGE_H_

#include "midialf/ui_pages/ui_page.h"

namespace midialf {

class SysSettingsPage : public UiPage {
 public:
  SysSettingsPage() { }
  
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
  static void UpdateClockMode(int8_t value);
  static void UpdateSeqSwitchMode(int8_t value);
  static void UpdateStrobeWidth(int8_t value);
  static void UpdateProgramChange(int8_t value);
  static void UpdateControlChange(int8_t value);

  DISALLOW_COPY_AND_ASSIGN(SysSettingsPage);
};

} // namespace midialf

#endif  // MIDIALF_UI_PAGES_SYS_SETTINGS_PAGE_H_
