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

#include "midialf/ui_pages/ext_settings_page.h"

#ifdef ENABLE_CV_OUTPUT
#include "midialf/cv/cv.h"
#include "midialf/cv/port.h"
#endif

namespace midialf {

/* static */
const prog_EventHandlers ExtSettingsPage::event_handlers_ PROGMEM = {
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
void ExtSettingsPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void ExtSettingsPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t ExtSettingsPage::OnIncrement(uint8_t id, int8_t value) {
  switch (id) {
    case ENCODER_1:
    case ENCODER_2:
    case ENCODER_3:
    case ENCODER_4: UpdateCvMode(id - ENCODER_1, value); return 1;
    case ENCODER_5: 
    case ENCODER_6: 
    case ENCODER_7: 
    case ENCODER_8: UpdateGateMode(id - ENCODER_5, value); return 1;
  }
  return 0;
}

/* static */
uint8_t ExtSettingsPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  switch (id) {
    case ENCODER_1: 
    case ENCODER_2: 
    case ENCODER_3: 
    case ENCODER_4: UpdateCvModeOffset(id - ENCODER_1, seq.cv_mode_offset(id - ENCODER_1) ? 0 : 1); return 1;
    case ENCODER_5: 
    case ENCODER_6: 
    case ENCODER_7: 
    case ENCODER_8: UpdateGateModeInvert(id - ENCODER_5, seq.gate_mode_invert(id - ENCODER_5) ? 0 : 1); return 1;
  }
  return 0;
}

/* static */
uint8_t ExtSettingsPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    switch (value) {
      case SWITCH_1: 
      case SWITCH_2: 
      case SWITCH_3: 
      case SWITCH_4: UpdateCvMode(value - SWITCH_1, 0); return 1;
      case SWITCH_5: 
      case SWITCH_6: 
      case SWITCH_7: 
      case SWITCH_8: UpdateGateMode(value - SWITCH_5, 0); return 1;
    }
  }
  return 0;
}

/* static */
uint8_t ExtSettingsPage::OnIdle() {
  return 0;
}

/* static */
void ExtSettingsPage::UpdateScreen() {
  DrawSeparators();
  DrawCells(0, PSTR(" CV1 CV2 CV3 CV4 Gt1 Gt2 Gt3 Gt4"));
  
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  for (uint8_t n = 0; n < 4; n++) {
    line1[cell_pos(n)] = seq.cv_mode_offset(n) ? '-' : ' ';
    line1[cell_pos(n + 4)] = seq.gate_mode_invert(n) ? '-' : ' ';
    DrawSelStr(&line2[cell_pos(n)], seq.cv_mode(n), PSTR("notevelo cc1 cc2lfo1lfo2atchpbnd"));
    DrawSelStr(&line2[cell_pos(n + 4)], seq.gate_mode(n), PSTR("gatestrbclckstrt seqlfo1lfo2"));
  }
}

/* static */
void ExtSettingsPage::UpdateLeds() {
}

///////////////////////////////////////////////////////////////////////////////
// Commmand handlers

/* static */
void ExtSettingsPage::UpdateCvMode(int8_t index, int8_t value) {
  if (value) {
    seq.set_cv_mode(index, Clamp(static_cast<int16_t>(seq.cv_mode(index)) + value, 0, CVMODE_MAX));
  } else {
    uint8_t cv_mode = seq.cv_mode(index);
    if (++cv_mode > CVMODE_MAX) cv_mode = 0;
    seq.set_cv_mode(index, cv_mode);
  }
}

void ExtSettingsPage::UpdateCvModeOffset(int8_t index, int8_t value) {
  if (value) {
    seq.set_cv_mode_offset(index, Clamp(static_cast<int16_t>(seq.cv_mode_offset(index)) + value, 0, 1));
  } else {
    uint8_t cv_mode_offset = seq.cv_mode_offset(index);
    if (++cv_mode_offset > 1) cv_mode_offset = 0;
    seq.set_cv_mode_offset(index, cv_mode_offset);
  }

#ifdef ENABLE_CV_OUTPUT
  port.UpdateCvOffset();
#endif
}

/* static */
void ExtSettingsPage::UpdateGateMode(int8_t index, int8_t value) {
  if (value) {
    seq.set_gate_mode(index, Clamp(static_cast<int16_t>(seq.gate_mode(index)) + value, 0, GATEMODE_MAX));
  } else {
    uint8_t gate_mode = seq.gate_mode(index);
    if (++gate_mode > GATEMODE_MAX) gate_mode = 0;
    seq.set_gate_mode(index, gate_mode);
  }
}

void ExtSettingsPage::UpdateGateModeInvert(int8_t index, int8_t value) {
  if (value) {
    seq.set_gate_mode_invert(index, Clamp(static_cast<int16_t>(seq.gate_mode_invert(index)) + value, 0, 1));
  } else {
    uint8_t gate_mode_invert = seq.gate_mode_invert(index);
    if (++gate_mode_invert > 1) gate_mode_invert = 0;
    seq.set_gate_mode_invert(index, gate_mode_invert);
  }

#ifdef ENABLE_CV_OUTPUT
  port.UpdateGateInvert();
#endif
}

} // namespace midialf
