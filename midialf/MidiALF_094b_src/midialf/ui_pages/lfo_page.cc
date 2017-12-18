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
// LFO settings page class.

#include "midialf/ui_pages/lfo_page.h"
#include "midialf/note_duration.h"

namespace midialf {

/* static */
uint8_t LfoPage::lfo_;

/* static */
const prog_EventHandlers LfoPage::event_handlers_ PROGMEM = {
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
void LfoPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  lfo_ = pageInfo->index - PAGE_LFO1;
}

/* static */
void LfoPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t LfoPage::OnIncrement(uint8_t id, int8_t value) {
  switch (id) {
    case ENCODER_1: UpdateWaveform(value); return 1;
    case ENCODER_2: UpdateCC(value); return 1;
    case ENCODER_3: UpdateResolution(value); return 1;
    case ENCODER_4: UpdateAmount(value); return 1;
    case ENCODER_5: UpdateCenter(value); return 1;
    case ENCODER_6: UpdateRate(value); return 1;
    case ENCODER_7: UpdateSync(value); return 1;
    case ENCODER_8: return 1;
  }
  return 0;
}

/* static */
uint8_t LfoPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  switch (id) {
    case ENCODER_1: lfo.set_waveform(lfo_, lfo_ ? kDefLfo2Waveform : kDefLfo1Waveform); return 1;
    case ENCODER_2: 
      lfo.set_cc_number(lfo_, 
        GetDefSetting(lfo.cc_number(lfo_), 
          lfo_ ? kDefLfo2CCNumber : kDefLfo1CCNumber, 
          lfo_ ? kDefLfo2CCNumber2 : kDefLfo1CCNumber2));
      return 1;
    case ENCODER_3: lfo.set_resolution(kDefLfoResolution); return 1;
    case ENCODER_4: 
      lfo.set_amount(lfo_, 
        GetDefSetting(lfo.amount(lfo_), 
          lfo_ ? kDefLfo2Amount : kDefLfo1Amount, 
          lfo_ ? kDefLfo2Amount2 : kDefLfo1Amount2));
      return 1;
    case ENCODER_5: lfo.set_center(lfo_, lfo_ ? kDefLfo2Center : kDefLfo1Center); return 1;
    case ENCODER_6: lfo.set_rate(lfo_, lfo_ ? kDefLfo2Rate : kDefLfo1Rate); return 1;
    case ENCODER_7: lfo.set_sync(lfo_, lfo_ ? kDefLfo2Sync : kDefLfo1Sync); return 1;
    case ENCODER_8: return 1;
  }
  return 0;
}

/* static */
uint8_t LfoPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    switch (value) {
      case SWITCH_1: UpdateWaveform(0); return 1;
      case SWITCH_2: lfo.set_cc_number(lfo_, seq.last_received_cc()); return 1;
      case SWITCH_3: UpdateResolution(0); return 1;
      case SWITCH_4: UpdateAmount(0); return 1;
      case SWITCH_5: UpdateCenter(0); return 1;
      case SWITCH_6: UpdateRate(0); return 1;
      case SWITCH_7: UpdateSync(0); return 1;
      case SWITCH_8: return 1;
    }
  }
  return 0;
}

/* static */
uint8_t LfoPage::OnIdle() {
  return 0;
}

/* static */
void LfoPage::UpdateScreen() {
  DrawSeparators();
  DrawCells(0, PSTR("LFO? CC ResoAmntCentRateSync----"));
  
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  line1[cell_pos(0) + 3] = '1' + lfo_;

  DrawSelStr(&line2[cell_pos(0)], lfo.waveform(lfo_), PSTR("none tri sqrramp sinsin2sin3sin5grg1grg2bat1bat2spk1spk2 lp1 lp2 rs1 rs2 s&h"));

  UnsafeItoa(lfo.cc_number(lfo_), 3, &line2[cell_pos(1) + 1]);

  DrawSelStr(&line2[cell_pos(2)], lfo.resolution(), midi_clock_ticks_per_note_str);
  UnsafeItoa((int16_t)lfo.amount(lfo_) - 63, 3, &line2[cell_pos(3) + 1]);
  UnsafeItoa(lfo.center(lfo_), 3, &line2[cell_pos(4) + 1]);

  DrawSelStr(&line2[cell_pos(5)], kNumLfoRates - 1 - lfo.rate(lfo_), 
    midi_clock_ticks_per_note_str + (kNoteDurationCount - kNumLfoRates) * 4);
  
  DrawSelStr(&line2[cell_pos(6)], lfo.sync(lfo_), PSTR("freestrt seqstep"));
}

/* static */
void LfoPage::UpdateLeds() {
}

///////////////////////////////////////////////////////////////////////////////
// Commmand handlers

/* static */
void LfoPage::UpdateWaveform(int8_t value) {
  if (value) {
    lfo.set_waveform(lfo_, Clamp(static_cast<int16_t>(lfo.waveform(lfo_)) + value, 0, kNumLfoWaveforms - 1));
  } else {
    uint8_t waveform = lfo.waveform(lfo_);
    if (++waveform >= kNumLfoWaveforms) waveform = 0;
    lfo.set_waveform(lfo_, waveform);
  }
}

/* static */
void LfoPage::UpdateCC(int8_t value) {
  if (value) {
    lfo.set_cc_number(lfo_, Clamp7F(static_cast<int16_t>(lfo.cc_number(lfo_)) + value));
  } else {
    uint8_t cc_number = lfo.cc_number(lfo_);
    if (++cc_number > 0x7f) cc_number = 0;
    lfo.set_cc_number(lfo_, cc_number);
  }
}

/* static */
void LfoPage::UpdateResolution(int8_t value) {
  if (value) {
    lfo.set_resolution(Clamp(static_cast<int16_t>(lfo.resolution()) - value, 0, kNoteDurationCount - 1));
  } else {
    uint8_t resolution = lfo.resolution();
    if (++resolution >= kNoteDurationCount) resolution = 0;
    lfo.set_resolution(resolution);
  }
}

/* static */
void LfoPage::UpdateAmount(int8_t value) {
  if (value) {
    lfo.set_amount(lfo_, Clamp(static_cast<int16_t>(lfo.amount(lfo_)) + value, 0, 126));
  } else {
    uint8_t amount = lfo.amount(lfo_);
    if (++amount > 126) amount = 0;
    lfo.set_amount(lfo_, amount);
  }
}

/* static */
void LfoPage::UpdateCenter(int8_t value) {
  if (value) {
    lfo.set_center(lfo_, Clamp(static_cast<int16_t>(lfo.center(lfo_)) + value, 0, 127));
  } else {
    uint8_t center = lfo.center(lfo_);
    if (++center > 127) center = 0;
    lfo.set_center(lfo_, center);
  }
}

/* static */
void LfoPage::UpdateRate(int8_t value) {
  if (value) {
    lfo.set_rate(lfo_, Clamp(static_cast<int16_t>(lfo.rate(lfo_)) + value, 0, kNumLfoRates - 1));
  } else {
    uint8_t rate = lfo.rate(lfo_);
    if (++rate >= kNumLfoRates) rate = 0;
    lfo.set_rate(lfo_, rate);
  }
}

/* static */
void LfoPage::UpdateSync(int8_t value) {
  if (value) {
    lfo.set_sync(lfo_, Clamp(static_cast<int16_t>(lfo.sync(lfo_)) + value, 0, LFO_SYNC_STEP));
  } else {
    uint8_t sync = lfo.sync(lfo_);
    if (++sync > LFO_SYNC_STEP) sync = 0;
    lfo.set_sync(lfo_, sync);
  }
}

} // namespace midialf
