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
// Progression settings page class.

#include "midialf/ui_pages/prg_settings_page.h"

namespace midialf {

/* static */
const prog_EventHandlers PrgSettingsPage::event_handlers_ PROGMEM = {
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
void PrgSettingsPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void PrgSettingsPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t PrgSettingsPage::OnIncrement(uint8_t id, int8_t value) {
  switch (id) {
    case ENCODER_1: UpdateDirection(value); return 1;
    case ENCODER_2: UpdateStepsForward(value); return 1;
    case ENCODER_3: UpdateStepsBackward(value); return 1;
    case ENCODER_4: UpdateStepsReplay(value); return 1;
    case ENCODER_5: UpdateStepsInterval(value); return 1;
    case ENCODER_6: UpdateStepsRepeat(value); return 1;
    case ENCODER_7: UpdateStepsSkip(value); return 1;
    case ENCODER_8: UpdateLinkMode(value); return 1;
  }
  return 0;
}

/* static */
uint8_t PrgSettingsPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  switch (id) {
    case ENCODER_1: seq.set_direction(DIRECTION_FORWARD); return 1;
    case ENCODER_2: seq.set_steps_forward(0); return 1;
    case ENCODER_3: seq.set_steps_backward(0); return 1;
    case ENCODER_4: seq.set_steps_replay(0); return 1;
    case ENCODER_5: seq.set_steps_interval(0); return 1;
    case ENCODER_6: seq.set_steps_repeat(0); return 1;
    case ENCODER_7: seq.set_steps_skip(0); return 1;
    case ENCODER_8: seq.set_link_mode(LINK_MODE_NONE); return 1;
  }
  return 0;
}

/* static */
uint8_t PrgSettingsPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    switch (value) {
      case SWITCH_1: UpdateDirection(0); return 1;
      case SWITCH_2: UpdateStepsForward(0); return 1;
      case SWITCH_3: UpdateStepsBackward(0); return 1;
      case SWITCH_4: UpdateStepsReplay(0); return 1;
      case SWITCH_5: UpdateStepsInterval(0); return 1;
      case SWITCH_6: UpdateStepsRepeat(0); return 1;
      case SWITCH_7: UpdateStepsSkip(0); return 1;
      case SWITCH_8: UpdateLinkMode(0); return 1;
    }
  }
  return 0;
}

/* static */
uint8_t PrgSettingsPage::OnIdle() {
  return 0;
}

/* static */
void PrgSettingsPage::UpdateScreen() {
  DrawSeparators();
  DrawCells(0, PSTR(" DirForwBackReplIntvReptSkipLink"));
  
  char* line2 = display.line_buffer(1);
  
  DrawSelStr(&line2[cell_pos(0)], seq.direction(), PSTR("forwbackpendrand"));
  UnsafeItoa(seq.steps_forward(), 3, &line2[cell_pos(1) + 1]);
  UnsafeItoa(seq.steps_backward(), 3, &line2[cell_pos(2) + 1]);
  UnsafeItoa(seq.steps_replay(), 3, &line2[cell_pos(3) + 1]);
  UnsafeItoa(seq.steps_interval(), 3, &line2[cell_pos(4) + 1]);
  UnsafeItoa(seq.steps_repeat(), 3, &line2[cell_pos(5) + 1]);
  UnsafeItoa(seq.steps_skip(), 3, &line2[cell_pos(6) + 1]);
  DrawSelStr(&line2[cell_pos(7)], seq.link_mode(), PSTR(" no 2x161x32"));
}

/* static */
void PrgSettingsPage::UpdateLeds() {
}

///////////////////////////////////////////////////////////////////////////////
// Commmand handlers

/* static */
void PrgSettingsPage::UpdateDirection(int8_t value) {
  if (value) {
    seq.set_direction(Clamp(static_cast<int16_t>(seq.direction()) + value, DIRECTION_FORWARD, DIRECTION_RANDOM));
  } else {
    uint8_t direction = seq.direction();
    if (++direction > DIRECTION_RANDOM) direction = DIRECTION_FORWARD;
    seq.set_direction(direction);
  }
}

/* static */
void PrgSettingsPage::UpdateStepsForward(int8_t value) {
  if (value) {
    seq.set_steps_forward(Clamp(static_cast<int16_t>(seq.steps_forward()) + value, 0, kNumSteps));
  } else {
    uint8_t steps_forward = seq.steps_forward();
    if (++steps_forward > kNumSteps) steps_forward = 0;
    seq.set_steps_forward(steps_forward);
  }
}

/* static */
void PrgSettingsPage::UpdateStepsBackward(int8_t value) {
  if (value) {
    seq.set_steps_backward(Clamp(static_cast<int16_t>(seq.steps_backward()) + value, 0, kNumSteps));
  } else {
    uint8_t steps_backward = seq.steps_backward();
    if (++steps_backward > kNumSteps) steps_backward = 0;
    seq.set_steps_backward(steps_backward);
  }
}

/* static */
void PrgSettingsPage::UpdateStepsReplay(int8_t value) {
  if (value) {
    seq.set_steps_replay(Clamp(static_cast<int16_t>(seq.steps_replay()) + value, 0, kNumSteps));
  } else {
    uint8_t steps_replay = seq.steps_replay();
    if (++steps_replay > kNumSteps) steps_replay = 0;
    seq.set_steps_replay(steps_replay);
  }
}

/* static */
void PrgSettingsPage::UpdateStepsInterval(int8_t value) {
  if (value) {
    seq.set_steps_interval(Clamp(static_cast<int16_t>(seq.steps_interval()) + value, 0, kNumSteps));
  } else {
    uint8_t steps_interval = seq.steps_interval();
    if (++steps_interval > kNumSteps) steps_interval = 0;
    seq.set_steps_interval(steps_interval);
  }
}

/* static */
void PrgSettingsPage::UpdateStepsRepeat(int8_t value) {
  if (value) {
    seq.set_steps_repeat(Clamp(static_cast<int16_t>(seq.steps_repeat()) + value, 0, kNumSteps));
  } else {
    uint8_t steps_repeat = seq.steps_repeat();
    if (++steps_repeat > kNumSteps) steps_repeat = 0;
    seq.set_steps_repeat(steps_repeat);
  }
}

/* static */
void PrgSettingsPage::UpdateStepsSkip(int8_t value) {
  if (value) {
    seq.set_steps_skip(Clamp(static_cast<int16_t>(seq.steps_skip()) + value, 0, kNumSteps));
  } else {
    uint8_t steps_skip = seq.steps_skip();
    if (++steps_skip > kNumSteps) steps_skip = 0;
    seq.set_steps_skip(steps_skip);
  }
}

/* static */
void PrgSettingsPage::UpdateLinkMode(int8_t value) {
  if (value) {
    seq.set_link_mode(Clamp(static_cast<int16_t>(seq.link_mode()) + value, LINK_MODE_NONE, LINK_MODE_32));
  } else {
    uint8_t link_mode = seq.link_mode();
    if (++link_mode > LINK_MODE_32) link_mode = LINK_MODE_NONE;
    seq.set_link_mode(link_mode);
  }
}

} // namespace midialf
