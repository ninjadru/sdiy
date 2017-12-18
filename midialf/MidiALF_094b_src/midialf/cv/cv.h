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
// CV extention board DACs.

#ifndef MIDIALF_CV_H_
#define MIDIALF_CV_H_

#ifdef ENABLE_CV_OUTPUT

#include "midialf/midialf.h"
#include "midialf/seq.h"

namespace midialf {

static const int kCVCount = 4;

class CV {
 public:
  CV() {}
  static void Init();
  static void Tick();

  static void set(uint8_t index, uint16_t value) {
    if (cv_value_[index] != value) {
      cv_value_[index] = value;
      cv_dirty_|= (1 << index);
    }
  }

  static void set7F(uint8_t index, uint8_t value) {
    set(index, value << 5);
  }

  static uint16_t get(uint8_t index) {
    return cv_value_[index];
  }

  static uint8_t dirty() {
    return cv_dirty_;
  }

  static uint8_t dirty(uint8_t index) {
    return cv_dirty_ & (1 << index);
  }

  static void set_dirty(uint8_t dirty) {
    cv_dirty_ = dirty;
  }

  static void SendMode(uint8_t mode, uint8_t value);

  static void SendNote(uint8_t value);
  static void SendVelo(uint8_t value) { SendMode(CVMODE_VELO, value); }
  static void SendCC1(uint8_t value) { SendMode(CVMODE_CC1, value); }
  static void SendCC2(uint8_t value) { SendMode(CVMODE_CC2, value); }
  static void SendLFO1(uint8_t value) { SendMode(CVMODE_LFO1, value); }
  static void SendLFO2(uint8_t value) { SendMode(CVMODE_LFO2, value); }
  static void SendATch(uint8_t value) { SendMode(CVMODE_ATCH, value); }
  static void SendPBnd(uint16_t value);

  static int16_t tune(uint8_t note) { return cv_tune_[note]; }
  static void set_tune(uint8_t note, int16_t value) { cv_tune_[note] = value; }

  static void InterpolateTune(uint8_t note);

  static void ClearTune();

  static void AdjustTune(uint8_t note, int16_t value);

  static uint8_t* GetSaveTuneAddr();
  static void SaveTune();
  static void LoadTune();

private:

  static uint16_t cv_value_[kCVCount];
  static uint8_t cv_dirty_;
  static int16_t cv_tune_[128];

  DISALLOW_COPY_AND_ASSIGN(CV);
};

extern CV cv;

} // namespace midialf

#endif // #ifdef ENABLE_CV_OUTPUT

#endif // MIDIALF_CV_H_
