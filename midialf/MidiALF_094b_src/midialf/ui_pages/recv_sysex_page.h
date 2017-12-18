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
// Recv SysEx page class.

#ifndef MIDIALF_UI_PAGES_RECV_SYSEX_PAGE_H_
#define MIDIALF_UI_PAGES_RECV_SYSEX_PAGE_H_

#include "midialf/ui_pages/ui_page.h"
#include "midialf/seq.h"

namespace midialf {

class RecvSysExPage : public UiPage {
 public:
  RecvSysExPage() { }
  
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
  static UiPageIndex prev_page_;

  DISALLOW_COPY_AND_ASSIGN(RecvSysExPage);
};

} // namespace midialf

#endif  // MIDIALF_UI_PAGES_RECV_SYSEX_PAGE_H_
