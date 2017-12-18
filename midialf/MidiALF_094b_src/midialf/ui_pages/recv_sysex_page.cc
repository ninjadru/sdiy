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

#include "midialf/ui_pages/recv_sysex_page.h"

namespace midialf {

/* static */
UiPageIndex RecvSysExPage::prev_page_;

/* static */
const prog_EventHandlers RecvSysExPage::event_handlers_ PROGMEM = {
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
void RecvSysExPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  seq.Stop();
  prev_page_ = prevPage;
}

/* static */
void RecvSysExPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t RecvSysExPage::OnIncrement(uint8_t id, int8_t value) {
  Ui::ShowPage(prev_page_);
  return 1;
}

/* static */
uint8_t RecvSysExPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  Ui::ShowPage(prev_page_);
  return 1;
}

/* static */
uint8_t RecvSysExPage::OnSwitch(uint8_t id, uint8_t value) {
  Ui::ShowPage(prev_page_);
  return 1;
}

/* static */
uint8_t RecvSysExPage::OnIdle() {
  return 0;
}

/* static */
void RecvSysExPage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  memcpy_P(&line1[0], PSTRN("Received program:"));

  Ui::PrintNumb(&line2[0], 1 + seq.slot());
  seq.GetSeqName((uint8_t*)&line2[4]);
}

/* static */
void RecvSysExPage::UpdateLeds() {
}

} // namespace midialf
