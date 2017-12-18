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

#ifdef ENABLE_CV_OUTPUT

#include "midialf/cv/cv.h"
#include "midialf/cv/port.h"

#include <avr/eeprom.h>

namespace midialf {

using namespace avrlib;

/* extern */
CV cv;

/* <static> */
uint16_t CV::cv_value_[kCVCount];
uint8_t CV::cv_dirty_ = 0xff;
int16_t CV::cv_tune_[128] = { 0 };
/* </static> */

static SPIInterface spi;

#define DAC1_SS_LOW  { SSSelector::Write(1); SSEnable::Low(); }
#define DAC1_SS_HIGH { SSEnable::High(); SSSelector::Write(0); }

#define DAC2_SS_LOW  { SSSelector::Write(2); SSEnable::Low(); }
#define DAC2_SS_HIGH { SSEnable::High(); SSSelector::Write(0); }

const prog_uint16_t note2dac[128] PROGMEM = {
   0,  34,  68, 102, 137, 171, 205, 239, 273, 307, 341, 375, 410, 444, 478, 512,
 546, 580, 614, 649, 683, 717, 751, 785, 819, 853, 887, 922, 956, 990,1024,1058,
1092,1126,1161,1195,1229,1263,1297,1331,1365,1399,1434,1468,1502,1536,1570,1604,
1638,1673,1707,1741,1775,1809,1843,1877,1911,1946,1980,2014,2048,2082,2116,2150,
2185,2219,2253,2287,2321,2355,2389,2423,2458,2492,2526,2560,2594,2628,2662,2697,
2731,2765,2799,2833,2867,2901,2935,2970,3004,3038,3072,3106,3140,3174,3209,3243,
3277,3311,3345,3379,3413,3447,3482,3516,3550,3584,3618,3652,3686,3721,3755,3789,
3823,3857,3891,3925,3959,3994,4028,4062,4096,4130,4164,4198,4233,4267,4301,4335,
};

/* static */
void CV::Init() {
  spi.Init();
  SSSelector::set_mode(DIGITAL_OUTPUT);
  SSEnable::set_mode(DIGITAL_OUTPUT);
  SSEnable::High();
  LoadTune();
}

/* static */
void CV::Tick() {
  
  if (!cv_dirty_)
    return;

  Word dac_value;

  DisableInterrupts di;

  // This takes up to ~14us (2us overhead plus 3us per cv update)

  // bit15: DAC select (~A/B)
  // bit14: BUF Vref buffer control (1 = buffered)
  // bit13: output gain select (0=2x, 1=1x)
  // bit12: shutdown if 0

  // CV1
  if (cv_dirty_ & 0x01) {
    dac_value.value = 0x3000 | cv_value_[0];
    DAC1_SS_LOW;
    spi.Send(dac_value.bytes[1]);
    spi.Send(dac_value.bytes[0]);
    DAC1_SS_HIGH;
  }

  // CV2
  if (cv_dirty_ & 0x02) {
    dac_value.value = 0xb000 | cv_value_[1];
    DAC1_SS_LOW;
    spi.Send(dac_value.bytes[1]);
    spi.Send(dac_value.bytes[0]);
    DAC1_SS_HIGH;
  }

  // CV3
  if (cv_dirty_ & 0x04) {
    dac_value.value = 0x3000 | cv_value_[2];
    DAC2_SS_LOW;
    spi.Send(dac_value.bytes[1]);
    spi.Send(dac_value.bytes[0]);
    DAC2_SS_HIGH;
  }
  
  // CV4
  if (cv_dirty_ & 0x08) {
    dac_value.value = 0xb000 | cv_value_[3];
    DAC2_SS_LOW;
    spi.Send(dac_value.bytes[1]);
    spi.Send(dac_value.bytes[0]);
    DAC2_SS_HIGH;
  }

  cv_dirty_ = 0;
}

/* static */
void CV::SendMode(uint8_t mode, uint8_t value)
{
  for (uint8_t n = 0; n < kCVCount; n++) {
    if (seq.cv_mode(n) == mode) {
      set7F(n, value);
    }
  }

  // Make sure CV offset port is up to date
  port.UpdateCvOffset();
}

/* static */
void CV::SendNote(uint8_t value)
{
  for (uint8_t n = 0; n < kCVCount; n++) {
    if (seq.cv_mode(n) == CVMODE_NOTE) {
      int16_t dacvalue = ResourcesManager::Lookup<uint16_t, uint8_t>(note2dac, value) + cv_tune_[value];
      if (dacvalue < 0) dacvalue = 0; else
      if (dacvalue > 0x0fff) dacvalue = 0x0fff;
      set(n, dacvalue);
    }
  }

  // Make sure CV offset port is up to date
  port.UpdateCvOffset();
}

/* static */
void CV::SendPBnd(uint16_t value) {
  for (uint8_t n = 0; n < kCVCount; n++) {
    if (seq.cv_mode(n) == CVMODE_PBND) {
      int16_t dacvalue = value >> 2;
      if (dacvalue < 0) dacvalue = 0; else
      if (dacvalue > 0x0fff) dacvalue = 0x0fff;
      set(n, dacvalue);
    }
  }

  // Make sure CV offset port is up to date
  port.UpdateCvOffset();
}

/* static */
void CV::InterpolateTune(uint8_t note) {
  if (note == 0 || note == 0x7f)
    return;

  int16_t t1 = 0;
  uint8_t n1 = note - 1;
  for ( ; ; n1--) {
    if (cv_tune_[n1] != 0) {
      t1 = cv_tune_[n1];
      break;
    }
    if (n1 == 0)
      break;
  }

  int16_t t2 = 0;
  uint8_t n2 = note + 1;
  for ( ; ; n2++) {
    if (cv_tune_[n2] != 0) {
      t2 = cv_tune_[n2];
      break;
    }
    if (n2 == 0x7f)
      break;
  }
  
  int16_t x1 = static_cast<int16_t>(note) - n1;
  int16_t x2 = static_cast<int16_t>(n2) - n1;

  cv_tune_[note] = t1 + (x1 * (t2 - t1)) / x2;
}

/* static */
void CV::AdjustTune(uint8_t note, int16_t value) {
  int16_t dacvalue = ResourcesManager::Lookup<uint16_t, uint8_t>(note2dac, note);
  if (dacvalue + cv_tune_[note] + value < 0) {
    cv_tune_[note] = - dacvalue;
    return;
  } else
  if (dacvalue + cv_tune_[note] + value > 0x0fff) {
    cv_tune_[note] = 0x0fff - dacvalue;
    return;
  }

  cv_tune_[note]+= value;
}

/* static */
uint8_t* CV::GetSaveTuneAddr() {
  return (uint8_t*)(2048 - sizeof(cv_tune_));
}

/* static */
void CV::ClearTune() {
  for (uint8_t n = 0; n < numbof(cv_tune_); n++) {
    cv_tune_[n] = 0;
  }
}

/* static */
void CV::SaveTune() {
  eeprom_update_block(&cv_tune_[0], GetSaveTuneAddr(), sizeof(cv_tune_));
}

/* static */
void CV::LoadTune() {
  eeprom_read_block(&cv_tune_[0], GetSaveTuneAddr(), sizeof(cv_tune_));

  // Check if uninitialized and clear
  for (uint8_t n = 0; n < numbof(cv_tune_); n++) {
    if (cv_tune_[n] != 0xffff)
      return;
  }
  ClearTune(); 
}

} // namespace midialf

#endif // #ifdef ENABLE_CV_OUTPUT
