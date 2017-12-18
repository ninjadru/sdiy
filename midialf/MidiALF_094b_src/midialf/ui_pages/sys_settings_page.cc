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

#include "midialf/ui_pages/sys_settings_page.h"

namespace midialf {

/* static */
const prog_EventHandlers SysSettingsPage::event_handlers_ PROGMEM = {
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
void SysSettingsPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void SysSettingsPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t SysSettingsPage::OnIncrement(uint8_t id, int8_t value) {
  switch (id) {
    case ENCODER_1: UpdateClockMode(value); return 1;
    case ENCODER_2: UpdateSeqSwitchMode(value); return 1;
    case ENCODER_3: return 1;
#ifdef ENABLE_CV_OUTPUT
    case ENCODER_4: UpdateStrobeWidth(value); return 1;
#else
    case ENCODER_4: return 1;
#endif
    case ENCODER_5: UpdateProgramChange(value); return 1;
    case ENCODER_6: UpdateControlChange(value); return 1;
    case ENCODER_7: return 1;
    case ENCODER_8: return 1;
  }
  return 0;
}

/* static */
uint8_t SysSettingsPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  switch (id) {
    case ENCODER_1: seq.set_clock_mode(CLOCK_MODE_INTERNAL); return 1;
    case ENCODER_2: seq.set_seq_switch_mode(SEQ_SWITCH_MODE_IMMEDIATE); return 1;
    case ENCODER_3: return 1;
#ifdef ENABLE_CV_OUTPUT
    case ENCODER_4: seq.set_strobe_width(kDefStrobeWidth); return 1;
#else
    case ENCODER_4: return 1;
#endif
    case ENCODER_5: seq.set_prog_change_flags(PROGRAM_CHANGE_NONE); return 1;
    case ENCODER_6: seq.set_ctrl_change_flags(CONTROL_CHANGE_NONE); return 1;
    case ENCODER_7: return 1;
    case ENCODER_8: return 1;
  }
  return 0;
}

/* static */
uint8_t SysSettingsPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    switch (value) {
      case SWITCH_1: UpdateClockMode(0); return 1;
      case SWITCH_2: UpdateSeqSwitchMode(0); return 1;
      case SWITCH_3: return 1;
#ifdef ENABLE_CV_OUTPUT
      case SWITCH_4: UpdateStrobeWidth(0); return 1;
#else
      case SWITCH_4: return 1;
#endif
      case SWITCH_5: UpdateProgramChange(0); return 1;
      case SWITCH_6: UpdateControlChange(0); return 1;
      case SWITCH_7: return 1;
      case SWITCH_8: return 1;
    }
  }
  return 0;
}

/* static */
uint8_t SysSettingsPage::OnIdle() {
  return 0;
}

/* static */
void SysSettingsPage::UpdateScreen() {
  DrawSeparators();
#ifdef ENABLE_CV_OUTPUT
  DrawCells(0, PSTR(" ClkSeqX----StrbPrgCCtlC--------"));
#else
  DrawCells(0, PSTR(" ClkSeqX--------PrgCCtlC--------"));
#endif

  char* line2 = display.line_buffer(1);

  DrawSelStr(&line2[cell_pos(0)], seq.clock_mode(), PSTR(" int ext"));
  DrawSelStr(&line2[cell_pos(1)], seq.seq_switch_mode(), PSTR(" imm end"));
#ifdef ENABLE_CV_OUTPUT
  DrawSelStr(&line2[cell_pos(3)], seq.strobe_width(), PSTR(" 1ms 2ms 3ms 4ms 5ms"));
#endif
  DrawSelStr(&line2[cell_pos(4)], seq.prog_change_flags(), PSTR("nonerecvsendboth"));
  DrawSelStr(&line2[cell_pos(5)], seq.ctrl_change_flags(), PSTR("nonerecvsendboth"));
}

/* static */
void SysSettingsPage::UpdateLeds() {
}

///////////////////////////////////////////////////////////////////////////////
// Commmand handlers

/* static */
void SysSettingsPage::UpdateClockMode(int8_t value) {
  if (value) {
    seq.set_clock_mode(Clamp(static_cast<int16_t>(seq.clock_mode()) + value, CLOCK_MODE_INTERNAL, CLOCK_MODE_EXTERNAL));
  } else {
    uint8_t clock_mode = seq.clock_mode();
    if (++clock_mode > CLOCK_MODE_EXTERNAL) clock_mode = CLOCK_MODE_INTERNAL;
    seq.set_clock_mode(clock_mode);
  }
}

/* static */
void SysSettingsPage::UpdateSeqSwitchMode(int8_t value) {
  if (value) {
    seq.set_seq_switch_mode(Clamp(static_cast<int16_t>(seq.seq_switch_mode()) + value, SEQ_SWITCH_MODE_IMMEDIATE, SEQ_SWITCH_MODE_ONSEQEND));
  } else {
    uint8_t seq_switch_mode = seq.seq_switch_mode();
    if (++seq_switch_mode > SEQ_SWITCH_MODE_ONSEQEND) seq_switch_mode = SEQ_SWITCH_MODE_IMMEDIATE;
    seq.set_seq_switch_mode(seq_switch_mode);
  }
}

/* static */
void SysSettingsPage::UpdateStrobeWidth(int8_t value) {
  if (value) {
    seq.set_strobe_width(Clamp(static_cast<int16_t>(seq.strobe_width()) + value, kMinStrobeWidth, kMaxStrobeWidth));
  } else {
    uint8_t strobe_width = seq.strobe_width();
    if (++strobe_width > kMaxStrobeWidth) strobe_width = kMinStrobeWidth;
    seq.set_strobe_width(strobe_width);
  }
}

/* static */
void SysSettingsPage::UpdateProgramChange(int8_t value) {
  if (value) {
    seq.set_prog_change_flags(Clamp(static_cast<int16_t>(seq.prog_change_flags()) + value, PROGRAM_CHANGE_NONE, PROGRAM_CHANGE_BOTH));
  } else {
    uint8_t prog_change_flags = seq.prog_change_flags();
    if (++prog_change_flags > PROGRAM_CHANGE_BOTH) prog_change_flags = PROGRAM_CHANGE_NONE;
    seq.set_prog_change_flags(prog_change_flags);
  }
}

/* static */
void SysSettingsPage::UpdateControlChange(int8_t value) {
  if (value) {
    seq.set_ctrl_change_flags(Clamp(static_cast<int16_t>(seq.ctrl_change_flags()) + value, CONTROL_CHANGE_NONE, CONTROL_CHANGE_BOTH));
  } else {
    uint8_t ctrl_change_flags = seq.ctrl_change_flags();
    if (++ctrl_change_flags > CONTROL_CHANGE_BOTH) ctrl_change_flags = CONTROL_CHANGE_NONE;
    seq.set_ctrl_change_flags(ctrl_change_flags);
  }
}

} // namespace midialf
