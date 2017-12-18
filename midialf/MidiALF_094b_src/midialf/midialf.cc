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

#include "midialf/midialf.h"

#include "avrlib/boot.h"
#include "avrlib/time.h"
#include "avrlib/timer.h"
#ifdef MIDIOUT_DEBUG_OUTPUT  
#include "avrlib/debug_output.h"
#endif
#include "avrlib/random.h"
#include "avrlib/watchdog_timer.h"
#include "avrlib/devices/led.h"

#include "midi/midi.h"
#include "midialf/event_scheduler.h"
#include "midialf/midi_handler.h"
#include "midialf/resources.h"
#include "midialf/display.h"
#include "midialf/storage.h"
#include "midialf/clock.h"
#include "midialf/state.h"
#include "midialf/leds.h"
#include "midialf/seq.h"
#include "midialf/lfo.h"
#include "midialf/ui.h"
#ifdef ENABLE_CV_OUTPUT
#include "midialf/cv/cv.h"
#include "midialf/cv/port.h"
#endif

#include "util/delay.h"

using namespace avrlib;
using namespace midi;
using namespace midialf;

// Midi input

Serial<MidiPort, 31250, POLLED, POLLED> midi_io;
MidiStreamParser<MidiHandler> midi_parser;

// Midi LEDs

#ifndef ENABLE_CV_OUTPUT
FlashLed< Led<LedIn, LED_SOURCE_CURRENT>, 5> midiInLed;
FlashLed< Led<LedOut, LED_SOURCE_CURRENT>, 5> midiOutLed;
#endif

namespace midialf {

#ifndef ENABLE_CV_OUTPUT
#ifdef MIDILED_DEBUG_OUTPUT
void FlashMidiInLed() {}
void FlashMidiOutLed() {}
void FlashRedDebugLed() { midiInLed.On(); }
void FlashGreenDebugLed() { midiOutLed.On(); }
#else
void FlashMidiInLed() { midiInLed.On(); }
void FlashMidiOutLed() { midiOutLed.On(); }
void FlashRedDebugLed() {}
void FlashGreenDebugLed() {}
#endif
#endif

} // namespace midialf

#ifdef MIDIOUT_DEBUG_OUTPUT  
DebugOutput< Serial<SerialPort0, 57600, DISABLED, POLLED> > dbg;
void DebugWrite(char c) { dbg.Write(c); }
void DebugWrite(char* s) { dbg.Write(s); }
void DebugWriteHex(char c) {
  static char hex[] = "0123456789abcdef";
  dbg.Write(hex[(uint8_t)c >> 4]);
  dbg.Write(hex[(uint8_t)c & 0x0f]);
}
#endif

// Timer2 ISR: midi i/o and ui

inline void PollMidiIn() {
  // Receive midi input
  if (midi_io.readable()) {
    uint8_t byte = midi_io.ImmediateRead();
    if (byte != 0xfe) {
#ifndef ENABLE_CV_OUTPUT
      if (byte != 0xf8) midiInLed.On();
#endif
      midi_parser.PushByte(byte);
    }
  }
}

inline void SendMidiOut() {
#ifndef MIDIOUT_DEBUG_OUTPUT  
  // Send midi output
  if (MidiHandler::OutputBuffer::readable() && midi_io.writable()) {
    uint8_t byte = MidiHandler::OutputBuffer::ImmediateRead();
#ifndef ENABLE_CV_OUTPUT
    if (byte != 0xf8) midiOutLed.On();
#endif
    midi_io.Overwrite(byte);
  }
#endif
}

ISR(TIMER2_OVF_vect, ISR_NOBLOCK) {
  // Called at 4.9KHz

  // Handle MIDI I/O
  PollMidiIn();
  SendMidiOut();

  // Handle CV/Gates
#ifdef ENABLE_CV_OUTPUT
  cv.Tick();
  port.Tick();
#endif

  // Handle lower priority tasks
  static uint8_t sub_clock;
  ++sub_clock;
  if ((sub_clock & 1) == 0) {
    // 2.45KHz
    ui.Poll();
    if ((sub_clock & 3) == 0) {
      // 1.225KHz
      TickSystemClock();
#ifndef ENABLE_CV_OUTPUT
      midiInLed.Tick();
      midiOutLed.Tick();
#endif
      leds.Write();
      // 306Hz
      if ((sub_clock & 7) == 0) {
        display.BlinkCursor();
      }
    }
  } 
}

// Timer1 ISR: internal clock

ISR(TIMER1_COMPA_vect) {
  PwmChannel1A::set_frequency(clock.Tick());
  if (clock.running()) {
    seq.OnInternalClockTick();
    if (clock.stepped()) {
      seq.OnInternalClockStep();
    }
  }
}

void Init() {
  sei();
  UCSR0B = 0;

#ifndef ENABLE_CV_OUTPUT
  IOPort1::set_mode(DIGITAL_OUTPUT);
  IOPort2::set_mode(DIGITAL_OUTPUT);
  midiInLed.Init();
  midiOutLed.Init();
#endif
 
  event_scheduler.Init();

  Timer<1>::set_prescaler(1);
  Timer<1>::set_mode(0, _BV(WGM12), 3);
  PwmChannel1A::set_frequency(6510);
  Timer<1>::StartCompare();
  
  Timer<2>::set_prescaler(2);
  Timer<2>::set_mode(TIMER_PWM_PHASE_CORRECT);
  Timer<2>::Start();

#ifndef MIDIOUT_DEBUG_OUTPUT  
  midi_io.Init();
#else
  dbg.Init();
#endif

#ifdef ENABLE_CV_OUTPUT
  cv.Init();
  port.Init();
#endif

  seq.Init();
  lfo.Init();
  leds.Init();
  clock.Init();
  storage.Init();
  ui.Init();
}

int main(void) {
  ResetWatchdog();
  Init();

  while (1) {
    ui.DoEvents();
  }
}
