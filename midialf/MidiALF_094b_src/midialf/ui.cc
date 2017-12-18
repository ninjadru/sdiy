// Copyright 2012 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// Based on Ambika code by Olivier Gillet (ol.gillet@gmail.com)
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

#include "midialf/ui.h"

#include "avrlib/op.h"
#include "avrlib/string.h"

#include "midialf/seq.h"
#include "midialf/leds.h"
#include "midialf/state.h"
#include "midialf/display.h"
#include "midialf/sysex_handler.h"

#include "midialf/ui_pages/note_page.h"
#include "midialf/ui_pages/velo_page.h"
#include "midialf/ui_pages/gate_page.h"
#include "midialf/ui_pages/cc1_page.h"
#include "midialf/ui_pages/cc2_page.h"
#include "midialf/ui_pages/seq_settings_page.h"
#include "midialf/ui_pages/prg_settings_page.h"
#include "midialf/ui_pages/sys_settings_page.h"
#ifdef ENABLE_CV_OUTPUT
#include "midialf/ui_pages/ext_settings_page.h"
#include "midialf/ui_pages/tune_page.h"
#endif
#include "midialf/ui_pages/dlg_page.h"
#include "midialf/ui_pages/copy_seq_page.h"
#include "midialf/ui_pages/command_page.h"
#include "midialf/ui_pages/init_page.h"
#include "midialf/ui_pages/save_page.h"
#include "midialf/ui_pages/load_page.h"
#include "midialf/ui_pages/send_sysex_page.h"
#include "midialf/ui_pages/recv_sysex_page.h"
#include "midialf/ui_pages/randomize_page.h"
#include "midialf/ui_pages/rotate_page.h"
#include "midialf/ui_pages/lfo_page.h"

namespace midialf {

using namespace avrlib;

static const uint8_t kDblClickTime = 250;

/* <static> */
EncoderA Ui::encoderA_;
EncoderB Ui::encoderB_;
Encoders Ui::encoders_;

StepSwitches Ui::stepSwitches_;
SideSwitches Ui::sideSwitches_;

EventQueue<32> Ui::queue_;

UiPageIndex Ui::active_page_;
UiPageIndex Ui::last_page_;
EventHandlers Ui::event_handlers_;
PageInfo Ui::page_info_;

uint8_t Ui::cycle_;
uint8_t Ui::save_state_;
uint8_t Ui::inhibit_sel_raised_;
uint8_t Ui::last_click_encoder_;
uint32_t Ui::last_click_time_;
uint16_t Ui::led_blink_cycle_;
/* </static> */

/* extern */
Ui ui;

const prog_PageInfo page_registry[] PROGMEM = {
  { PAGE_NOTE,
    &NotePage::event_handlers_,
  },
  { PAGE_VELO,
    &VeloPage::event_handlers_,
  },
  { PAGE_GATE,
    &GatePage::event_handlers_,
  },
  { PAGE_CC1,
    &CC1Page::event_handlers_,
  },
  { PAGE_CC2,
    &CC2Page::event_handlers_,
  },
  { PAGE_SEQ_SETTINGS,
    &SeqSettingsPage::event_handlers_,
  },
  { PAGE_PRG_SETTINGS,
    &PrgSettingsPage::event_handlers_,
  },
  { PAGE_SYS_SETTINGS,
    &SysSettingsPage::event_handlers_,
  },
#ifdef ENABLE_CV_OUTPUT
  { PAGE_EXT_SETTINGS,
    &ExtSettingsPage::event_handlers_,
  },
#endif
  { PAGE_LFO1,
    &LfoPage::event_handlers_,
  },
  { PAGE_LFO2,
    &LfoPage::event_handlers_,
  },
  { PAGE_DLG,
    &DlgPage::event_handlers_,
  },
  { PAGE_COPYSEQ,
    &CopySeqPage::event_handlers_,
  },
  { PAGE_COMMAND,
    &CommandPage::event_handlers_,
  },
  { PAGE_INIT,
    &InitPage::event_handlers_,
  },
  { PAGE_SAVE,
    &SavePage::event_handlers_,
  },
  { PAGE_LOAD,
    &LoadPage::event_handlers_,
  },
  { PAGE_SEND_SYSEX,
    &SendSysExPage::event_handlers_,
  },
  { PAGE_RECV_SYSEX,
    &RecvSysExPage::event_handlers_,
  },
  { PAGE_RANDOMIZE,
    &RandomizePage::event_handlers_,
  },
  { PAGE_ROTATE,
    &RotatePage::event_handlers_,
  },
#ifdef ENABLE_CV_OUTPUT
  { PAGE_TUNE,
    &TunePage::event_handlers_,
  },
#endif
};

/* static */
void Ui::Init() {
  encoderA_.Init();
  encoderB_.Init();
  encoders_.Init();
  stepSwitches_.Init();
  sideSwitches_.Init();
  lcd.Init();
  display.Init();

  // Load last state unless ENCB is down
  encoderB_.Read();
  if (encoderB_.immediate_value() != 0) {
    state.Load();
  }

  Logo();

  ShowPage(PAGE_NOTE);
}

/* static */
void Ui::Logo() {
  char* line1 = display.line_buffer(0);
#ifdef ENABLE_CV_OUTPUT
  memcpy_P(&line1[0], PSTRN("MidiALF/CV v" MIDIALF_VERSION));
#else
  memcpy_P(&line1[0], PSTRN("MidiALF v" MIDIALF_VERSION));
#endif
  uint8_t x = 20;
  line1[x++] = '(';
  UnsafeItoa(sizeof(SeqInfo), 2, &line1[x]); x+= 2;
  line1[x++] = '/';
  UnsafeItoa(sizeof(SeqData) * 4, 3, &line1[x]); x+=3;
  line1[x++] = ')';
  RedrawScreen();

  // Run led check sequence

  static const prog_uint8_t xleds[] PROGMEM = {
    LED_5    | (LED_4    << 4),
    LED_6    | (LED_3    << 4),
    LED_7    | (LED_2    << 4),
    LED_8    | (LED_1    << 4),
    LED_RUN  | (LED_SEQA << 4),
    LED_SEQ  | (LED_SEQB << 4),
    LED_CLK  | (LED_SEQC << 4),
    LED_SEL  | (LED_SEQD << 4),
    LED_CLK  | (LED_SEQC << 4),
    LED_SEQ  | (LED_SEQB << 4),
    LED_RUN  | (LED_SEQA << 4),
    LED_8    | (LED_1    << 4),
    LED_7    | (LED_2    << 4),
    LED_6    | (LED_3    << 4),
  };

  for (uint8_t n = 0; n < 1; n++) {
    for (uint8_t i = 0; i < numbof(xleds); i++) {
      uint8_t byte = pgm_read_byte(&xleds[i]);
      uint8_t pixel1 = byte & 0xf;
      uint8_t pixel2 = U8ShiftRight4(byte);
      leds.set_pixel(pixel1);
      leds.set_pixel(pixel2);
      leds.Write(); _delay_ms(50);
      leds.clr_pixel(pixel1);
      leds.clr_pixel(pixel2);
    }
  }

  for (uint8_t n = Leds::max_intensity - 4; n-- > 0; ) {
    leds.set_pixel(LED_4, n);
    leds.set_pixel(LED_5, n);
    _delay_ms(25);
  }
  
  leds.Clear();
  _delay_ms(250);
}

/* static */
void Ui::Poll() {

  // 2.45KHz
  cycle_++;

  // Poll Encoder A
  { int8_t increment = encoderA_.Read();
    if (increment) {
      if (CheckShifted()) increment*= 10;
      queue_.AddEvent(CONTROL_ENCODER, ENCODER_A, increment);
    }
    if (encoderA_.clicked()) {    
      queue_.AddEvent(CONTROL_ENCODER_CLICK, ENCODER_A, GetEncoderClickValue(ENCODER_A));
    }
  }

  // Poll Encoder B
  { int8_t increment = encoderB_.Read();
    if (increment) {
      if (CheckShifted()) increment*= 10;
      queue_.AddEvent(CONTROL_ENCODER, ENCODER_B, increment);
    }

    if (encoderB_.clicked()) {    
      queue_.AddEvent(CONTROL_ENCODER_CLICK, ENCODER_B, GetEncoderClickValue(ENCODER_B));
    }
  }

  // Poll step encoders
  { encoders_.Poll();
    for (uint8_t i = 0; i < 8; ++i) {
      int8_t increment = encoders_.Read(i);
      if (increment) {
        if (CheckShifted()) increment*= 10;
        queue_.AddEvent(CONTROL_ENCODER, ENCODER_1 + i, increment);
      }
      if (encoders_.lowered(i)) {
        uint8_t id = ENCODER_1 + i;
        queue_.AddEvent(CONTROL_ENCODER_CLICK, id, GetEncoderClickValue(id));
      }
    }
  }

  // Poll switches
  if ((cycle_ & 3) == 0) {
    // 612Hz
    stepSwitches_.Read();
    sideSwitches_.Read();
    for (uint8_t i = 0; i < 8; ++i) {
      if (stepSwitches_.lowered(i)) {
        queue_.AddEvent(CONTROL_SWITCH, SWITCH + CheckShifted(), i);
      }
      if (sideSwitches_.lowered(i)) {
        if (i != SIDE_SWITCH_SEL) {
          queue_.AddEvent(CONTROL_SWITCH, SIDE_SWITCH + CheckShifted(), i);
        } else
          queue_.AddEvent(CONTROL_SWITCH, SIDE_SWITCH, i);
      } else
      if (sideSwitches_.raised(i)) {
        if (inhibit_sel_raised_ && i == SIDE_SWITCH_SEL) {
          inhibit_sel_raised_ = 0;
        } else
          queue_.AddEvent(CONTROL_SWITCH, SIDE_SWITCH_RAISED, i);
      }
    }
  }

  // Output
  lcd.Tick();
}

/* static */
void Ui::DoEvents() {
  display.Tick();
  
  uint8_t redraw = 0;
  while (queue_.available()) {
    Event e = queue_.PullEvent();
#ifdef MIDIOUT_DEBUG_OUTPUT  
    //printf("DoEvents: type=%d id=%d value=%d\n", e.control_type, e.control_id, (int8_t)e.value);
#endif
    queue_.Touch();
    HandleEvent(e);
    save_state_ = 1;
    redraw = 1;
  }
  
  if (queue_.idle_time_ms() > 1000) {
    queue_.Touch();
    if ((*event_handlers_.OnIdle)()) {
      redraw = 1;
    }
    if (save_state_ && !seq.running()) {
      save_state_ = 0;
      state.Save();
    }
  }
  
  if (redraw) {
    display.Clear();
    (*event_handlers_.UpdateScreen)();
  }
  
  UpdateSideLeds();
  UpdateStepLeds();
  (*event_handlers_.UpdateLeds)();
}

/* static */
void Ui::HandleEvent(const Event& e) {
  switch (e.control_type) {
    case CONTROL_ENCODER:
      if (!(*event_handlers_.OnIncrement)(e.control_id, e.value)) {
        HandleEncoderEvent(e);
      }
      break;
    case CONTROL_ENCODER_CLICK:
      if (!(*event_handlers_.OnClick)(e.control_id, e.value)) {
        HandleEncoderClickEvent(e);
      }
      break;
    case CONTROL_SWITCH:
      if (!(*event_handlers_.OnSwitch)(e.control_id, e.value)) {
        HandleSwitchEvent(e);
      }
      break;
    case CONTROL_REQUEST:
      {
        HandleRequestEvent(e);
      }
      break;
  }
}

/* static */
void Ui::HandleEncoderEvent(const Event& e) {
  // Cycle between primary pages on ENCB
  if (e.control_id == ENCODER_B) {
    uint8_t page = active_page_;
    if ((int8_t)e.value > 0) {
      if (page < LAST_PAGE) {
        page++;
      } else
        page = FIRST_PAGE;
    } else
    if ((int8_t)e.value < 0) {
      if (page > FIRST_PAGE) {
        page--;
      } else
        page = LAST_PAGE;
    }
    ShowPage(static_cast<UiPageIndex>(page));
  }
}

void Ui::HandleEncoderClickEvent(const Event& e) {
  if (e.value == CLICK) {
    // Copy sequence on ENCA click
    if (e.control_id == ENCODER_A) {
      ShowPage(PAGE_COPYSEQ);
    } else
    // Switch to Note page
    if (e.control_id == ENCODER_B) {
      ShowPage(FIRST_PAGE);
    } else
    // Send step note/ccs on ENCx click
    if (e.control_id >= ENCODER_1 && e.control_id <= ENCODER_8) {
      uint8_t step = e.control_id - ENCODER_1;
      seq.set_step(step);
      if (!seq.running()) {
        seq.SendStep(step);
        seq.set_manual_step_selected(1);
      }
    }
  } else

  if (e.value == DBLCLICK) {
    // Switch to Sequence Settings page
    if (e.control_id == ENCODER_B) {
      ShowPage(PAGE_SEQ_SETTINGS);
    }
  } else

  if (e.value == DBLCLICK_SHIFTED) {
    // ???
    if (e.control_id == ENCODER_B) {
      ;
    }
  }
}

void Ui::HandleSwitchEvent(const Event& e) {
  if (e.control_id == SIDE_SWITCH) {
    // Select sequence on SEQX switch
    if (e.value >= SIDE_SWITCH_SEQA && e.value <= SIDE_SWITCH_SEQD) {
      seq.SetSeq(e.value);
    } else
    // Handle right side switches
    switch (e.value) {
      case SIDE_SWITCH_RUN: seq.ToggleRun(); break;
      case SIDE_SWITCH_SEQ: ShowPage(PAGE_LOAD); break;
      case SIDE_SWITCH_CLK: seq.AdvanceClockDivision(); break;
      case SIDE_SWITCH_SEL: break;
    }
  } else
  if (e.control_id == SIDE_SWITCH_SHIFTED) {
    // Select link mode, show save page, and start recording
    switch (e.value) {
      case SIDE_SWITCH_SEQA: seq.set_link_mode(LINK_MODE_NONE); break;
      case SIDE_SWITCH_SEQB: seq.set_link_mode(LINK_MODE_16); seq.set_seq(0); break;
      case SIDE_SWITCH_SEQC: seq.set_link_mode(LINK_MODE_16); seq.set_seq(2); break;
      case SIDE_SWITCH_SEQD: seq.set_link_mode(LINK_MODE_32); break;
      case SIDE_SWITCH_RUN: seq.ToggleRecording(); break;
      case SIDE_SWITCH_SEQ: ShowPage(PAGE_SAVE); break;
      case SIDE_SWITCH_CLK: seq.ToggleStepHold(); break;
    }
  } else
  if (e.control_id == SWITCH_SHIFTED) {
    // Select ui page directly
    if (e.value >= SWITCH_1 && e.value <= SWITCH_8) {
      ShowPage(static_cast<UiPageIndex>(e.value));
    }
  } else
  if (e.control_id == SIDE_SWITCH_RAISED) {
    // Activate command mode on lone Sel
    if (e.value == SIDE_SWITCH_SEL) {
      ShowPage(PAGE_COMMAND);
    }
  }
}

/* static */
void Ui::HandleRequestEvent(const Event& e) {
  switch (e.control_id) {

    case REQUEST_SHOWPAGE:
      if (active_page_ != e.value) {
        ShowPage(static_cast<UiPageIndex>(e.value));
      } else
        UpdateScreen();
      break;

    case REQUEST_UPDATESCREEN:
      UpdateScreen();
      break;

    case REQUEST_SENDCURSEQUENCE:
      sysex_handler.SendSeq();
      break;

    case REQUEST_SENDCURPROGRAM:
      sysex_handler.SendPgm();
      break;

    case REQUEST_SENDALLPROGRAMS:
      sysex_handler.SendAll();
      break;

    case REQUEST_SENDPROGRAM:
      sysex_handler.SendPgm(e.value);
      break;
  }
}

/* static */
void Ui::UpdateSideLeds() {
  // Update sequence leds
  for (uint8_t n = 0; n < 4; n++) {
    if (n == seq.seq()) {
      leds.set_pixel(LED_SEQA + n);
    } else {
      switch (seq.link_mode()) {
        case LINK_MODE_16: leds.set_pixel(LED_SEQA + n, ((seq.seq() & 2) == (n & 2)) ? 1 : 0); break;
        case LINK_MODE_32: leds.set_pixel(LED_SEQA + n, 1); break;
        default: leds.clr_pixel(LED_SEQA + n); break;
      }
    }
  }

  if (seq.running() && seq.request_set_seq()) {
      led_blink_cycle_ = (led_blink_cycle_ + 1) & 0x1fff;
      leds.set_pixel(LED_SEQA + seq.request_set_seq() - 1, led_blink_cycle_ & 0x1000 ? 0 : 0xf);
  }

  // Update right side leds
  leds.set_pixel(LED_RUN, seq.running() ? 0xf : 0);
  
  switch (seq.clock_division()) {
    case CLOCK_DIVISION_NONE: leds.clr_pixel(LED_CLK); break;
    case CLOCK_DIVISION_X2: leds.set_pixel(LED_CLK, 0x1); break;
    case CLOCK_DIVISION_X4: leds.set_pixel(LED_CLK, 0xf); break;
  }
}

/* static */
void Ui::UpdateStepLeds() {
  for (uint8_t n = 0; n < kNumSteps; n++) {
    if (n == seq.step() && (((seq.running() || seq.recording()) && !seq.all_skipped()) || seq.manual_step_selected())) {
      uint8_t intensity = seq.manual_step_selected() && !seq.recording() ? 1 : 0xf;
      leds.set_pixel(LED_1 + n, intensity);
    } else
      leds.set_pixel(LED_1 + n, seq.recording() ? 1 : 0);
  }
}

/* static */
void Ui::ShowPage(UiPageIndex page) {
  // Flush the event queue.
  queue_.Flush();
  queue_.Touch();

  // Inform current page if any
  if (*event_handlers_.OnQuit) {
    (*event_handlers_.OnQuit)(page);
  }

#ifdef MIDIOUT_DEBUG_OUTPUT  
  printf("ShowPage: %d\n", page);
#endif

  // Only remember primary pages
  if (active_page_ >= FIRST_PAGE && active_page_ <= LAST_PAGE) {
    last_page_ = active_page_;
  }

  // Activate new page
  UiPageIndex prevPage = active_page_;
  active_page_ = page;

  // Load the page info structure in RAM.
  ResourcesManager::Load(page_registry, page, &page_info_);

  // Load the event handlers structure in RAM.
  ResourcesManager::Load(page_info_.event_handlers, 0, &event_handlers_);

  (*event_handlers_.OnInit)(&page_info_, prevPage);

  UpdateScreen();
}

Dialog::Dialog(prog_char* text, prog_char* buttons, uint8_t num_buttons) {
  data_.text = text;
  data_.buttons = buttons;
  data_.num_buttons = num_buttons;
  Ui::ShowDialog(data_);
}

/* static */
void Ui::ShowDialog(DlgData& dlg_data) {
  // Flush the event queue.
  queue_.Flush();
  queue_.Touch();

  // Load the Dlg page info structure in RAM.
  ResourcesManager::Load(page_registry, PAGE_DLG, &page_info_);

  // Load the event handlers structure in RAM.
  ResourcesManager::Load(page_info_.event_handlers, 0, &event_handlers_);

  memcpy(&page_info_.dlg_data, &dlg_data, sizeof(DlgData));

  (*event_handlers_.OnInit)(&page_info_, active_page_);

  UpdateScreen();

  // Run local message loop
  page_info_.dlg_data.result = 0;
  while (!page_info_.dlg_data.result) {
    ui.DoEvents();
  }

  // Set result for the caller
  dlg_data.result = page_info_.dlg_data.result;

  // Flush the event queue.
  queue_.Flush();
  queue_.Touch();

  // Load the active page info structure in RAM.
  ResourcesManager::Load(page_registry, active_page_, &page_info_);

  // Load the event handlers structure in RAM.
  ResourcesManager::Load(page_info_.event_handlers, 0, &event_handlers_);

  UpdateScreen();
}

/* static */
void Ui::Clear() {
  display.Clear();
}

/* static */
void Ui::UpdateScreen() {
  display.Clear();
  (*event_handlers_.UpdateScreen)();
}

/* static */
void Ui::RedrawScreen() {
  for (uint8_t n = 0; n < kLcdWidth * kLcdHeight; n++) {
    display.Tick();
  }
}

/* static */
uint8_t Ui::CheckShifted() {
  if (sideSwitches_.low(SIDE_SWITCH_SEL)) {
    inhibit_sel_raised_ = 1;
    return 1;
  }
  return 0;
}

/* static */
int8_t Ui::FixOctaveIncrement(int8_t value) {
  if (value > 1) return 12;
  if (value < -1) return -12;
  return value;
}

/* static */
void Ui::PrintChannel(char* buffer, uint8_t channel) {
  *buffer = (channel == 0xff)
      ? ' '
      : ((channel < 9) ? '1' + channel : (channel - 8));
}

/* static */
void Ui::PrintHex(char* buffer, uint8_t value) {
  *buffer++ = NibbleToAscii(U8ShiftRight4(value));
  *buffer = NibbleToAscii(value & 0xf);
}

/* static */
void Ui::PrintNote(char* buffer, uint8_t note) {
  uint8_t octave = 0;
  while (note >= 12) {
    ++octave;
    note -= 12;
  }

  switch (note) {
    case 1: case 3: case 6: case 8: case 10: buffer++; break;
  }

  static const prog_char note_names[] PROGMEM = " CC# DD# E FF# GG# AA# B";
  static const prog_char octaves[] PROGMEM = "-0123456789";

  *buffer++ = ResourcesManager::Lookup<char, uint8_t>(
      note_names, note << 1);
  *buffer++ = ResourcesManager::Lookup<char, uint8_t>(
      note_names, 1 + (note << 1));
  *buffer = ResourcesManager::Lookup<char, uint8_t>(
      octaves, octave);
}

/* static */
void Ui::PrintNumb(char* buffer, uint16_t numb) {
  uint8_t x = 0;
  if (numb < 10)  { x = 2; buffer[0] = '0'; buffer[1] = '0'; } else 
  if (numb < 100) { x = 1; buffer[0] = '0'; }
  UnsafeItoa(numb, 3, &buffer[x]);
}

/* static */
uint8_t Ui::GetEncoderState(uint8_t id) {
  if (id == ENCODER_A) {
    return encoderA_.immediate_value();
  } else
  if (id == ENCODER_B) {
    return encoderB_.immediate_value();
  } else
  if (id >= ENCODER_1 && id <= ENCODER_8) {
    return encoders_.state(id - ENCODER_1);
  }
  return 0;
}

/* static */
uint8_t Ui::GetEncoderClickValue(uint8_t id) {
  uint32_t time = milliseconds(); uint8_t value;
  if (last_click_encoder_ == id) {
    value = (time - last_click_time_) < kDblClickTime ? DBLCLICK : CLICK;
  } else {
    last_click_encoder_ = id;
    value = CLICK;
  }
  
  last_click_time_ = time;
  
  return value  + CheckShifted();
}

/* static */
int16_t Ui::Scale(int16_t x0, int16_t x0min, int16_t x0max, int16_t x1min, int16_t x1max) {
	if (x0max == x0min) 
		return x1max;
	
	int16_t x1 = x1min + ((x0 - x0min) * (x1max - x1min)) / (x0max - x0min);

	if (x1min < x1max) {
		if (x1 < x1min) {
			x1 = x1min;
		} else
		if (x1 > x1max) {
			x1 = x1max;
		}
	} else {
		if (x1 < x1max) {
			x1 = x1max;
		} else
		if (x1 > x1min) {
			x1 = x1min;
		}
	}
	
	return x1;
}

} // namespace midialf
