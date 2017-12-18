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
// Sequence settings page class.

#ifndef MIDIALF_UI_PAGES_SEQ_SETTINGS_PAGE_H_
#define MIDIALF_UI_PAGES_SEQ_SETTINGS_PAGE_H_

#include "midialf/ui_pages/ui_page.h"

namespace midialf {

class SeqSettingsPage : public UiPage {
 public:
  SeqSettingsPage() { }
  
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
  static uint8_t num_taps_;
  static uint32_t elapsed_time_;

  static void UpdateBpm(int8_t value);
  static void UpdateTapBpm();
  static void UpdateClockRate(int8_t value);
  static void UpdateGrooveTemplate(int8_t value);
  static void UpdateGrooveAmount(int8_t value);
  static void UpdateRootNote(int8_t value);
  static void UpdateCC1(int8_t value);
  static void UpdateCC2(int8_t value);
  static void UpdateChannel(int8_t value);

  DISALLOW_COPY_AND_ASSIGN(SeqSettingsPage);
};

} // namespace midialf

#endif  // MIDIALF_UI_PAGES_SEQ_SETTINGS_PAGE_H_
