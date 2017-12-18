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

#ifndef MIDIALF_UI_H_
#define MIDIALF_UI_H_

#include "midialf/midialf.h"

#include "avrlib/devices/switch.h"
#include "avrlib/devices/rotary_encoder.h"
#include "avrlib/devices/rotary_encoder_array.h"
#include "avrlib/ui/event_queue.h"

namespace midialf {

using namespace avrlib;

typedef RotaryEncoder<EncoderA_A, EncoderA_B, EncoderA_C> EncoderA;
typedef RotaryEncoder<EncoderB_A, EncoderB_B, EncoderB_C> EncoderB;

typedef RotaryEncoderArray<Encoder_Load, IOClock, Encoder_SerialA, Encoder_SerialB, Encoder_SerialC> Encoders;

typedef DebouncedSwitches<StepSwitch_Load, IOClock, StepSwitch_Serial, 8, MSB_FIRST> StepSwitches;
typedef DebouncedSwitches<SideSwitch_Load, IOClock, SideSwitch_Serial, 8, MSB_FIRST> SideSwitches;

enum StepSwitchIndex {
  SWITCH_1,
  SWITCH_2,
  SWITCH_3,
  SWITCH_4,
  SWITCH_5,
  SWITCH_6,
  SWITCH_7,
  SWITCH_8,
};

enum SideSwitchIndex {
  SIDE_SWITCH_SEQA,
  SIDE_SWITCH_SEQB,
  SIDE_SWITCH_SEQC,
  SIDE_SWITCH_SEQD,
  SIDE_SWITCH_RUN,
  SIDE_SWITCH_SEQ,
  SIDE_SWITCH_CLK,
  SIDE_SWITCH_SEL,
};

enum ControlId {
  ENCODER_1,
  ENCODER_2,
  ENCODER_3,
  ENCODER_4,
  ENCODER_5,
  ENCODER_6,
  ENCODER_7,
  ENCODER_8,
  ENCODER_A,
  ENCODER_B,
  SWITCH,
  SWITCH_SHIFTED,
  SIDE_SWITCH,
  SIDE_SWITCH_SHIFTED,
  SIDE_SWITCH_RAISED,
};

enum EncoderClickValue {
  CLICK,
  CLICK_SHIFTED,
  DBLCLICK,
  DBLCLICK_SHIFTED,
};

enum ControlTypeEx {
  CONTROL_REQUEST = 10,
};

enum RequestControlId {
  REQUEST_SHOWPAGE = 1,     // event.value specifies page id
  REQUEST_UPDATESCREEN, 
  REQUEST_SENDCURSEQUENCE, 
  REQUEST_SENDCURPROGRAM, 
  REQUEST_SENDALLPROGRAMS,
  REQUEST_SENDPROGRAM,      // event.value specifies program slot
};

enum UiPageIndex {
  PAGE_NOTE,
  PAGE_VELO,
  PAGE_GATE,
  PAGE_CC1,
  PAGE_CC2,
  PAGE_SEQ_SETTINGS,
  PAGE_PRG_SETTINGS,
  PAGE_SYS_SETTINGS,
#ifdef ENABLE_CV_OUTPUT
  PAGE_EXT_SETTINGS,
#endif
  PAGE_LFO1,
  PAGE_LFO2,
  FIRST_PAGE = PAGE_NOTE,
  LAST_PAGE = PAGE_LFO2,

  PAGE_DLG,
  PAGE_COPYSEQ,
  PAGE_COMMAND,
  PAGE_INIT,
  PAGE_SAVE,
  PAGE_LOAD,
  PAGE_SEND_SYSEX,
  PAGE_RECV_SYSEX,
  PAGE_RANDOMIZE,
  PAGE_ROTATE,
#ifdef ENABLE_CV_OUTPUT
  PAGE_TUNE,
#endif
};

struct PageInfo;

struct EventHandlers {
  void (*OnInit)(PageInfo* pageInfo, UiPageIndex prevPage);
  void (*OnQuit)(UiPageIndex nextPage);
  uint8_t (*OnIncrement)(uint8_t, int8_t);
  uint8_t (*OnClick)(uint8_t, uint8_t);
  uint8_t (*OnSwitch)(uint8_t, uint8_t);
  uint8_t (*OnIdle)();
  void (*UpdateScreen)();
  void (*UpdateLeds)();
};

typedef EventHandlers PROGMEM prog_EventHandlers;

struct DlgData {
  const prog_char* text;
  const prog_char* buttons;
  uint8_t num_buttons;
  uint8_t result;
};

class Dialog {
 public:
  Dialog(prog_char* text, prog_char* buttons, uint8_t num_buttons);

  uint8_t result() { return data_.result; }

 protected:
  DlgData data_;
};

struct PageInfo {
  uint8_t index;
  const prog_EventHandlers* event_handlers;
  union {
    uint8_t data[1];
    DlgData dlg_data;
  };
};

typedef PageInfo PROGMEM prog_PageInfo;

class Ui {
 public:
  static void Init();
  static void Logo();
  static void Poll();
  static void DoEvents();
  
  static void ShowPage(UiPageIndex page);
  static void ShowLastPage() { ShowPage(last_page_); }
  static void ShowDialog(DlgData& dlg_data);

  static UiPageIndex active_page() { return active_page_; }
  static UiPageIndex last_page() { return last_page_; }

  static void Clear();

  static void UpdateScreen();
  static void RedrawScreen();

  static void RequestRefresh() { 
    queue_.AddEvent(CONTROL_REFRESH, 0, 0);
  }

  static void AddRequest(uint8_t id = 0, uint8_t value = 0) { 
    queue_.AddEvent(CONTROL_REQUEST, id, value);
  }

  static uint8_t CheckShifted();
  
  static int8_t FixOctaveIncrement(int8_t value);
  
  static void PrintChannel(char* buffer, uint8_t channel);
  static void PrintHex(char* buffer, uint8_t value);
  static void PrintNote(char* buffer, uint8_t note);
  static void PrintNumb(char* buffer, uint16_t numb);

  static uint8_t GetEncoderState(uint8_t id);
  static uint8_t GetEncoderClickValue(uint8_t id);

  static int16_t Scale(int16_t x0, int16_t x0min, int16_t x0max, int16_t x1min, int16_t x1max);
  
 private:
  static EncoderA encoderA_;
  static EncoderB encoderB_;
  static Encoders encoders_;

  static StepSwitches stepSwitches_;
  static SideSwitches sideSwitches_;

  static avrlib::EventQueue<32> queue_;

  static UiPageIndex active_page_;
  static UiPageIndex last_page_;
  static EventHandlers event_handlers_;
  static PageInfo page_info_;

  static uint8_t cycle_;
  static uint8_t save_state_;
  static uint8_t inhibit_sel_raised_;

  static uint8_t last_click_encoder_;
  static uint32_t last_click_time_;

  static uint16_t led_blink_cycle_;

  static void HandleEvent(const Event& e);
  static void HandleEncoderEvent(const Event& e);
  static void HandleEncoderClickEvent(const Event& e);
  static void HandleSwitchEvent(const Event& e);
  static void HandleRequestEvent(const Event& e);

  static void UpdateSideLeds();
  static void UpdateStepLeds();
};

extern Ui ui;

} // namespace midialf

#endif // MIDIALF_UI_H_
