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
// Copy sequence page class.

#include "midialf/ui_pages/copy_seq_page.h"
#include "midialf/leds.h"

namespace midialf {

/* static */
uint32_t CopySeqPage::started_time_;

/* static */
const prog_EventHandlers CopySeqPage::event_handlers_ PROGMEM = {
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
void CopySeqPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  started_time_ = milliseconds();
}

/* static */
void CopySeqPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t CopySeqPage::OnIncrement(uint8_t id, int8_t value) {
  // Cancel mode on any encoder increment
  Ui::ShowLastPage();
  return 1;
}

/* static */
uint8_t CopySeqPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  // Cancel mode on any encoder click
  Ui::ShowLastPage();
  return 1;
}

/* static */
uint8_t CopySeqPage::OnSwitch(uint8_t id, uint8_t value) {
  // Copy sequence on SeqX switch
  if (id == SIDE_SWITCH && value >= SIDE_SWITCH_SEQA && value <= SIDE_SWITCH_SEQD) {
    uint8_t targetSeq = value - SIDE_SWITCH_SEQA;
    seq.CopyTo(targetSeq);
    seq.set_seq(targetSeq);
  }
  // Cancel mode on any switch
  Ui::ShowLastPage();
  return 1;
}

/* static */
uint8_t CopySeqPage::OnIdle() {
  return 0;
}

/* static */
void CopySeqPage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  memcpy_P(&line1[0], PSTRN("Select sequence to copy SeqX to..."));
  line1[27] = 'A' + seq.seq();
}

/* static */
void CopySeqPage::UpdateLeds() {
  uint32_t now = milliseconds();
  if ((now - started_time_) % 200 > 100) {
    leds.set_pixel(LED_SEQA);
    leds.set_pixel(LED_SEQB);
    leds.set_pixel(LED_SEQC);
    leds.set_pixel(LED_SEQD);
  }
}

} // namespace midialf
