// Copyright 2012 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// Based on MidiPal code by Olivier Gillet (ol.gillet@gmail.com)
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
// Sequence data handling.

#include "midialf/seq.h"
#include "midialf/event_scheduler.h"
#include "midialf/note_duration.h"
#include "midialf/midi_handler.h"
#include "midialf/display.h"
#include "midialf/storage.h"
#include "midialf/clock.h"
#include "midialf/leds.h"
#include "midialf/lfo.h"
#include "midialf/ui.h"
#ifdef ENABLE_CV_OUTPUT
#include "midialf/cv/cv.h"
#include "midialf/cv/port.h"
#endif

#include "avrlib/random.h"

#include <avr/eeprom.h>
#include <util/crc16.h>

namespace midialf {

using namespace avrlib;

Serial<MidiPort, 31250, DISABLED, POLLED> midi_out;

/* extern */
Seq seq;

/* <static> */
SeqData Seq::data_[4];

uint8_t Seq::name_[kNameLength];
uint8_t Seq::slot_;
uint8_t Seq::channel_ = kDefChannel;
uint8_t Seq::bpm_ = kDefBpm;
uint8_t Seq::clock_rate_ = kDefClockRate;
uint8_t Seq::clock_mode_ = CLOCK_MODE_INTERNAL;
uint8_t Seq::clock_division_ = CLOCK_DIVISION_NONE;
uint8_t Seq::direction_ = DIRECTION_FORWARD;
uint8_t Seq::groove_template_ = 0;
uint8_t Seq::groove_amount_ = 0;
uint8_t Seq::root_note_ = kDefRootNote;
uint8_t Seq::link_mode_ = LINK_MODE_NONE;
uint8_t Seq::cc1_numb_ = kDefCC1Numb;
uint8_t Seq::cc2_numb_ = kDefCC2Numb;
uint8_t Seq::steps_forward_;
uint8_t Seq::steps_backward_;
uint8_t Seq::steps_replay_;
uint8_t Seq::steps_interval_;
uint8_t Seq::steps_repeat_;
uint8_t Seq::steps_skip_;
uint8_t Seq::seq_switch_mode_;
uint8_t Seq::cv_mode_[4];
uint8_t Seq::gate_mode_[4];

uint8_t Seq::seq_;
uint8_t Seq::tick_;
uint8_t Seq::step_;
uint8_t Seq::running_;
uint8_t Seq::recording_;
uint8_t Seq::step_hold_;
uint8_t Seq::last_note_;
uint8_t Seq::pendulum_backward_;
uint8_t Seq::midi_clock_prescaler_;
uint8_t Seq::clock_division_counter_;
uint8_t Seq::steps_forward_counter_;
uint8_t Seq::steps_replay_counter_;
uint8_t Seq::steps_interval_counter_;
uint8_t Seq::steps_repeat_counter_;
uint8_t Seq::steps_skip_counter_;
uint8_t Seq::saved_replay_step_;
uint8_t Seq::available_steps_;
uint8_t Seq::next_step_;
uint8_t Seq::next_seq_;
uint8_t Seq::last_recorded_step_;
uint8_t Seq::last_recorded_seq_;
uint8_t Seq::last_legato_note_;
uint8_t Seq::manual_step_selected_;
uint8_t Seq::bank_select_msb_;
uint8_t Seq::bank_select_lsb_;
uint8_t Seq::prog_change_flags_;
uint8_t Seq::ctrl_change_flags_;
uint8_t Seq::strobe_width_;
uint8_t Seq::last_received_note_;
uint8_t Seq::last_received_cc_;
uint8_t Seq::request_set_seq_;

NoteStack<16> Seq::note_stack_;
/* </static> */

// Helper class for profiling

struct RaiseIOPort1 {
  RaiseIOPort1() { IOPort1::High(); }
  ~RaiseIOPort1() { IOPort1::Low(); }
};

///////////////////////////////////////////////////////////////////////////////
// Sequence data rotation helpers

static void ArrayRotL(uint8_t* pb, uint8_t cb) {
  uint8_t tmp = pb[0];
  for (uint8_t n = 0; n < cb - 1; n++) {
    pb[n] = pb[n + 1];
  }
  pb[cb - 1] = tmp;
}

static void ArrayRotR(uint8_t* pb, uint8_t cb) {
  uint8_t tmp = pb[cb - 1];
  for (uint8_t n = cb - 1; n > 0; n--) {
    pb[n] = pb[n - 1];
  }
  pb[0] = tmp;
}

static uint8_t U8RotL(uint8_t byte) {
  uint8_t carry = byte & 0x80; byte<<= 1; 
  if (carry) byte|= 1;
  return byte;
}

static uint8_t U8RotR(uint8_t byte) {
  uint8_t carry = byte & 1; byte>>= 1; 
  if (carry) byte|= 0x80;
  return byte;
}

///////////////////////////////////////////////////////////////////////////////
// Sequence data handling

void SeqData::Init() {
  for (uint8_t n = 0; n < 8; n++) {
    note_[n] = kDefRootNote;
    velo_[n] = kDefVelocity;
    gate_[n] = kDefNoteLeng;
    cc1_[n] = kDefCC1Value;
    cc2_[n] = kDefCC1Value;
    mute_ = 0;
    skip_ = 0;
    lega_ = 0;
    cc1send_ = 0;
    cc2send_ = 0;
  }
}

void SeqData::Rand() {
  Init(); uint8_t mask = 1; mute_ = 0;
  for (uint8_t n = 0; n < 8; n++, mask<<= 1) {
    uint8_t note = Random::GetByte() & 0x0f;
    if (note < 12) {
      note_[n] = kDefRootNote + note;
    } else
      mute_|= mask;
  }
  // FIXME: user arp and drum patterns for step randomization later...
}

void SeqData::RotL() {
  ArrayRotL(note_, numbof(note_));
  ArrayRotL(velo_, numbof(velo_));
  ArrayRotL(gate_, numbof(gate_));
  ArrayRotL(cc1_, numbof(cc1_));
  ArrayRotL(cc2_, numbof(cc2_));
  mute_ = U8RotR(mute_);
  skip_ = U8RotR(skip_);
  lega_ = U8RotR(lega_);
  cc1send_ = U8RotR(cc1send_);
  cc2send_ = U8RotR(cc2send_);
}

void SeqData::RotR() {
  ArrayRotR(note_, numbof(note_));
  ArrayRotR(velo_, numbof(velo_));
  ArrayRotR(gate_, numbof(gate_));
  ArrayRotR(cc1_, numbof(cc1_));
  ArrayRotR(cc2_, numbof(cc2_));
  mute_ = U8RotL(mute_);
  skip_ = U8RotL(skip_);
  lega_ = U8RotL(lega_);
  cc1send_ = U8RotL(cc1send_);
  cc2send_ = U8RotL(cc2send_);
}

void SeqData::Verify() {
  for (uint8_t n = 0; n < 8; n++) {
    if (note_[n] > 127 || velo_[n] > 127 || gate_[n] >= kNoteDurationCount || 
      cc1_[n] > 127  || cc2_[n] > 127 ) {
      Init();
      return;
    }
  }
}

void SeqData::SwapSteps(uint8_t step1, uint8_t step2) {
  uint8_t tmp;
  tmp = note(step1); set_note(step1, note(step2)); set_note(step2, tmp);
  tmp = velo(step1); set_velo(step1, velo(step2)); set_velo(step2, tmp);
  tmp = gate(step1); set_gate(step1, gate(step2)); set_gate(step2, tmp);
  tmp = cc1(step1); set_cc1(step1, cc1(step2)); set_cc1(step2, tmp);
  tmp = cc2(step1); set_cc2(step1, cc2(step2)); set_cc2(step2, tmp);
  tmp = mute(step1); set_mute(step1, mute(step2)); set_mute(step2, tmp);
  tmp = skip(step1); set_skip(step1, skip(step2)); set_skip(step2, tmp);
  tmp = lega(step1); set_lega(step1, lega(step2)); set_lega(step2, tmp);
  tmp = cc1send(step1); set_cc1send(step1, cc1send(step2)); set_cc1send(step2, tmp);
  tmp = cc2send(step1); set_cc2send(step1, cc2send(step2)); set_cc2send(step2, tmp);
}

///////////////////////////////////////////////////////////////////////////////
// Sequence info handling

const prog_uint8_t default_seqinfo_data[] PROGMEM = {
  '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',
  0, 0, // slot, channel
  kDefBpm, kDefClockRate, CLOCK_MODE_INTERNAL, CLOCK_DIVISION_NONE, 
  DIRECTION_FORWARD, 0, 0, // groove_template_, groove_amount_
  kDefRootNote, LINK_MODE_NONE, kDefCC1Numb, kDefCC2Numb,
  0, 0, 0, // forward, backward, replay
  0, 0, 0, // interval, repeat, skip
  kDefLfoResolution,
  kDefLfo1CCNumber, kDefLfo1Amount, kDefLfo1Center, kDefLfo1Waveform, kDefLfo1Rate, kDefLfo1Sync, 
  kDefLfo2CCNumber, kDefLfo2Amount, kDefLfo2Center, kDefLfo2Waveform, kDefLfo2Rate, kDefLfo2Sync, 
  kDefCV1Mode, kDefCV2Mode, kDefCV3Mode, kDefCV4Mode, 
  kDefGate1Mode, kDefGate2Mode, kDefGate3Mode, kDefGate4Mode, 
  kDefSeqSwitchMode,
};

void SeqInfo::Init(uint8_t slot) {
  memcpy_P(this, default_seqinfo_data, sizeof(SeqInfo));
  slot_ = slot;
}

///////////////////////////////////////////////////////////////////////////////
// Sequence object handling

/* static */
void Seq::Init() {
  OnInit();
  note_stack_.Init();
}

///////////////////////////////////////////////////////////////////////////////
// MIDI event handlers

/* static */
void Seq::OnInit() {
//lcd.SetCustomCharMapRes(chr_res_digits_10, 7, 1);
//lcd.SetCustomCharMapRes(chr_res_sequencer_icons, 4, 1);
  UpdateClock();
  clock.Start();
  running_ = 0;
  set_seq(0);
  set_step(0);
  UpdatePrescaler();
}

/* static */
void Seq::OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (channel != channel_)
    return;

  // Save last received note for learning
  last_received_note_ = note;
  
  if (!running_ || recording_) {  
    Send3(0x90 | channel, note, velocity);
    note_stack_.NoteOn(note, velocity);
  } else
    last_note_ = note;

  if (recording_) {
      RecordStep(note, velocity);
  }
}

/* static */
void Seq::OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (channel != channel_)
    return;
  
  if (!running_ || recording_) {
    uint8_t top_note = note_stack_.most_recent_note().note;
    note_stack_.NoteOff(note);
    Send3(0x80 | channel, note, velocity);
    if (recording_ && note_stack_.size() > 0) {
      // Most recent pressed key has been released, however some keys are 
      // still pressed, so we need to record the most recent one...
      if (top_note == note) {
        RecordStep(note_stack_.most_recent_note().note, note_stack_.most_recent_note().velocity);
      }
    }
  }
}

/* static */
void Seq::OnAftertouch(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (channel != channel_)
    return;

#ifdef ENABLE_CV_OUTPUT
    cv.SendATch(velocity);
#endif
}

/* static */
void Seq::OnAftertouch(uint8_t channel, uint8_t velocity) {
  if (channel != channel_)
    return;

#ifdef ENABLE_CV_OUTPUT
    cv.SendATch(velocity);
#endif
}

/* static */
void Seq::OnControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
  if (channel != channel_)
    return;

  // Save last received cc for learning
  last_received_cc_ = controller; 

  // Save bank select for program change
  switch (controller) {
    case 0x00: bank_select_msb_ = value; break;
    case 0x20: bank_select_lsb_ = value; break;
  }

  if (HandleCC(channel, controller, value))
    return;

  if (controller == cc1_numb_) {
#ifdef ENABLE_CV_OUTPUT
    cv.SendCC1(value);
#endif
    if (recording_) {
      set_cc1(value);
      Ui::AddRequest(REQUEST_UPDATESCREEN);
    }
  } else
  if (controller == cc2_numb_) {
#ifdef ENABLE_CV_OUTPUT
    cv.SendCC2(value);
#endif
    if (recording_) {
      set_cc2(value);
      Ui::AddRequest(REQUEST_UPDATESCREEN);
    }
  }
}

/* static */
void Seq::OnProgramChange(uint8_t channel, uint8_t program) {
  if (channel != channel_)
    return;

  if (prog_change_flags_ & PROGRAM_CHANGE_RECV) {
    LoadFromStorage((bank_select_lsb_ << 7) + program);
  }
}

/* static */
void Seq::OnPitchBend(uint8_t channel, uint16_t pitch_bend) {
  if (channel != channel_)
    return;

#ifdef ENABLE_CV_OUTPUT
  cv.SendPBnd(pitch_bend);
#endif
}

/* static */
void Seq::OnSysExByte(uint8_t sysex_byte) {
}

/* static */
void Seq::OnClock() {
  if (clock_mode_ == CLOCK_MODE_EXTERNAL && running_) {
    Tick();
  }
}

/* static */
void Seq::OnStart() {
  if (clock_mode_ == CLOCK_MODE_EXTERNAL) {
    Start();
  }
}

/* static */
void Seq::OnContinue() {
  if (clock_mode_ == CLOCK_MODE_EXTERNAL) {
    running_ = 1;
  }
}

/* static */
void Seq::OnStop() {
  if (clock_mode_ == CLOCK_MODE_EXTERNAL) {
    Stop();
  }
}

/* static */
uint8_t Seq::CheckChannel(uint8_t channel) {
  return 1;
}

/* static */
void Seq::OnRawByte(uint8_t byte) {
}

/* static */
void Seq::OnRawMidiData(
   uint8_t status,
   uint8_t* data,
   uint8_t data_size,
   uint8_t accepted_channel) {
  // Forward everything except note on for the selected channel.
  if (status != (0x80 | channel_) && 
      status != (0x90 | channel_)) {
    Send(status, data, data_size);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Event handlers for internal clock

/* static */
void Seq::OnInternalClockTick() {
  if (clock_mode_ == CLOCK_MODE_INTERNAL && running_) {
    SendNow(0xf8);
    Tick();
  } else
  // Make sure ENCx click notes get their note offs served
  if (!running_ && event_scheduler.size() > 0) {
    SendScheduledNotes(channel_);
  }
}

/* static */
void Seq::OnInternalClockStep() {
}

///////////////////////////////////////////////////////////////////////////////
// Sequence actions

/* static */
void Seq::SetSeq(uint8_t seq) {
  if (seq == seq_)
    return;

  if (running_ && seq_switch_mode_) {
    request_set_seq_ = 1 + seq;
  } else
    set_seq(seq);
}

/* static */
void Seq::Stop() {
  if (!running_)
    return;
  
#ifdef ENABLE_CV_OUTPUT
  port.SendStart(0);
#endif

  // Flush the note off messages in the queue.
  FlushQueue(channel_);

  // Send an all notes/controllers off messages.
  Send3(0xb0 | channel_, 123, 0);
  Send3(0xb0 | channel_, 121, 0);
  if (clock_mode_ == CLOCK_MODE_INTERNAL) {
    SendNow(0xfc);
  }

  running_ = 0;
  request_set_seq_ = 0;
  last_legato_note_ = -1;

  lfo.Stop();
}

/* static */
void Seq::Start() {
  if (running_)
    return;

  if (clock_mode_ == CLOCK_MODE_INTERNAL) {
    clock.Start();
    SendNow(0xfa);
  }

  UpdatePrescaler();

#ifdef ENABLE_CV_OUTPUT
  port.SendStart(1);
#endif

  saved_replay_step_ = 0;

  // Set step to start with
  if (step_hold_) {
    ;
  } else
  if (CheckAllSkipped()) {
    step_ = 0;
  } else {
    // Set initial step one before the start step to ensure consistent led behavior
    switch (direction_) {
      case DIRECTION_FORWARD: {
          uint8_t first_step = (kNumSteps << link_mode_) - 1;
          switch (link_mode_) {
            case LINK_MODE_16: seq_ = (seq_ & 2) + (first_step >> 3); break;
            case LINK_MODE_32: seq_ = first_step >> 3; break;
          }
          step_ = first_step & (kNumSteps - 1);
        }
        break;
      case DIRECTION_BACKWARD: 
        step_ = 0; 
        break;
      case DIRECTION_PENDULUM:
        step_ = 1; pendulum_backward_ = 1;
        switch (link_mode_) {
          case LINK_MODE_16: seq_&= 2; break;
          case LINK_MODE_32: seq_ = 0; break;
        }
        break;
      case DIRECTION_RANDOM: 
        Random::Seed((uint16_t)milliseconds()); 
        break;
    }
    AdvanceStep();
  }

  // Reset progression controls
  steps_forward_counter_ = 0;
  steps_replay_counter_ = 0;
  steps_interval_counter_ = 0;
  steps_repeat_counter_ = 0;
  steps_skip_counter_ = 0;

  // Reset running controls
  tick_ = midi_clock_prescaler_ - 1;
  clock_division_counter_ = -1;
  last_legato_note_ = -1;
  last_note_ = root_note_;
  request_set_seq_ = 0;
  running_ = -1;
  
  lfo.Start();
}

/* static */
void Seq::Tick() {
  ++tick_;

  lfo.Tick();

#ifdef ENABLE_CV_OUTPUT
  port.SendClock();
#endif

  SendScheduledNotes(channel_);

  if (tick_ < midi_clock_prescaler_)
    return;

  tick_ = 0;

  switch (clock_division_) {
    case CLOCK_DIVISION_X2:
      if ((clock_division_counter_++ & 1) != 1)
        return;
      break;
    case CLOCK_DIVISION_X4:
      if ((clock_division_counter_++ & 3) != 3)
        return;
      break;
  }

  if (running_ == 1) {
    if (!step_hold_) {
      AdvanceStep();
    }
  } else
    running_ = 1;

  if (step_ == 0) {
    switch (link_mode_) {
      case LINK_MODE_16:
        if ((seq_ & 1) == 0) 
          goto OnSeq;
        break;
      case LINK_MODE_32:
        if (seq_) 
          break;
        // fall through
      default:
OnSeq:  if (request_set_seq_) {
          set_seq(request_set_seq_ - 1);
          request_set_seq_ = 0;
          Ui::RequestRefresh();
        }
#ifdef ENABLE_CV_OUTPUT
        port.SendSeq();
#endif
        break;
    }
  }

  // Send CCs and note if not muted
  SendStep();
}

/* static */
void Seq::AdvanceStep() {
  manual_step_selected_ = 0;

  // If all steps are skipped, do nothing
  available_steps_ = CountAvailableSteps();
  if (available_steps_ == 0)
    return;

  // Setup running position
  next_step_ = step_;
  next_seq_ = seq_;

  // Make it a virtual step
  switch (link_mode_) {
    case LINK_MODE_16: if (seq_ & 1) next_step_+= kNumSteps; break;
    case LINK_MODE_32: if (seq_) next_step_+= seq_ * kNumSteps; break;
  }
 
  // Step progression logic based on MIDIbox SEQ V4 code by Thorsten Klose (tk@midibox.org)
  uint8_t save_step = 0;
  uint8_t advance = 1;
 
  // Check if progrssion options are not applicable
  if (available_steps_ > 1 && direction_ != DIRECTION_RANDOM) {

    if (steps_forward_ && ++steps_forward_counter_ >= steps_forward_) {
      steps_forward_counter_ = 0;
      if (steps_backward_) {
        for (uint8_t i = 0; i < steps_backward_; ++i) {
          AdvanceStep(GetRunningDirection(1)); // reverse
        }
      }
      if (++steps_replay_counter_ > steps_replay_) {
        steps_replay_counter_ = 0;
        save_step = 1;
      } else {
        next_step_ = saved_replay_step_;
        advance = 0;
      }
    }

    if (steps_interval_ && !steps_repeat_counter_ && !steps_skip_counter_) {
      if (++steps_interval_counter_ >= steps_interval_) {
        steps_interval_counter_ = 0;
        steps_repeat_counter_ = steps_repeat_;
        steps_skip_counter_ = steps_skip_;
      }
    }

    if (steps_repeat_counter_) {
      --steps_repeat_counter_;
      advance = 0;
    } else {
      while(steps_skip_counter_) {
        AdvanceStep(GetRunningDirection(0));  // forward
        --steps_skip_counter_;
      }
    }
  }

  // Advance to the next step if necessary
  if (advance) {
    AdvanceStep(direction_);
  }

  // Advance to the next sequence
  if (next_seq_ != seq_) {
    set_seq(next_seq_);
    Ui::AddRequest(REQUEST_UPDATESCREEN);
  }

  set_step(next_step_ & (kNumSteps - 1));

  // Save step if requested
  if (save_step) {
    saved_replay_step_ = next_step_;
  }

  // Sync lfos
  lfo.OnStep();
}

/* static */
void Seq::AdvanceStep(uint8_t direction) {
  uint8_t current_step = next_step_;

  // Find out number of virtual steps
  uint8_t num_steps = kNumSteps << link_mode_;

  // Advance to the next virtual step
  for (;;) {
    switch (direction) {
      case DIRECTION_FORWARD:
        if (++next_step_ >= num_steps) {
          next_step_ = 0;
        }
        break;
      case DIRECTION_BACKWARD:
        if (next_step_-- == 0) {
          next_step_ = num_steps - 1;
        }
        break;
      case DIRECTION_PENDULUM:
        if (pendulum_backward_) {
          if (next_step_-- == 0) {
            pendulum_backward_ = 0;
            next_step_ = 1;
          }
        } else {
          if (++next_step_ >= num_steps) {
            pendulum_backward_ = 1;
            next_step_ = num_steps - 2;
          }
        }
        break;
      case DIRECTION_RANDOM:
        do {
          next_step_ = Random::GetByte() & (num_steps - 1); 
        } while (next_step_ == current_step && available_steps_ > 1);
        break;
    }

    // Figure out next sequence
    switch (link_mode_) {
      case LINK_MODE_16: next_seq_ = (next_seq_ & 2) + (next_step_ >> 3); break;
      case LINK_MODE_32: next_seq_ = next_step_ >> 3; break;
    }

    // Check if next step is not skipped, keep trying if it is
    if (!skip(next_seq_, next_step_ & (kNumSteps - 1))) {
      // Check if we've got the same step and keep trying only if we have more than one step to
      // ensure that we activate the next not skipped step
      if (next_step_ == current_step && available_steps_ > 1)
        continue;
      break;
    }
  }
}

/* static */
void Seq::UpdateClock() {
  clock.Update(bpm_, groove_template_, groove_amount_);
#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("Seq::UpdateClock: bpm=%u grv=%u amt=%u\n", bpm_, groove_template_, groove_amount_);
#endif
}

/* static */
void Seq::UpdatePrescaler() {
  midi_clock_prescaler_ = NoteDuration::GetMidiClockTicks(clock_rate_);
#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("Seq::UpdatePrescaler: clock_rate=%u midi_clock_tick_per_step=%u\n", clock_rate, midi_clock_tick_per_step);
#endif
  lfo.UpdatePrescaler();
}

/* static */
void Seq::UpdateStrobeWidth() {
#ifdef ENABLE_CV_OUTPUT
  port.UpdateStrobeWidth();
#endif
}

/* static */
void Seq::RecordStep(uint8_t note, uint8_t velocity) {
  // Check if we have last recorded step and update its legato state
  if (last_recorded_step_ != 0xff) {
    uint8_t legato = note_stack_.size() > 1;
    set_lega(last_recorded_seq_, last_recorded_step_, legato);
  }

  // Record note
  set_note(note);
  set_velo(velocity);

  // Save last recorded step and sequence for legato recording
  last_recorded_step_ = step_;
  last_recorded_seq_ = seq_;

  // Advance step unless recording in real time
  if (!running_) {
    AdvanceStep();
  }
   
  // Request screen update
  Ui::AddRequest(REQUEST_UPDATESCREEN);
}

/* static */
uint8_t Seq::CheckAllSkipped() {
  switch (link_mode_) {
    case LINK_MODE_NONE: 
      return all_skipped();
    case LINK_MODE_16:
      return all_skipped(seq_ & 2) && all_skipped((seq_ & 2) + 1);
    case LINK_MODE_32:
      return all_skipped(0) && all_skipped(1) && all_skipped(2) && all_skipped(3);
  }
}

/* static */
uint8_t Seq::CountAvailableSteps() {
  switch (link_mode_) {
    case LINK_MODE_NONE: 
      return available_steps();
    case LINK_MODE_16:
      return available_steps(seq_ & 2) + available_steps((seq_ & 2) + 1);
    case LINK_MODE_32:
      return available_steps(0) + available_steps(1) + available_steps(2) + available_steps(3);
  }
}

/* static */
uint8_t Seq::GetRunningDirection(uint8_t reversed) {
  if (direction_ == DIRECTION_BACKWARD || (direction_ == DIRECTION_PENDULUM && pendulum_backward_)) {
    reversed = !reversed;
  }
  return reversed ? DIRECTION_BACKWARD : DIRECTION_FORWARD;
}

/* static */
uint8_t Seq::IsFirstStep(uint8_t step) {
  if (direction_ != DIRECTION_BACKWARD)
    return step == 0;

  if (link_mode_ == LINK_MODE_16) {
    if ((seq_ & 1) == 0) 
      return 0; 
  } else
  if (link_mode_ == LINK_MODE_32) {
    if (seq_ < 3)
      return 0; 
  }

  return step == kNumSteps - 1;
}

///////////////////////////////////////////////////////////////////////////////
// Output helpers

/* static */
void Seq::SendNow(uint8_t byte) {
#ifndef MIDIOUT_DEBUG_OUTPUT  
  midi_out.Write(byte);
#ifndef ENABLE_CV_OUTPUT
  if (byte != 0xf8) FlashMidiOutLed();
#endif
#endif
}

/* static */
void Seq::Send2(uint8_t a, uint8_t b) {
#ifndef MIDIOUT_DEBUG_OUTPUT  
  FlushOutputBuffer(2);
  MidiHandler::OutputBuffer::Write(a);
  MidiHandler::OutputBuffer::Write(b);
#endif
}

/* static */
void Seq::Send3(uint8_t a, uint8_t b, uint8_t c) {
#ifdef ENABLE_CV_OUTPUT
  if ((a & 0x0f) == channel_) {
    switch (a & 0xf0) {
      case 0x80:
        if (!running_ || recording_) {
          if (note_stack_.size() == 0) {
            port.SendGate(0);
          }
        } else {
          // This check assumes we're only scheduling note offs
          if (event_scheduler.size() <= 1) {
            port.SendGate(0);
          }
        }
        break;
      case 0x90:
        cv.SendNote(b);
        cv.SendVelo(c);
        if (c) {
          port.SendGate(1);
          port.SendStrobe();
        }
        break;
      case 0xb0:
        if (b == cc1_numb()) {
          cv.SendCC1(c);
        } else
        if (b == cc2_numb()) {
          cv.SendCC2(c);
        }
        break;
    }
  }
#endif

#ifndef MIDIOUT_DEBUG_OUTPUT  
  FlushOutputBuffer(3);
  MidiHandler::OutputBuffer::Write(a);
  MidiHandler::OutputBuffer::Write(b);
  MidiHandler::OutputBuffer::Write(c);
#endif
}

/* static */
void Seq::Send(uint8_t status, uint8_t* data, uint8_t size) {
#ifndef MIDIOUT_DEBUG_OUTPUT  
  FlushOutputBuffer(1 + size);
  MidiHandler::OutputBuffer::Write(status);
  if (size) {
    MidiHandler::OutputBuffer::Write(*data++);
    --size;
  }
  if (size) {
    MidiHandler::OutputBuffer::Write(*data++);
    --size;
  }
#endif
}

/* static */
void Seq::FlushOutputBuffer(uint8_t requested_size) {
#ifndef MIDIOUT_DEBUG_OUTPUT  
  while (MidiHandler::OutputBuffer::writable() < requested_size) {
    display.set_status('!');
    uint8_t byte = MidiHandler::OutputBuffer::Read();
#ifndef ENABLE_CV_OUTPUT
    if (byte != 0xf8) FlashMidiOutLed();
#endif
    midi_out.Write(byte);
  }
#endif
}

/* static */
void Seq::SendLater(uint8_t note, uint8_t velocity, uint8_t when, uint8_t tag) {
#ifndef MIDIOUT_DEBUG_OUTPUT  
  event_scheduler.Schedule(note, velocity, when, tag);
  if (!clock.running()) {
    clock.Start();
  }
#endif
}

/* static */
void Seq::SendScheduledNotes(uint8_t channel) {
  uint8_t current = event_scheduler.root();
  while (current) {
    const SchedulerEntry& entry = event_scheduler.entry(current);
    if (entry.when) {
      break;
    }
    if (entry.note != kZombieSlot) {
      if (entry.velocity == 0) {
        Send3(0x80 | channel, entry.note, 0);
      } else {
        Send3(0x90 | channel, entry.note, entry.velocity);
      }
    }
    current = entry.next;
  }
  event_scheduler.Tick();
}

/* static */
void Seq::FlushQueue(uint8_t channel) {
  while (event_scheduler.size()) {
    SendScheduledNotes(channel);
  }
}

/* static */
void Seq::ToggleRun() {
  if (!running_) {
    Start();
  } else
    Stop();
}

/* static */
void Seq::StopRecording() {
  if (recording_) {
    recording_ = 0;
  }
}

/* static */
void Seq::StartRecording() {
  if (!recording_) {
    step_ = 0;
    direction_ = DIRECTION_FORWARD;
    last_note_ = root_note_;
    last_recorded_step_ = 0xff;
    recording_ = 1;
  }
}

/* static */
void Seq::ToggleRecording() {
  if (!recording_) {
    StartRecording();
  } else
    StopRecording();
}

/* static */
void Seq::AdvanceClockDivision() {
  if (clock_division_ < CLOCK_DIVISION_X4) {
    clock_division_++;
  } else
    clock_division_ = CLOCK_DIVISION_NONE;
}

/* static */
void Seq::ToggleStepHold() {
  step_hold_ = !step_hold_;
}

/* static */
void Seq::CopyTo(uint8_t seq) {
  if (seq_ != seq) {
    data_[seq] = data_[seq_];
  }
}

/* static */
void Seq::SendStep(uint8_t step) {
  // Send CCs
  if (cc1send(step)) {
    Send3(0xb0 | channel_, cc1_numb(), cc1(step));
  }
  if (cc2send()) {
    Send3(0xb0 | channel_, cc2_numb(), cc2(step));
  }

  // Send note if not muted
  if (velo(step) > 0 && (!mute(step) || manual_step_selected_)) {

    // Fixup last note
    if (last_note_ == 0) {
      last_note_ = root_note_;
    } 

    // Send note
    uint8_t the_note = Clip(static_cast<int16_t>(note(step)) + last_note_ - root_note_, 0, 0x7f);
    Send3(0x90 | channel_, the_note, velo(step));

    // Release previous legato note if any
    if (last_legato_note_ != 0xff) {
      Send3(0x80 | channel_, last_legato_note_, 0);
      last_legato_note_ = 0xff;
    }

    // Schedule note off
    if (lega(step) && !(step_hold_ || manual_step_selected_)) {
      last_legato_note_ = the_note;
    } else {
      uint8_t duration = NoteDuration::GetMidiClockTicks(gate(step));
      SendLater(the_note, 0, duration - 1);
    }
  }
}

/* static */
void Seq::SendNote(uint8_t note, uint8_t velo) {
  Send3(0x90 | channel_, note & 0x7f, velo & 0x7f);
}

/* static */
void Seq::SendNoteOff(uint8_t note, uint8_t velo) {
  Send3(0x80 | channel_, note & 0x7f, velo & 0x7f);
}

/* static */
void Seq::InitSeqInfo() {
  SeqInfo info(slot_); LoadSeqInfo(info);
}

///////////////////////////////////////////////////////////////////////////////
// Save/Load to internal EEPROM, used to persist current state

/* static */
uint16_t Seq::CalcCrc16(uint16_t crc16) {

  uint8_t* p = (uint8_t*)&data_[0];
  for (uint16_t cb = sizeof(data_); cb--; ) {
    crc16 = _crc16_update(crc16, *p++);
  }

  p = &name_[0];
  for (uint8_t cb = sizeof(name_); cb--; ) {
    crc16 = _crc16_update(crc16, *p++);
  }

  crc16 = _crc16_update(crc16, slot_);
  crc16 = _crc16_update(crc16, channel_);
  crc16 = _crc16_update(crc16, bpm_);
  crc16 = _crc16_update(crc16, clock_rate_);
  crc16 = _crc16_update(crc16, clock_mode_);
  crc16 = _crc16_update(crc16, clock_division_);  
  crc16 = _crc16_update(crc16, direction_);
  crc16 = _crc16_update(crc16, groove_template_);
  crc16 = _crc16_update(crc16, groove_amount_);
  crc16 = _crc16_update(crc16, root_note_);
  crc16 = _crc16_update(crc16, link_mode_);
  crc16 = _crc16_update(crc16, cc1_numb_);
  crc16 = _crc16_update(crc16, cc2_numb_);
  crc16 = _crc16_update(crc16, steps_forward_);
  crc16 = _crc16_update(crc16, steps_backward_);
  crc16 = _crc16_update(crc16, steps_replay_);
  crc16 = _crc16_update(crc16, steps_interval_);
  crc16 = _crc16_update(crc16, steps_repeat_);
  crc16 = _crc16_update(crc16, steps_skip_);

  p = &cv_mode_[0];
  for (uint8_t cb = sizeof(cv_mode_); cb--; ) {
    crc16 = _crc16_update(crc16, *p++);
  }

  p = &gate_mode_[0];
  for (uint8_t cb = sizeof(gate_mode_); cb--; ) {
    crc16 = _crc16_update(crc16, *p++);
  }

  crc16 = _crc16_update(crc16, seq_switch_mode_);

  crc16 = lfo.CalcCrc16(crc16);

  // These are not part of the sequence data but we track/save them anyway
  crc16 = _crc16_update(crc16, prog_change_flags_);
  crc16 = _crc16_update(crc16, ctrl_change_flags_);
  crc16 = _crc16_update(crc16, strobe_width_);

  return crc16;
}

/* static */
void Seq::LoadSeqData(uint8_t seq, const SeqData& data) {
  memcpy(&data_[seq], &data, sizeof(data));
  VerifySeqData(seq);
}

/* static */
void Seq::SaveToIntEeprom(uint8_t* p) {
  SeqInfo info; SaveSeqInfo(info);
  eeprom_update_block(&info, p, sizeof(SeqInfo)); p+= sizeof(SeqInfo);
  eeprom_update_block(&data_[0], p, sizeof(data_)); p+= sizeof(data_);
}

/* static */
void Seq::LoadFromIntEeprom(const uint8_t* p) {
  SeqInfo info;
  eeprom_read_block(&info, p, sizeof(SeqInfo)); p+= sizeof(SeqInfo);
  eeprom_read_block(&data_[0], p, sizeof(data_)); p+= sizeof(data_);
  LoadSeqInfo(info);
  VerifySeqData();
}

///////////////////////////////////////////////////////////////////////////////
// Save/Load to external EEPROM

/* static */
void Seq::SaveToStorage(uint8_t slot) {
#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("Seq::SaveToStorage: slot=%03u\n", slot + 1);
#endif
  // This takes about 75ms (!!!)
  set_slot(slot);
  SeqInfo info; SaveSeqInfo(info);
  storage.WriteSeqInfo(slot, &info);
  storage.WriteSeqData(slot, &data_[0], sizeof(data_));
}

/* static */
void Seq::LoadFromStorage(uint8_t slot) {
#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("Seq::LoadFromStorage: slot=%03u\n", slot + 1);
#endif
  // This takes about 25ms (!!!)
  SeqInfo info;
  if (storage.ReadSeqInfo(slot, &info)) {
    LoadSeqInfo(info); info.slot_ = slot_;
    storage.ReadSeqData(slot, &data_[0], sizeof(data_));
    VerifySeqData(); set_slot(slot); 
  }
}

/* static */
void Seq::SaveSeqInfo(SeqInfo& info) {
  memcpy(info.name_, name_, kNameLength);
  info.slot_ = slot_;
  info.channel_ = channel_;
  info.bpm_ = bpm_;
  info.clock_rate_ = clock_rate_;
  info.clock_mode_ = clock_mode_;
  info.clock_division_ = clock_division_;
  info.direction_ = direction_;
  info.groove_template_ = groove_template_;
  info.groove_amount_ = groove_amount_;
  info.root_note_ = root_note_;
  info.link_mode_ = link_mode_;
  info.cc1_numb_ = cc1_numb_;
  info.cc2_numb_ = cc2_numb_;
  info.steps_forward_ = steps_forward_;
  info.steps_backward_ = steps_backward_;
  info.steps_replay_ = steps_replay_;
  info.steps_interval_ = steps_interval_;
  info.steps_repeat_ = steps_repeat_;
  info.steps_skip_ = steps_skip_;
  memcpy(info.cv_mode_, cv_mode_, sizeof(cv_mode_));
  memcpy(info.gate_mode_, gate_mode_, sizeof(gate_mode_));
  info.seq_switch_mode_ = seq_switch_mode_;
  lfo.SaveLfoInfo(info);
}

/* static */
void Seq::LoadSeqInfo(const SeqInfo& info) {
  memcpy(name_, info.name_, kNameLength);
  slot_ = info.slot_;
  channel_ = Verify(info.channel_, 0, 0xf, kDefChannel);
  bpm_ = Verify(info.bpm_, kMinBpm, kMaxBpm, kDefBpm);
  clock_rate_ = Verify(info.clock_rate_, 0, kNoteDurationCount - 1, kDefClockRate);
  clock_mode_ = Verify(info.clock_mode_, CLOCK_MODE_INTERNAL, CLOCK_MODE_EXTERNAL, CLOCK_MODE_INTERNAL);
  clock_division_ = Verify(info.clock_division_, CLOCK_DIVISION_NONE, CLOCK_DIVISION_X4, CLOCK_DIVISION_NONE);
  direction_ = Verify(info.direction_, DIRECTION_FORWARD, DIRECTION_RANDOM, DIRECTION_FORWARD);
  groove_template_ = Verify(info.groove_template_, 0, kNumGrooveTemplates - 1, 0);
  groove_amount_ = Verify(info.groove_amount_, 0, 127, 0);
  root_note_ = Verify(info.root_note_, 0, 127, kDefRootNote);
  link_mode_ = Verify(info.link_mode_, LINK_MODE_NONE, LINK_MODE_32, LINK_MODE_NONE);
  cc1_numb_ = Verify(info.cc1_numb_, 0, 127, kDefCC1Numb);
  cc2_numb_ = Verify(info.cc2_numb_, 0, 127, kDefCC2Numb);
  steps_forward_ = Verify(info.steps_forward_, 0, kNumSteps, 0);
  steps_backward_ = Verify(info.steps_backward_, 0, kNumSteps, 0);
  steps_replay_ = Verify(info.steps_replay_, 0, kNumSteps, 0);
  steps_interval_ = Verify(info.steps_interval_, 0, kNumSteps, 0);
  steps_repeat_ = Verify(info.steps_repeat_, 0, kNumSteps, 0);
  steps_skip_ = Verify(info.steps_skip_, 0, kNumSteps, 0);
  
  cv_mode_[0] = Verify(info.cv_mode_[0] & CVMODE_MASK, 0, CVMODE_MAX, kDefCV1Mode) | (info.cv_mode_[0] & CVMODE_OFFSET);
  cv_mode_[1] = Verify(info.cv_mode_[1] & CVMODE_MASK, 0, CVMODE_MAX, kDefCV2Mode) | (info.cv_mode_[1] & CVMODE_OFFSET);
  cv_mode_[2] = Verify(info.cv_mode_[2] & CVMODE_MASK, 0, CVMODE_MAX, kDefCV3Mode) | (info.cv_mode_[2] & CVMODE_OFFSET);
  cv_mode_[3] = Verify(info.cv_mode_[3] & CVMODE_MASK, 0, CVMODE_MAX, kDefCV4Mode) | (info.cv_mode_[3] & CVMODE_OFFSET);

  gate_mode_[0] = Verify(info.gate_mode_[0] & GATEMODE_MASK, 0, GATEMODE_MAX, kDefGate1Mode) | (info.gate_mode_[0] & GATEMODE_INVERT);
  gate_mode_[1] = Verify(info.gate_mode_[1] & GATEMODE_MASK, 0, GATEMODE_MAX, kDefGate2Mode) | (info.gate_mode_[1] & GATEMODE_INVERT);
  gate_mode_[2] = Verify(info.gate_mode_[2] & GATEMODE_MASK, 0, GATEMODE_MAX, kDefGate3Mode) | (info.gate_mode_[2] & GATEMODE_INVERT);
  gate_mode_[3] = Verify(info.gate_mode_[3] & GATEMODE_MASK, 0, GATEMODE_MAX, kDefGate4Mode) | (info.gate_mode_[3] & GATEMODE_INVERT);

  seq_switch_mode_ = Verify(info.seq_switch_mode_, SEQ_SWITCH_MODE_IMMEDIATE, SEQ_SWITCH_MODE_ONSEQEND, kDefSeqSwitchMode);

#ifdef ENABLE_CV_OUTPUT
  port.UpdateCvOffset();
  port.UpdateGateInvert();
#endif

  lfo.LoadLfoInfo(info);
  
  UpdateClock();
  UpdatePrescaler();
}

/* static */
uint8_t Seq::HasSeqName() {
  for (uint8_t n = 0; n < kNameLength; n++) {
    if (name_[n] != '_')
      return 1;
  }
  return 0;
}

/* static */
uint8_t Seq::Verify(uint8_t value, uint8_t min, uint8_t max, uint8_t def) {
  return value >= min && value <= max ? value : def;
}

/* static */
void Seq::VerifySeqData() {
  for (uint8_t n = 0; n < numbof(data_); n++) {
    VerifySeqData(n);
  }
}

/* static */
void Seq::FixSeqName(uint8_t* name) {
  for (uint8_t n = 0; n < kNameLength; n++) {
    if (name[n] < kMinNameChar || name[n] > kMaxNameChar) {
      name[n] = '_';
    }
  }
}

/* static */
uint8_t Seq::HandleCC(uint8_t channel, uint8_t controller, uint8_t value) {
  if (!(ctrl_change_flags_ & CONTROL_CHANGE_RECV))
    return 0;

  int16_t setting;
  switch (controller) {

    case kCCSetSequence:
      if (value < 4) {
        SetSeq(value);
      }
      break;

    case kCCSetDirection:
      if (value <= DIRECTION_RANDOM) {
        set_direction(value);
      }
      break;

    case kCCSetLinkMode:
      if (value <= LINK_MODE_32) {
        set_link_mode(value);
      }
      break;

    case kCCSetClockDivision: 
      if (value <= CLOCK_DIVISION_X4) {
        set_clock_division(value);
      }
      break;

    case kCCIncreaseTempo:
      if (value) {
        setting = static_cast<int16_t>(bpm_) + 1;
        goto SetBpm;
      }
      break;

    case kCCDecreaseTempo:
      if (value) {
        setting = static_cast<int16_t>(bpm_) - 1;
        goto SetBpm;
      }
      break;

    case kCCSetTempo:
      setting = Ui::Scale(static_cast<int16_t>(value), 0, 127, 25, 250);
SetBpm: set_bpm(Clip(setting, kMinBpm, kMaxBpm));
      break;

    case kCCToggleRun: 
      if (value) {
        ToggleRun(); 
      }
      break;

    case kCCStopSequencer:
      if (value) {
        Stop(); 
      }
      break;

    case kCCStartSequencer:
      if (value) {
        Start(); 
      }
      break;

    case kCCToggleRecording:
      if (value) {
        ToggleRecording(); 
      }
      break;

    case kCCStopRecording:
      if (value) {
        StopRecording();
      }
      break;

    case kCCStartRecording:
      if (value) {
        StartRecording();
      }
      break;

    default:
      return 0;
  }

  Ui::RequestRefresh();

  return 1;
}

/* static */
void Seq::RotLSeq() {
  // Find out number of virtual steps
  uint8_t num_steps = kNumSteps << link_mode_;

  // Save first step
  uint8_t saved_note = note(0, 0);
  uint8_t saved_velo = velo(0, 0);
  uint8_t saved_gate = gate(0, 0);
  uint8_t saved_cc1 = cc1(0, 0);
  uint8_t saved_cc2 = cc2(0, 0);
  uint8_t saved_mute = mute(0, 0);
  uint8_t saved_skip = skip(0, 0);
  uint8_t saved_lega = lega(0, 0);
  uint8_t saved_cc1send = cc1send(0, 0);
  uint8_t saved_cc2send = cc2send(0, 0);

  // Move steps left
  uint8_t n, step, step2, seq, seq2;
  for (n = 0; n < num_steps - 1; n++) {
    step = n % kNumSteps; seq = n / kNumSteps;
    step2 = (n + 1) % kNumSteps; seq2 = (n + 1) / kNumSteps;

    // Move right step to the left
    set_note(seq, step, note(seq2, step2));
    set_velo(seq, step, velo(seq2, step2));
    set_gate(seq, step, gate(seq2, step2));
    set_cc1(seq, step, cc1(seq2, step2));
    set_cc2(seq, step, cc2(seq2, step2));
    set_mute(seq, step, mute(seq2, step2));
    set_skip(seq, step, skip(seq2, step2));
    set_lega(seq, step, lega(seq2, step2));
    set_cc1send(seq, step, cc1send(seq2, step2));
    set_cc2send(seq, step, cc2send(seq2, step2));
  }

  // Copy saved first step into the last step
  step = n % kNumSteps; seq = n / kNumSteps;
  set_note(seq, step, saved_note);
  set_velo(seq, step, saved_velo);
  set_gate(seq, step, saved_gate);
  set_cc1(seq, step, saved_cc1);
  set_cc2(seq, step, saved_cc2);
  set_mute(seq, step, saved_mute);
  set_skip(seq, step, saved_skip);
  set_lega(seq, step, saved_lega);
  set_cc1send(seq, step, saved_cc1send);
  set_cc2send(seq, step, saved_cc2send);
}

/* static */
void Seq::RotRSeq() {
  // Find out number of virtual steps
  uint8_t num_steps = kNumSteps << link_mode_;

  // Save last step
  uint8_t n = num_steps - 1;
  uint8_t step = n % kNumSteps; 
  uint8_t seq = n / kNumSteps;

  uint8_t saved_note = note(seq, step);
  uint8_t saved_velo = velo(seq, step);
  uint8_t saved_gate = gate(seq, step);
  uint8_t saved_cc1 = cc1(seq, step);
  uint8_t saved_cc2 = cc2(seq, step);
  uint8_t saved_mute = mute(seq, step);
  uint8_t saved_skip = skip(seq, step);
  uint8_t saved_lega = lega(seq, step);
  uint8_t saved_cc1send = cc1send(seq, step);
  uint8_t saved_cc2send = cc2send(seq, step);

  // Move steps right
  uint8_t step2, seq2;
  for ( ; n > 0; n--) {
    step = n % kNumSteps; seq = n / kNumSteps;
    step2 = (n - 1) % kNumSteps; seq2 = (n - 1) / kNumSteps;

    // Move left step to the right
    set_note(seq, step, note(seq2, step2));
    set_velo(seq, step, velo(seq2, step2));
    set_gate(seq, step, gate(seq2, step2));
    set_cc1(seq, step, cc1(seq2, step2));
    set_cc2(seq, step, cc2(seq2, step2));
    set_mute(seq, step, mute(seq2, step2));
    set_skip(seq, step, skip(seq2, step2));
    set_lega(seq, step, lega(seq2, step2));
    set_cc1send(seq, step, cc1send(seq2, step2));
    set_cc2send(seq, step, cc2send(seq2, step2));
  }

  // Copy saved last step into the first step
  set_note(0, 0, saved_note);
  set_velo(0, 0, saved_velo);
  set_gate(0, 0, saved_gate);
  set_cc1(0, 0, saved_cc1);
  set_cc2(0, 0, saved_cc2);
  set_mute(0, 0, saved_mute);
  set_skip(0, 0, saved_skip);
  set_lega(0, 0, saved_lega);
  set_cc1send(0, 0, saved_cc1send);
  set_cc2send(0, 0, saved_cc2send);
}

/* static */
void Seq::SetFirstStep(uint8_t step) {
  // Make it a virtual step
  switch (link_mode_) {
    case LINK_MODE_16: if (seq_ & 1) step+= kNumSteps; break;
    case LINK_MODE_32: if (seq_) step+= seq_ * kNumSteps; break;
  }

  // Rotate left to make it the first step
  for (uint8_t n = 0; n < step; n++) {
    RotLSeq();
  }
}

} // namespace midialf
