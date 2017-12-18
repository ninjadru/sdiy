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
// Randomize sequence page.

#include "midialf/ui_pages/randomize_page.h"

#include "avrlib/random.h"

#include <avr/eeprom.h>

namespace midialf {

uint8_t EEMEM eememScale = kDefScale;

/* static */
uint8_t RandomizePage::scale_;

/* static */
uint8_t RandomizePage::name_[kScaleNameSize];

/* static */
const prog_EventHandlers RandomizePage::event_handlers_ PROGMEM = {
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
void RandomizePage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  Random::Seed((uint16_t)milliseconds());
  if (!scale_) {
    uint8_t scale = eeprom_read_byte(&eememScale);
    SetScale(scale < Scale::count() ? scale : kDefScale);
  }
}

/* static */
void RandomizePage::OnQuit(UiPageIndex nextPage) {
  SaveState();

  // Check if we're going to save sequence and there is no name yet and if so,
  // assume scale name
  if (nextPage == PAGE_SAVE && !seq.HasSeqName()) {
    seq.SetSeqName(name_);
  }
}

/* static */
uint8_t RandomizePage::OnIncrement(uint8_t id, int8_t value) {
  // ENCA advances all notes within scale or octave if shifted
  if (id == ENCODER_A) {
    if (value == -1 || value == 1) {
      for (uint8_t n = 0; n < kNumSteps; n++) {
        ScaleStep(n, value > 0);
      }
    } else {
      value = Ui::FixOctaveIncrement(value);
      for (uint8_t n = 0; n < kNumSteps; n++) {
        uint8_t note = seq.note(n);
        seq.set_note(n, Clamp7F(static_cast<int16_t>(note) + value));
      }
    }
    return 1;
  }
  // ENCB selects scale
  if (id == ENCODER_B) {
    SetScale(Clamp(static_cast<int16_t>(scale_) + value, 0, Scale::count() - 1));
    return 1;
  }
  // ENCx advances note within scale or octave if shifted
  if (id >= ENCODER_1 && id <= ENCODER_8) {
    uint8_t step = id - ENCODER_1;
    if (value == -1 || value == 1) {
      ScaleStep(step, value > 0);
    } else {
      uint8_t note = seq.note(step);
      value = Ui::FixOctaveIncrement(value);
      seq.set_note(step, Clamp7F(static_cast<int16_t>(note) + value));
    }
    SendStep(step);
    return 1;
  }
  return 0;
}

/* static */
uint8_t RandomizePage::OnClick(uint8_t id, uint8_t value) {
  // ENCA cancels mode or resets notes to root if shifted
  if (id == ENCODER_A) {
    switch (value) {
      case CLICK:
        Ui::ShowLastPage(); 
        return 1;
      case CLICK_SHIFTED:
        ResetNotes(); 
        return 1;
    }
  } else

  // ENCB randomizes all step values or order if shifted
  if (id == ENCODER_B) {
    switch (value) {
      case CLICK:
        for (uint8_t n = 0; n < kNumSteps; n++) {
          RandStep(n);
        }
        return 1;
      case CLICK_SHIFTED:
        ShuffleOrder();
        return 1;
    }
  }
  return 0;
}

/* static */
uint8_t RandomizePage::OnSwitch(uint8_t id, uint8_t value) {
  // Step switch randomizes step
  if (id == SWITCH && value < kNumSteps) {
    RandStep(value);
    return 1;
  }
  return 0;
}

/* static */
uint8_t RandomizePage::OnIdle() {
  SaveState();
  return 0;
}

/* static */
void RandomizePage::UpdateScreen() {
  DrawSeparators();
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  for (uint8_t n = 0; n < kNumSteps; n++) {
    uint8_t x = cell_pos(n);
    Ui::PrintNote(&line1[x], seq.note(n));
  }

  Ui::PrintNumb(line2, 1 + scale_);
  memcpy(&line2[4], name_, kScaleNameSize);
}

/* static */
void RandomizePage::UpdateLeds() {
}

/* static */
void RandomizePage::SetScale(uint8_t scale) {
  scale_ = scale;
  Scale::GetScaleName(scale, name_);
}

/* static */
void RandomizePage::ScaleStep(uint8_t step, uint8_t up) {
  uint8_t note = seq.note(step);
  if (up) {
    note = Scale::GetNextScaledNote(scale_, note);
  } else { 
    note = Scale::GetPrevScaledNote(scale_, note);
  }
  seq.set_note(step, note);
}

/* static */
void RandomizePage::RandStep(uint8_t step) {
  uint8_t note = seq.root_note();

  // Normalize note
  uint8_t octave = 0;
  while(note >= 12) {
    note -= 12;
    ++octave;
  }

  // Get random note
  uint8_t random_note;
  do  {
    random_note = Scale::GetScaledNote(scale_, Random::GetByte() & 0x7f);
    while(random_note >= 12) {
      random_note -= 12;
    }
  } while (random_note == note);

  // Up/down an octave
  static const uint8_t change_probability = 64;
  uint8_t random_byte = Random::GetByte();
  if (random_byte < change_probability) {
    if (octave > 2) octave--; 
  } else 
  if (random_byte > 256 - change_probability) {
    if (octave < 8) octave++; 
  }

  // Restore octave
  while (octave > 0) {
    random_note += 12;
    --octave;
  }

  seq.set_note(step, random_note);
}

/* static */
void RandomizePage::ShuffleOrder() {
  for (uint8_t n = 0; n < 32; n++) {
    uint8_t random_byte = Random::GetByte();
    uint8_t step1 = random_byte & 0x7;
    uint8_t step2 = U8ShiftRight4(random_byte) & 0x7;
    seq.SwapSteps(step1, step2);
  }
}

/* static */
void RandomizePage::ResetNotes() {
  for (uint8_t n = 0; n < kNumSteps; n++) {
    seq.set_note(n, seq.root_note());
  }
}

/* static */
void RandomizePage::SaveState() {
  // Update scale stored in internal eeprom
  if (scale_ != eeprom_read_byte(&eememScale)) {
    eeprom_write_byte(&eememScale, scale_);
  }
}

} // namespace midialf
