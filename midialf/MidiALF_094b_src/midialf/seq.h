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

#ifndef MIDIALF_SEQ_H_
#define MIDIALF_SEQ_H_

#include "midialf/midialf.h"
#include "midialf/note_stack.h"
#include "midialf/lfo.h"

namespace midialf {

class SeqData {
public:

  SeqData() { Init(); }

  void Init();
  void Rand();
  void RotL();
  void RotR();

  void Verify();

  inline uint8_t note(uint8_t step) { return note_[step]; }
  inline uint8_t velo(uint8_t step) { return velo_[step]; }
  inline uint8_t gate(uint8_t step) { return gate_[step]; }
  inline uint8_t cc1(uint8_t step) { return cc1_[step]; }
  inline uint8_t cc2(uint8_t step) { return cc2_[step]; }
  inline uint8_t mute(uint8_t step) { return mute_ & (1 << step); }
  inline uint8_t skip(uint8_t step) { return skip_ & (1 << step); }
  inline uint8_t lega(uint8_t step) { return lega_ & (1 << step); }
  inline uint8_t cc1send(uint8_t step) { return cc1send_ & (1 << step); }
  inline uint8_t cc2send(uint8_t step) { return cc2send_ & (1 << step); }

  inline void set_note(uint8_t step, uint8_t value) { note_[step] = value; }
  inline void set_velo(uint8_t step, uint8_t value) { velo_[step] = value; }
  inline void set_gate(uint8_t step, uint8_t value) { gate_[step] = value; }
  inline void set_cc1(uint8_t step, uint8_t value) { cc1_[step] = value; }
  inline void set_cc2(uint8_t step, uint8_t value) { cc2_[step] = value; }
  inline void set_mute(uint8_t step, uint8_t value) { value ? mute_ |= (1 << step) : mute_ &= ~(1 << step); }
  inline void set_skip(uint8_t step, uint8_t value) { value ? skip_ |= (1 << step) : skip_ &= ~(1 << step); }
  inline void set_lega(uint8_t step, uint8_t value) { value ? lega_ |= (1 << step) : lega_ &= ~(1 << step); }
  inline void set_cc1send(uint8_t step, uint8_t value) { value ? cc1send_ |= (1 << step) : cc1send_ &= ~(1 << step); }
  inline void set_cc2send(uint8_t step, uint8_t value) { value ? cc2send_ |= (1 << step) : cc2send_ &= ~(1 << step); }

  inline uint8_t mute_mask() { return mute_; }
  inline uint8_t skip_mask() { return skip_; }
  inline uint8_t lega_mask() { return lega_; }
  inline uint8_t cc1send_mask() { return cc1send_; }
  inline uint8_t cc2send_mask() { return cc2send_; }

  inline void set_mute_mask(uint8_t value) { mute_ = value; }
  inline void set_skip_mask(uint8_t value) { skip_ = value; }
  inline void set_lega_mask(uint8_t value) { lega_ = value; }
  inline void set_cc1send_mask(uint8_t value) { cc1send_ = value; }
  inline void set_cc2send_mask(uint8_t value) { cc2send_ = value; }

  inline uint8_t all_skipped() { return skip_ == 0xff; }

  inline uint8_t available_steps() {
    uint8_t skip = skip_;
    uint8_t count = 0;
    for ( ; skip; count++) {
      skip&= skip - 1;
    }
    return 8 - count;
  }
  
  void SwapSteps(uint8_t step1, uint8_t step2);

private:
  uint8_t note_[8];
  uint8_t velo_[8];
  uint8_t gate_[8];
  uint8_t cc1_[8];
  uint8_t cc2_[8];
  uint8_t mute_;
  uint8_t skip_;
  uint8_t lega_;
  uint8_t cc1send_;
  uint8_t cc2send_;
};

class SeqInfo {
 public:
  SeqInfo(uint8_t slot = 0) { Init(slot); }

  uint8_t name_[kNameLength];
  uint8_t slot_;
  uint8_t channel_;
  uint8_t bpm_;
  uint8_t clock_rate_;
  uint8_t clock_mode_;
  uint8_t clock_division_;  
  uint8_t direction_;
  uint8_t groove_template_;
  uint8_t groove_amount_;
  uint8_t root_note_;
  uint8_t link_mode_;
  uint8_t cc1_numb_;
  uint8_t cc2_numb_;
  uint8_t steps_forward_;
  uint8_t steps_backward_;
  uint8_t steps_replay_;
  uint8_t steps_interval_;
  uint8_t steps_repeat_;
  uint8_t steps_skip_;

  uint8_t lfo_resolution_;
  LfoData lfo_data_[kNumLfos];
  
  uint8_t cv_mode_[4];
  uint8_t gate_mode_[4];

  uint8_t seq_switch_mode_;

  void Init(uint8_t slot = 0);
};

class Seq {
 public:
  Seq() {}
  
  static void Init();
  
  // MIDI event handlers
  static void OnInit();
  static void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
  static void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
  static void OnAftertouch(uint8_t channel, uint8_t note, uint8_t velocity);
  static void OnAftertouch(uint8_t channel, uint8_t velocity);
  static void OnControlChange(uint8_t channel, uint8_t controller, uint8_t value);
  static void OnProgramChange(uint8_t channel, uint8_t program);
  static void OnPitchBend(uint8_t channel, uint16_t pitch_bend);
  static void OnSysExByte(uint8_t sysex_byte);
  static void OnClock();
  static void OnStart();
  static void OnContinue();
  static void OnStop();

  static uint8_t CheckChannel(uint8_t channel);
  
  static void OnRawByte(uint8_t byte);

  static void OnRawMidiData(
     uint8_t status,
     uint8_t* data,
     uint8_t data_size,
     uint8_t accepted_channel);
  
  // Event handlers for internal clock
  static void OnInternalClockTick();
  static void OnInternalClockStep();
  
  // Output helpers
  static void FlushOutputBuffer(uint8_t size);
  static void SendNow(uint8_t byte);
  static void Send(uint8_t status, uint8_t* data, uint8_t size);
  static void Send2(uint8_t a, uint8_t b);
  static void Send3(uint8_t a, uint8_t b, uint8_t c);
  static void SendLater(uint8_t note, uint8_t velocity, uint8_t when) {
    SendLater(note, velocity, when, 0);
  }
  static void SendLater(uint8_t note, uint8_t velocity, uint8_t when, uint8_t tag);
  static void SendScheduledNotes(uint8_t channel);
  static void FlushQueue(uint8_t channel);

  // Sequence data accessors
  static uint8_t seq() { return seq_; }
  static uint8_t step() { return step_; }
  
  static void set_seq(uint8_t seq) { seq_ = seq & 0x03; }
  static void set_step(uint8_t step) { step_ = step & 0x07; }

  static uint8_t note() { return note(seq_, step_); }
  static uint8_t velo() { return velo(seq_, step_); }
  static uint8_t gate() { return gate(seq_, step_); }
  static uint8_t cc1() { return cc1(seq_, step_); }
  static uint8_t cc2() { return cc2(seq_, step_); }
  static uint8_t mute() { return mute(seq_, step_); }
  static uint8_t skip() { return skip(seq_, step_); }
  static uint8_t lega() { return lega(seq_, step_); }
  static uint8_t cc1send() { return cc1send(seq_, step_); }
  static uint8_t cc2send() { return cc2send(seq_, step_); }

  static uint8_t note(uint8_t step) { return note(seq_, step); }
  static uint8_t velo(uint8_t step) { return velo(seq_, step); }
  static uint8_t gate(uint8_t step) { return gate(seq_, step); }
  static uint8_t cc1(uint8_t step) { return cc1(seq_, step); }
  static uint8_t cc2(uint8_t step) { return cc2(seq_, step); }
  static uint8_t mute(uint8_t step) { return mute(seq_, step); }
  static uint8_t skip(uint8_t step) { return skip(seq_, step); }
  static uint8_t lega(uint8_t step) { return lega(seq_, step); }
  static uint8_t cc1send(uint8_t step) { return cc1send(seq_, step); }
  static uint8_t cc2send(uint8_t step) { return cc2send(seq_, step); }

  static uint8_t note(uint8_t seq, uint8_t step) { return data_[seq].note(step); }
  static uint8_t velo(uint8_t seq, uint8_t step) { return data_[seq].velo(step); }
  static uint8_t gate(uint8_t seq, uint8_t step) { return data_[seq].gate(step); }
  static uint8_t cc1(uint8_t seq, uint8_t step) { return data_[seq].cc1(step); }
  static uint8_t cc2(uint8_t seq, uint8_t step) { return data_[seq].cc2(step); }
  static uint8_t mute(uint8_t seq, uint8_t step) { return data_[seq].mute(step); }
  static uint8_t skip(uint8_t seq, uint8_t step) { return data_[seq].skip(step); }
  static uint8_t lega(uint8_t seq, uint8_t step) { return data_[seq].lega(step); }
  static uint8_t cc1send(uint8_t seq, uint8_t step) { return data_[seq].cc1send(step); }
  static uint8_t cc2send(uint8_t seq, uint8_t step) { return data_[seq].cc2send(step); }

  static void set_note(uint8_t value) { set_note(seq_, step_, value); }
  static void set_velo(uint8_t value) { set_velo(seq_, step_, value); }
  static void set_gate(uint8_t value) { set_gate(seq_, step_, value); }
  static void set_cc1(uint8_t value) { set_cc1(seq_, step_, value); }
  static void set_cc2(uint8_t value) { set_cc2(seq_, step_, value); }
  static void set_mute(uint8_t value) { set_mute(seq_, step_, value); }
  static void set_skip(uint8_t value) { set_skip(seq_, step_, value); }
  static void set_lega(uint8_t value) { set_lega(seq_, step_, value); }
  static void set_cc1send(uint8_t value) { set_cc1send(seq_, step_, value); }
  static void set_cc2send(uint8_t value) { set_cc2send(seq_, step_, value); }

  static void set_note(uint8_t step, uint8_t value) { set_note(seq_, step, value); }
  static void set_velo(uint8_t step, uint8_t value) { set_velo(seq_, step, value); }
  static void set_gate(uint8_t step, uint8_t value) { set_gate(seq_, step, value); }
  static void set_cc1(uint8_t step, uint8_t value) { set_cc1(seq_, step, value); }
  static void set_cc2(uint8_t step, uint8_t value) { set_cc2(seq_, step, value); }
  static void set_mute(uint8_t step, uint8_t value) { set_mute(seq_, step, value); }
  static void set_skip(uint8_t step, uint8_t value) { set_skip(seq_, step, value); }
  static void set_lega(uint8_t step, uint8_t value) { set_lega(seq_, step, value); }
  static void set_cc1send(uint8_t step, uint8_t value) { set_cc1send(seq_, step, value); }
  static void set_cc2send(uint8_t step, uint8_t value) { set_cc2send(seq_, step, value); }

  static void set_note(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_note(step, value); }
  static void set_velo(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_velo(step, value); }
  static void set_gate(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_gate(step, value); }
  static void set_cc1(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_cc1(step, value); }
  static void set_cc2(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_cc2(step, value); }
  static void set_mute(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_mute(step, value); }
  static void set_skip(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_skip(step, value); }
  static void set_lega(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_lega(step, value); }
  static void set_cc1send(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_cc1send(step, value); }
  static void set_cc2send(uint8_t seq, uint8_t step, uint8_t value) { data_[seq].set_cc2send(step, value); }

  static uint8_t mute_mask(uint8_t seq) { return data_[seq].mute_mask(); }
  static uint8_t skip_mask(uint8_t seq) { return data_[seq].skip_mask(); }
  static uint8_t lega_mask(uint8_t seq) { return data_[seq].lega_mask(); }
  static uint8_t cc1send_mask(uint8_t seq) { return data_[seq].cc1send_mask(); }
  static uint8_t cc2send_mask(uint8_t seq) { return data_[seq].cc2send_mask(); }

  static uint8_t mute_mask() { return mute_mask(seq_); }
  static uint8_t skip_mask() { return skip_mask(seq_); }
  static uint8_t lega_mask() { return lega_mask(seq_); }
  static uint8_t cc1send_mask() { return cc1send_mask(seq_); }
  static uint8_t cc2send_mask() { return cc2send_mask(seq_); }

  static void set_mute_mask(uint8_t seq, uint8_t value) { return data_[seq].set_mute_mask(value); }
  static void set_skip_mask(uint8_t seq, uint8_t value) { return data_[seq].set_skip_mask(value); }
  static void set_lega_mask(uint8_t seq, uint8_t value) { return data_[seq].set_lega_mask(value); }
  static void set_cc1send_mask(uint8_t seq, uint8_t value) { return data_[seq].set_cc1send_mask(value); }
  static void set_cc2send_mask(uint8_t seq, uint8_t value) { return data_[seq].set_cc2send_mask(value); }

  static void set_mute_mask(uint8_t value) { return set_mute_mask(seq_, value); }
  static void set_skip_mask(uint8_t value) { return set_skip_mask(seq_, value); }
  static void set_lega_mask(uint8_t value) { return set_lega_mask(seq_, value); }
  static void set_cc1send_mask(uint8_t value) { return set_cc1send_mask(seq_, value); }
  static void set_cc2send_mask(uint8_t value) { return set_cc2send_mask(seq_, value); }

  static uint8_t all_skipped(uint8_t seq) { return data_[seq].all_skipped(); }
  static uint8_t all_skipped() { return all_skipped(seq_); }

  static uint8_t available_steps(uint8_t seq) { return data_[seq].available_steps(); }
  static uint8_t available_steps() { return available_steps(seq_); }

  // Sequence parameter accessors

  static uint8_t slot() { return slot_; }
  static void set_slot(uint8_t slot) { slot_ = slot; }

  static uint8_t channel() { return channel_; }
  static void set_channel(uint8_t channel) { channel_ = channel; }

  static uint8_t bpm() { return bpm_; }
  static void set_bpm(uint8_t bpm) { bpm_ = bpm; UpdateClock(); }

  static uint8_t clock_rate() { return clock_rate_; }
  static void set_clock_rate(uint8_t clock_rate) { clock_rate_ = clock_rate; UpdatePrescaler(); }

  static uint8_t clock_mode() { return clock_mode_; }
  static void set_clock_mode(uint8_t clock_mode) { clock_mode_ = clock_mode; }

  static uint8_t clock_division() { return clock_division_; }
  static void set_clock_division(uint8_t clock_division) { clock_division_ = clock_division; }

  static uint8_t direction() { return direction_; }
  static void set_direction(uint8_t direction) { direction_ = direction; }

  static uint8_t groove_template() { return groove_template_; }
  static void set_groove_template(uint8_t groove_template) { groove_template_ = groove_template; UpdateClock(); }

  static uint8_t groove_amount() { return groove_amount_; }
  static void set_groove_amount(uint8_t groove_amount) { groove_amount_ = groove_amount; UpdateClock(); }

  static uint8_t root_note() { return root_note_; }
  static void set_root_note(uint8_t root_note) { root_note_ = root_note; }

  static uint8_t link_mode() { return link_mode_; }
  static void set_link_mode(uint8_t link_mode) { link_mode_ = link_mode; }

  static uint8_t cc1_numb() { return cc1_numb_; }
  static void set_cc1_numb(uint8_t cc1_numb) { cc1_numb_ = cc1_numb; }

  static uint8_t cc2_numb() { return cc2_numb_; }
  static void set_cc2_numb(uint8_t cc2_numb) { cc2_numb_ = cc2_numb; }

  static uint8_t steps_forward() { return steps_forward_; }
  static void set_steps_forward(uint8_t steps_forward) { steps_forward_ = steps_forward; }

  static uint8_t steps_backward() { return steps_backward_; }
  static void set_steps_backward(uint8_t steps_backward) { steps_backward_ = steps_backward; }

  static uint8_t steps_replay() { return steps_replay_; }
  static void set_steps_replay(uint8_t steps_replay) { steps_replay_ = steps_replay; }

  static uint8_t steps_interval() { return steps_interval_; }
  static void set_steps_interval(uint8_t steps_interval) { steps_interval_ = steps_interval; }

  static uint8_t steps_repeat() { return steps_repeat_; }
  static void set_steps_repeat(uint8_t steps_repeat) { steps_repeat_ = steps_repeat; }

  static uint8_t steps_skip() { return steps_skip_; }
  static void set_steps_skip(uint8_t steps_skip) { steps_skip_ = steps_skip; }

  static uint8_t cv_mode(uint8_t index) { return cv_mode_[index] & CVMODE_MASK; }
  static void set_cv_mode(uint8_t index, uint8_t value) { cv_mode_[index] = (value & CVMODE_MASK) | (cv_mode_[index] & CVMODE_OFFSET); }

  static uint8_t cv_mode_offset(uint8_t index) { return cv_mode_[index] & CVMODE_OFFSET; }
  static void set_cv_mode_offset(uint8_t index, uint8_t value) { SETFLAGTO(cv_mode_[index], CVMODE_OFFSET, value); }

  static uint8_t gate_mode(uint8_t index) { return gate_mode_[index] & GATEMODE_MASK; }
  static void set_gate_mode(uint8_t index, uint8_t value) { gate_mode_[index] = (value & GATEMODE_MASK) | (gate_mode_[index] & GATEMODE_INVERT); }

  static uint8_t gate_mode_invert(uint8_t index) { return gate_mode_[index] & GATEMODE_INVERT; }
  static void set_gate_mode_invert(uint8_t index, uint8_t value) { SETFLAGTO(gate_mode_[index], GATEMODE_INVERT, value); }

  static uint8_t seq_switch_mode() { return seq_switch_mode_; }
  static void set_seq_switch_mode(uint8_t seq_switch_mode) { seq_switch_mode_ = seq_switch_mode; }

  static uint8_t running() { return running_; }
  static uint8_t recording() { return recording_; }
  static uint8_t last_note() { return last_note_; }

  static uint8_t step_hold() { return step_hold_; }
  static void set_step_hold(uint8_t step_hold) { step_hold_ = step_hold; }

  static uint8_t manual_step_selected() { return manual_step_selected_; }
  static void set_manual_step_selected(uint8_t manual_step_selected) { manual_step_selected_ = manual_step_selected; }

  static uint8_t prog_change_flags() { return prog_change_flags_; }
  static void set_prog_change_flags(uint8_t prog_change_flags) { prog_change_flags_ = prog_change_flags; }

  static uint8_t ctrl_change_flags() { return ctrl_change_flags_; }
  static void set_ctrl_change_flags(uint8_t ctrl_change_flags) { ctrl_change_flags_ = ctrl_change_flags; }

  static uint8_t strobe_width() { return strobe_width_; }
  static void set_strobe_width(uint8_t strobe_width) { strobe_width_ = strobe_width; UpdateStrobeWidth(); }

  static uint8_t last_received_note() { return last_received_note_; }
  static uint8_t last_received_cc() { return last_received_cc_; }

  static uint8_t request_set_seq() { return request_set_seq_; }

  static uint8_t tick() { return tick_; }
  static void set_tick(uint8_t tick) { tick_ = tick; }

  // Operations

  static void SetSeq(uint8_t seq);

  static void Stop();
  static void Start();
  static void ToggleRun();
  
  static void StopRecording();
  static void StartRecording();
  static void ToggleRecording();

  static void AdvanceClockDivision();
  static void ToggleStepHold();
  static void CopyTo(uint8_t seq);
  
  static void SendStep(uint8_t step);
  static void SendStep() { SendStep(step_); }

  static void SendNote(uint8_t note, uint8_t velo);
  static void SendNoteOff(uint8_t note, uint8_t velo);

  static void InitSeqInfo();

  static void InitSeq(uint8_t seq) { data_[seq].Init(); }
  static void InitSeq() { InitSeq(seq_); }

  static void RandSeq(uint8_t seq) { data_[seq].Rand(); }
  static void RandSeq() { RandSeq(seq_); }

  static void RotLSeq(uint8_t seq) { data_[seq].RotL(); }
  static void RotLSeq();

  static void RotRSeq(uint8_t seq) { data_[seq].RotR(); }
  static void RotRSeq();

  static void SetFirstStep(uint8_t step);

  static void SwapSteps(uint8_t seq, uint8_t step1, uint8_t step2) { 
    data_[seq].SwapSteps(step1, step2); }

  static void SwapSteps(uint8_t step1, uint8_t step2) { 
    SwapSteps(seq_, step1, step2); }

  static uint16_t CalcCrc16(uint16_t crc16 = 0xffff);
  
  static void LoadSeqData(uint8_t seq, const SeqData& data);
  static void LoadSeqData(const SeqData& data) { LoadSeqData(seq_, data); }

  static void SaveToIntEeprom(uint8_t* p);
  static void LoadFromIntEeprom(const uint8_t* p);

  static void SaveToStorage(uint8_t slot);
  static void SaveToStorage() { SaveToStorage(slot_); }
  static void LoadFromStorage(uint8_t slot);

  static void SaveSeqInfo(SeqInfo& info);
  static void LoadSeqInfo(const SeqInfo& info);
  
  static void GetSeqName(uint8_t* name) { memcpy(name, name_, kNameLength); }
  static void SetSeqName(const uint8_t* name) { memcpy(name_, name, kNameLength); }
  static uint8_t HasSeqName();

  static void CopySeqData(uint8_t seq, SeqData& data) { memcpy(&data, &data_[seq], sizeof(SeqData)); }
  static void CopySeqData(SeqData& data) { CopySeqData(seq_, data); }
  
  static void FixSeqName(uint8_t* name);

  static uint8_t IsFirstStep() { return IsFirstStep(step_); }
  static uint8_t IsFirstStep(uint8_t step);

  static uint8_t Verify(uint8_t value, uint8_t min, uint8_t max, uint8_t def);

 private: friend class StateData;
  static void Tick();
  static void AdvanceStep();
  static void AdvanceStep(uint8_t direction);
  static void UpdateClock();
  static void UpdatePrescaler();
  static void UpdateStrobeWidth();
  static void RecordStep(uint8_t note, uint8_t velocity);
  static uint8_t CheckAllSkipped();
  static uint8_t CountAvailableSteps();
  static uint8_t GetRunningDirection(uint8_t reversed);
  static uint8_t HandleCC(uint8_t channel, uint8_t controller, uint8_t value);
  
  // Persistent data, see State.h/.cc
  static SeqData data_[4];

  static uint8_t name_[kNameLength];
  static uint8_t slot_;
  static uint8_t channel_;
  static uint8_t bpm_;
  static uint8_t clock_rate_;
  static uint8_t clock_mode_;
  static uint8_t clock_division_;  
  static uint8_t direction_;
  static uint8_t groove_template_;
  static uint8_t groove_amount_;
  static uint8_t root_note_;
  static uint8_t link_mode_;
  static uint8_t cc1_numb_;
  static uint8_t cc2_numb_;
  static uint8_t steps_forward_;
  static uint8_t steps_backward_;
  static uint8_t steps_replay_;
  static uint8_t steps_interval_;
  static uint8_t steps_repeat_;
  static uint8_t steps_skip_;

  static uint8_t cv_mode_[4];
  static uint8_t gate_mode_[4];

  static uint8_t seq_switch_mode_;

  static const uint8_t kSeqSaveSize = sizeof(SeqInfo) + sizeof(data_);
  
  // Volatile data
  static uint8_t seq_;
  static uint8_t tick_;
  static uint8_t step_;
  static uint8_t running_;
  static uint8_t recording_;
  static uint8_t step_hold_;  
  static uint8_t last_note_;
  static uint8_t pendulum_backward_;
  static uint8_t midi_clock_prescaler_;
  static uint8_t clock_division_counter_;
  static uint8_t steps_forward_counter_;
  static uint8_t steps_replay_counter_;
  static uint8_t steps_interval_counter_;
  static uint8_t steps_repeat_counter_;
  static uint8_t steps_skip_counter_;
  static uint8_t saved_replay_step_;
  static uint8_t available_steps_;
  static uint8_t next_step_;
  static uint8_t next_seq_;
  static uint8_t last_recorded_step_;
  static uint8_t last_recorded_seq_;
  static uint8_t last_legato_note_;
  static uint8_t manual_step_selected_;
  static uint8_t bank_select_msb_;
  static uint8_t bank_select_lsb_;
  static uint8_t prog_change_flags_;
  static uint8_t ctrl_change_flags_;
  static uint8_t strobe_width_;
  static uint8_t last_received_note_;
  static uint8_t last_received_cc_;
  static uint8_t request_set_seq_;

  static NoteStack<16> note_stack_;

  static void VerifySeqData(uint8_t seq) { data_[seq].Verify(); }
  static void VerifySeqData();

  DISALLOW_COPY_AND_ASSIGN(Seq);
};

extern Seq seq;

} // namespace midialf

#endif // MIDIALF_SEQ_H_
