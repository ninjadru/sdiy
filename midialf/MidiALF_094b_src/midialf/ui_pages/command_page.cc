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
// Command page class.

#include "midialf/ui_pages/command_page.h"

namespace midialf {

/* static */
const prog_EventHandlers CommandPage::event_handlers_ PROGMEM = {
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
void CommandPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void CommandPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t CommandPage::OnIncrement(uint8_t id, int8_t value) {
  // Cancel mode on any encoder increment
  Ui::ShowLastPage();
  return 1;
}

/* static */
uint8_t CommandPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  // Execute commands
  switch (id) {
    case ENCODER_1: Ui::ShowPage(PAGE_RANDOMIZE); return 1;
    case ENCODER_2: Ui::ShowPage(PAGE_ROTATE); return 1;
    case ENCODER_3: return 1;
#ifdef ENABLE_CV_OUTPUT
    case ENCODER_4: Ui::ShowPage(PAGE_TUNE); return 1;
#else
    case ENCODER_4: return 1;
#endif
    case ENCODER_5: Ui::ShowPage(PAGE_INIT); return 1;
    case ENCODER_6: Ui::ShowPage(PAGE_LOAD); return 1;
    case ENCODER_7: Ui::ShowPage(PAGE_SAVE); return 1;
    case ENCODER_8: Ui::ShowPage(PAGE_SEND_SYSEX); return 1;
  }
  // Cancel mode on any other encoder click
  Ui::ShowLastPage();
  return 1;
}

/* static */
uint8_t CommandPage::OnSwitch(uint8_t id, uint8_t value) {
  // Cancel mode on any switch
  Ui::ShowLastPage();
  return 1;
}

/* static */
uint8_t CommandPage::OnIdle() {
  return 0;
}

/* static */
void CommandPage::UpdateScreen() {
  DrawSeparators();
#ifdef ENABLE_CV_OUTPUT
  DrawCells(0, PSTR("RandRota----TuneInitLoadSaveSysX"));
#else
  DrawCells(0, PSTR("RandRota--------InitLoadSaveSysX"));
#endif
}

/* static */
void CommandPage::UpdateLeds() {
}

} // namespace midialf
