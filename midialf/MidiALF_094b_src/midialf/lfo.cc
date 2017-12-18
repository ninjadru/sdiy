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

#include "midialf/lfo.h"
#include "midialf/note_duration.h"
#include "midialf/clock.h"
#include "midialf/seq.h"
#include "midialf/ui.h"
#ifdef ENABLE_CV_OUTPUT
#include "midialf/cv/cv.h"
#include "midialf/cv/port.h"
#endif

#include "avrlib/op.h"
#include "avrlib/random.h"

#include <util/crc16.h>

namespace midialf {

using namespace avrlib;

/* extern */
Lfo lfo;

/* <static> */
uint8_t Lfo::resolution_ = kDefLfoResolution;
LfoData Lfo::data_[kNumLfos];

uint16_t Lfo::phase_[kNumLfos];
uint16_t Lfo::phase_increment_[kNumLfos];

uint8_t Lfo::tick_;
uint8_t Lfo::midi_clock_prescaler_;
uint8_t Lfo::running_;
/* </static> */

/* static */
void Lfo::Init() {
}

/* static */
void Lfo::SetLfoData(const LfoData* data) {
  memcpy(data_, data, sizeof(data_));
}

/* static */
void Lfo::UpdatePrescaler() {
  midi_clock_prescaler_ = NoteDuration::GetMidiClockTicks(resolution_);
#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("Lfo::UpdatePrescaler: resolution_=%u midi_clock_prescaler_=%u\n", resolution_, midi_clock_prescaler_);
#endif
  uint16_t factor = midi_clock_prescaler_;
  for (uint8_t i = 0; i < kNumLfos; ++i) {
    phase_increment_[i] = ResourcesManager::Lookup<uint16_t, uint8_t>(
      lut_res_increments, data_[i].rate) * factor;
  }
}

/* static */
void Lfo::OnStep() {
#ifdef MIDIOUT_DEBUG_OUTPUT  
  //printf("Lfo::OnStep: step=%d\n", seq.step());
#endif
  // Sync lfos
  for (uint8_t i = 0; i < kNumLfos; ++i) {
    if (data_[i].sync == LFO_SYNC_STEP || (data_[i].sync == LFO_SYNC_SEQSTART && seq.IsFirstStep())) {
      phase_[i] = 0;
    }
  }
}

/* static */
void Lfo::Stop() {
  if (!running_)
    return;

  running_ = 0;
}

/* static */
void Lfo::Start() {
  if (running_)
    return;

  tick_ = midi_clock_prescaler_ - 1;
  running_ = 1;

  // Sync lfos
  for (uint8_t i = 0; i < kNumLfos; ++i) {
    if (data_[i].sync == LFO_SYNC_START) {
      phase_[i] = 0;
    }
  }
}

/* static */
void Lfo::Tick() {
  ++tick_;
  if (tick_ >= midi_clock_prescaler_) {
    tick_ = 0;
    for (uint8_t i = 0; i < kNumLfos; ++i) {
      // Check if no waveform and skip
      if (data_[i].waveform == 0)
        continue;

#ifdef ENABLE_CV_OUTPUT
      if (phase_[i] < phase_increment_[i]) {
        port.SendLFO(i);
      }
#endif
      // Calculate phase value
      uint8_t value;
      uint8_t skip = 0;
      if (data_[i].waveform == kNumLfoWaveforms - 1) {
        if (phase_[i] < phase_increment_[i]) {
          value = Random::GetByte();
        } else {
          skip = 1;
        }
      } else {
        uint16_t offset = U8U8Mul(data_[i].waveform - 1, 129);
        value = InterpolateSample(
            wav_res_lfo_waveforms + offset,
            phase_[i] >> 1);
      }

      phase_[i] += phase_increment_[i];

      if (!skip) {
        int16_t scaled_value = static_cast<int16_t>(data_[i].center) + 
            S8S8MulShift8((data_[i].amount - 63) << 1, value - 128);
        scaled_value = Clip(scaled_value, 0, 127);
#ifdef MIDIOUT_DEBUG_OUTPUT  
        //printf("Lfo::Tick: scaled_value=%d\n", scaled_value);
#endif
        if (data_[i].cc_number) {
          seq.Send3(
              0xb0 | seq.channel(),
              data_[i].cc_number,
              scaled_value & 0x7f);
        }
#ifdef ENABLE_CV_OUTPUT
        switch (i) {
          case 0: cv.SendLFO1(scaled_value & 0x7f); break;
          case 1: cv.SendLFO2(scaled_value & 0x7f); break;
        }
#endif
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Save/Load lfo info rroutines

/* static */
uint16_t Lfo::CalcCrc16(uint16_t crc16) {
  crc16 = _crc16_update(crc16, resolution_);

  uint8_t* p = (uint8_t*)&data_[0];
  for (uint16_t cb = sizeof(data_); cb--; ) {
    crc16 = _crc16_update(crc16, *p++);
  }

  return crc16;
}

/* static */
void Lfo::SaveLfoInfo(SeqInfo& info) {
  info.lfo_resolution_ = resolution_;
  memcpy(info.lfo_data_, data_, sizeof(info.lfo_data_));
}

/* static */
void Lfo::LoadLfoInfo(const SeqInfo& info) {
  resolution_ = Verify(info.lfo_resolution_, 0, kNoteDurationCount - 1, kDefLfoResolution);

  data_[0].cc_number = Verify(info.lfo_data_[0].cc_number, 0, 127, kDefLfo1CCNumber);
  data_[0].amount = Verify(info.lfo_data_[0].amount, 0, 126, kDefLfo1Amount);
  data_[0].center = Verify(info.lfo_data_[0].center, 0, 127, kDefLfo1Center);
  data_[0].waveform = Verify(info.lfo_data_[0].waveform, 0, kNumLfoWaveforms - 1, kDefLfo1Waveform);
  data_[0].rate = Verify(info.lfo_data_[0].rate, 0, kNumLfoRates - 1, kDefLfo1Rate);
  data_[0].sync = Verify(info.lfo_data_[0].sync, 0, LFO_SYNC_START, kDefLfo1Sync);

  data_[1].cc_number = Verify(info.lfo_data_[1].cc_number, 0, 127, kDefLfo2CCNumber);
  data_[1].amount = Verify(info.lfo_data_[1].amount, 0, 126, kDefLfo2Amount);
  data_[1].center = Verify(info.lfo_data_[1].center, 0, 127, kDefLfo2Center);
  data_[1].waveform = Verify(info.lfo_data_[1].waveform, 0, kNumLfoWaveforms - 1, kDefLfo2Waveform);
  data_[1].rate = Verify(info.lfo_data_[1].rate, 0, kNumLfoRates - 1, kDefLfo2Rate);
  data_[1].sync = Verify(info.lfo_data_[1].sync, 0, LFO_SYNC_START, kDefLfo2Sync);
}

/* static */
uint8_t Lfo::Verify(uint8_t value, uint8_t min, uint8_t max, uint8_t def)
{
  return value >= min && value <= max ? value : def; 
}

} // namespace midialf
