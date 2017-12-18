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

#include "midialf/ui_pages/seq_settings_page.h"
#include "midialf/note_duration.h"
#include "midialf/clock.h"

namespace midialf {

/* static */
uint8_t SeqSettingsPage::num_taps_;

/* static */
uint32_t SeqSettingsPage::elapsed_time_;

/* static */
const prog_EventHandlers SeqSettingsPage::event_handlers_ PROGMEM = {
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
void SeqSettingsPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  num_taps_ = 0;
  elapsed_time_ = 0;
}

/* static */
void SeqSettingsPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t SeqSettingsPage::OnIncrement(uint8_t id, int8_t value) {
  switch (id) {
    case ENCODER_1: UpdateBpm(value); return 1;
    case ENCODER_2: UpdateClockRate(value); return 1;
    case ENCODER_3: UpdateGrooveTemplate(value); return 1;
    case ENCODER_4: UpdateGrooveAmount(value); return 1;
    case ENCODER_5: UpdateRootNote(Ui::FixOctaveIncrement(value)); return 1;
    case ENCODER_6: UpdateCC1(value); return 1;
    case ENCODER_7: UpdateCC2(value); return 1;
    case ENCODER_8: UpdateChannel(value); return 1;
  }
  return 0;
}

/* static */
uint8_t SeqSettingsPage::OnClick(uint8_t id, uint8_t value) {
  if (value != CLICK)
    return 0;

  switch (id) {
    case ENCODER_1: seq.set_bpm(GetDefSetting(seq.bpm(), kDefBpm, kDefBpm2)); return 1;
    case ENCODER_2: seq.set_clock_rate(GetDefSetting(seq.clock_rate(), kDefClockRate, kDefClockRate2)); return 1;
    case ENCODER_3: seq.set_groove_template(0); return 1;
    case ENCODER_4: seq.set_groove_amount(GetDefSetting(seq.groove_amount(), 0, 127)); return 1;
    case ENCODER_5: seq.set_root_note(kDefRootNote); return 1;
    case ENCODER_6: seq.set_cc1_numb(kDefCC1Numb); return 1;
    case ENCODER_7: seq.set_cc2_numb(kDefCC2Numb); return 1;
    case ENCODER_8: seq.set_channel(kDefChannel); return 1;
  }
  return 0;
}

/* static */
uint8_t SeqSettingsPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    switch (value) {
      case SWITCH_1: UpdateTapBpm(); return 1;
      case SWITCH_2: UpdateClockRate(0); return 1;
      case SWITCH_3: UpdateGrooveTemplate(0); return 1;
      case SWITCH_4: UpdateGrooveAmount(0); return 1;
      case SWITCH_5: seq.set_root_note(seq.last_received_note()); return 1;
      case SWITCH_6: seq.set_cc1_numb(seq.last_received_cc()); return 1;
      case SWITCH_7: seq.set_cc2_numb(seq.last_received_cc()); return 1;
      case SWITCH_8: UpdateChannel(0); return 1;
    }
  }
  return 0;
}

/* static */
uint8_t SeqSettingsPage::OnIdle() {
  return 0;
}

/* static */
void SeqSettingsPage::UpdateScreen() {
  DrawSeparators();
  DrawCells(0, PSTR("Temp Div GrvAmntRoot CC1 CC2Chan"));  

  char* line2 = display.line_buffer(1);

  UnsafeItoa(seq.bpm(), 3, &line2[cell_pos(0) + 1]);
  DrawSelStr(&line2[cell_pos(1)], seq.clock_rate(), midi_clock_ticks_per_note_str);
  DrawSelStr(&line2[cell_pos(2)], seq.groove_template(), PSTR("swngshflpush laghumnmnky"));
  UnsafeItoa(seq.groove_amount(), 3, &line2[cell_pos(3) + 1]);
  Ui::PrintNote(&line2[cell_pos(4)], seq.root_note());
  UnsafeItoa(seq.cc1_numb(), 3, &line2[cell_pos(5) + 1]);
  UnsafeItoa(seq.cc2_numb(), 3, &line2[cell_pos(6) + 1]);
  UnsafeItoa(seq.channel() + 1, 3, &line2[cell_pos(7) + 1]);
}

/* static */
void SeqSettingsPage::UpdateLeds() {
}

///////////////////////////////////////////////////////////////////////////////
// Commmand handlers

/* static */
void SeqSettingsPage::UpdateBpm(int8_t value) {
  seq.set_bpm(Clamp(static_cast<int16_t>(seq.bpm()) + value, kMinBpm, kMaxBpm));
}

/* static */
void SeqSettingsPage::UpdateTapBpm() {
  // Tap bpm code shamlessly stolen from Mutable Instruments MidiPal ClockSource app
  // by Olivier Gillet (ol.gillet@gmail.com)
  uint32_t t = clock.value();
  clock.Reset();
  if (num_taps_ > 0 && t < 400000L) {
    elapsed_time_ += t;
    seq.set_bpm(avrlib::Clip(18750000 * num_taps_ / elapsed_time_, kMinBpm, kMaxBpm));
  } else {
    num_taps_ = 0;
    elapsed_time_ = 0;
  }
  ++num_taps_;
}

/* static */
void SeqSettingsPage::UpdateClockRate(int8_t value) {
  if (value) {
    seq.set_clock_rate(Clamp(static_cast<int16_t>(seq.clock_rate()) + value, 0, kNoteDurationCount - 1));
  } else {
    uint8_t clock_rate = seq.clock_rate();
    if (++clock_rate >= kNoteDurationCount) clock_rate = 0;
    seq.set_clock_rate(clock_rate);
  }
}

/* static */
void SeqSettingsPage::UpdateGrooveTemplate(int8_t value) {
  if (value) {
    seq.set_groove_template(Clamp(static_cast<int16_t>(seq.groove_template()) + value, 0, kNumGrooveTemplates - 1));
  } else {
    uint8_t groove_template = seq.groove_template();
    if (++groove_template >= kNumGrooveTemplates) groove_template = 0;
    seq.set_groove_template(groove_template);
  }
}

/* static */
void SeqSettingsPage::UpdateGrooveAmount(int8_t value) {
  if (value) {
    seq.set_groove_amount(Clamp7F(static_cast<int16_t>(seq.groove_amount()) + value));
  } else {
    uint8_t groove_amount = seq.groove_amount();
    if (++groove_amount > 0x7f) groove_amount = 0;
    seq.set_groove_amount(groove_amount);
  }
}

/* static */
void SeqSettingsPage::UpdateRootNote(int8_t value) {
  if (value) {
    seq.set_root_note(Clamp7F(static_cast<int16_t>(seq.root_note()) + value));
  } else {
    uint8_t root_note = seq.root_note();
    if (++root_note > 0x7f) root_note = 0;
    seq.set_root_note(root_note);
  }
}

/* static */
void SeqSettingsPage::UpdateCC1(int8_t value) {
  if (value) {
    seq.set_cc1_numb(Clamp7F(static_cast<int16_t>(seq.cc1_numb()) + value));
  } else {
    uint8_t cc1_numb = seq.cc1_numb();
    if (++cc1_numb > 0x7f) cc1_numb = 0;
    seq.set_cc1_numb(cc1_numb);
  }
}

/* static */
void SeqSettingsPage::UpdateCC2(int8_t value) {
  if (value) {
    seq.set_cc2_numb(Clamp7F(static_cast<int16_t>(seq.cc2_numb()) + value));
  } else {
    uint8_t cc2_numb = seq.cc2_numb();
    if (++cc2_numb > 0x7f) cc2_numb = 0;
    seq.set_cc2_numb(cc2_numb);
  }
}

/* static */
void SeqSettingsPage::UpdateChannel(int8_t value) {
  if (value) {
    seq.set_channel(Clamp(static_cast<int16_t>(seq.channel()) + value, 0, 0xf));
  } else {
    uint8_t channel = seq.channel();
    if (++channel > 0xf) channel = 0;
    seq.set_channel(channel);
  }
}

} // namespace midialf
