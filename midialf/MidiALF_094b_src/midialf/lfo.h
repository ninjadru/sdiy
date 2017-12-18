// Copyright 2012 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// Based on MIDIPal code by Olivier Gillet (ol.gillet@gmail.com)
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
// CC LFO class.

#ifndef MIDIALF_LFO_H_
#define MIDIALF_LFO_H_

#include "midialf/midialf.h"

namespace midialf {

enum LfoSync {
  LFO_SYNC_FREE_RUNNING,
  LFO_SYNC_START,
  LFO_SYNC_SEQSTART,
  LFO_SYNC_STEP,
};

struct LfoData {
  uint8_t cc_number;
  uint8_t amount;
  uint8_t center;
  uint8_t waveform;
  uint8_t rate;
  uint8_t sync;
};

const uint8_t kNumLfos = 2;

const uint8_t kDefLfoResolution = 0;

const uint8_t kDefLfo1CCNumber = 7;   // Main volume
const uint8_t kDefLfo1CCNumber2 = 0;
const uint8_t kDefLfo1Amount = 126;
const uint8_t kDefLfo1Amount2 = 63;
const uint8_t kDefLfo1Center = 63;
const uint8_t kDefLfo1Waveform = 0;
const uint8_t kDefLfo1Rate = 3;  // 1 Bar
const uint8_t kDefLfo1Sync = LFO_SYNC_FREE_RUNNING;

const uint8_t kDefLfo2CCNumber = 10;   // Pan
const uint8_t kDefLfo2CCNumber2 = 0;
const uint8_t kDefLfo2Amount = 126;
const uint8_t kDefLfo2Amount2 = 63;
const uint8_t kDefLfo2Center = 63;
const uint8_t kDefLfo2Waveform = 0;
const uint8_t kDefLfo2Rate = 3;  // 1 Bar
const uint8_t kDefLfo2Sync = LFO_SYNC_FREE_RUNNING;

const uint8_t kNumLfoWaveforms = 19;
const uint8_t kNumLfoRates = 18;

class SeqInfo;

class Lfo {
 public:
  Lfo() { }

  static void Init();

  static void UpdatePrescaler();

  static void Stop();
  static void Start();
  static void Tick();

  static void OnStep();
  
  static void SetLfoData(const LfoData* data);

  static uint16_t CalcCrc16(uint16_t crc16);

  static void SaveLfoInfo(SeqInfo& info);
  static void LoadLfoInfo(const SeqInfo& info);

  static uint8_t resolution() { return resolution_; }
  static void set_resolution(uint8_t resolution) { resolution_ = resolution; UpdatePrescaler(); }

  static uint8_t cc_number(uint8_t lfo) { return data_[lfo].cc_number; }
  static uint8_t amount(uint8_t lfo) { return data_[lfo].amount; }
  static uint8_t center(uint8_t lfo) { return data_[lfo].center; }
  static uint8_t waveform(uint8_t lfo) { return data_[lfo].waveform; }
  static uint8_t rate(uint8_t lfo) { return data_[lfo].rate; }
  static uint8_t sync(uint8_t lfo) { return data_[lfo].sync; }

  static void set_cc_number(uint8_t lfo, uint8_t value) { data_[lfo].cc_number = value; }
  static void set_amount(uint8_t lfo, uint8_t value) { data_[lfo].amount = value; }
  static void set_center(uint8_t lfo, uint8_t value) { data_[lfo].center = value; }
  static void set_waveform(uint8_t lfo, uint8_t value) { data_[lfo].waveform = value; }
  static void set_rate(uint8_t lfo, uint8_t value) { data_[lfo].rate = value; UpdatePrescaler(); }
  static void set_sync(uint8_t lfo, uint8_t value) { data_[lfo].sync = value; }

 private:
  static uint8_t Verify(uint8_t value, uint8_t min, uint8_t max, uint8_t def);

  static uint8_t resolution_;
  static LfoData data_[kNumLfos];

  static uint16_t phase_[kNumLfos];
  static uint16_t phase_increment_[kNumLfos];
  
  static uint8_t tick_;
  static uint8_t midi_clock_prescaler_;
  static uint8_t running_;
  
  DISALLOW_COPY_AND_ASSIGN(Lfo);
};

extern Lfo lfo;

} // namespace midialf

#endif // MIDIALF_LFO_H_
