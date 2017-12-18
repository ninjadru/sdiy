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

#ifndef MIDIALF_MIDIALF_H_
#define MIDIALF_MIDIALF_H_

#include "avrlib/base.h"

#include <string.h>

#ifdef MIDIOUT_DEBUG_OUTPUT  
#include <stdlib.h>
#include <stdio.h>
#endif

#include "midialf/hardware_config.h"
#include "midialf/resources.h"
#include "midialf/settings.h"

namespace midialf {

#define MIDIALF_VERSION "0.94b"

// Configuration

static const uint8_t kNumSteps = 8;

static const uint8_t kDefBpm = 120;
static const uint8_t kDefBpm2 = 60;

static const uint8_t kDefClockRate = 7;   // 8th
static const uint8_t kDefClockRate2 = 4;  // 16th

static const uint8_t kDefRootNote = 60;   // C4

static const uint8_t kDefCC1Numb = 14;    // Shruthi-1 Filter cutoff
static const uint8_t kDefCC2Numb = 15;    // Shruthi-1 Filter resonance

static const uint8_t kDefChannel = 0;

static const uint8_t kDefVelocity = 100;

static const uint8_t kDefNoteLeng = 4;    // 16th

static const uint8_t kDefCC1Value = 100;
static const uint8_t kDefCC2Value = 100;

static const uint8_t kDefCV1Mode = CVMODE_NOTE;
static const uint8_t kDefCV2Mode = CVMODE_VELO;
static const uint8_t kDefCV3Mode = CVMODE_CC1;
static const uint8_t kDefCV4Mode = CVMODE_CC2;

static const uint8_t kDefGate1Mode = GATEMODE_GATE;
static const uint8_t kDefGate2Mode = GATEMODE_STROBE;
static const uint8_t kDefGate3Mode = GATEMODE_LFO1;
static const uint8_t kDefGate4Mode = GATEMODE_LFO2;

static const uint8_t kDefStrobeWidth = 0;
static const uint8_t kMinStrobeWidth = 0;
static const uint8_t kMaxStrobeWidth = 4;

static const uint8_t kDefSeqSwitchMode = SEQ_SWITCH_MODE_IMMEDIATE;

static const uint8_t kMinNameChar = ' ';
static const uint8_t kMaxNameChar = '~';
static const uint8_t kNameLength = 16;

// Device control change events

static const uint8_t kCCSetSequence = 106;
static const uint8_t kCCSetDirection = 107;
static const uint8_t kCCSetLinkMode = 108;
static const uint8_t kCCSetClockDivision = 109;
static const uint8_t kCCIncreaseTempo = 110;
static const uint8_t kCCDecreaseTempo = 111;
static const uint8_t kCCSetTempo = 112;
static const uint8_t kCCToggleRun = 113;
static const uint8_t kCCStopSequencer = 114;
static const uint8_t kCCStartSequencer = 115;
static const uint8_t kCCToggleRecording = 116;
static const uint8_t kCCStopRecording = 117;
static const uint8_t kCCStartRecording = 118;
static const uint8_t kCCReserved = 119;

// Useful declarations

#define numbof(a)  (sizeof(a)/sizeof(a[0]))
#define lengof(s)  (numbof(s) - 1)

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

#define SETFLAG(f,b)  (f)|= (b)
#define CLRFLAG(f,b)  (f)&=~(b)
#define SETFLAGTO(f,b,c)  if (c) SETFLAG(f,b); else CLRFLAG(f,b);

#define PSTRN(str) PSTR(str), lengof(str)

// Globally available externals

extern void FlashMidiInLed();
extern void FlashMidiOutLed();

extern void FlashRedDebugLed();
extern void FlashGreenDebugLed();

#ifdef MIDILED_DEBUG_OUTPUT
#define FLASH_RED_DEBUG_LED FlashRedDebugLed();
#define FLASH_GREEN_DEBUG_LED FlashGreenDebugLed();
#else
#define FLASH_RED_DEBUG_LED (void)0
#define FLASH_GREEN_DEBUG_LED (void)0
#endif

// 50ns @20MHz
static inline void nop() { 
  __asm__ volatile (
    "nop"
    ); 
}

// count*160ns @20MHz, 0=38us
static inline void Delay(uint8_t count) {
  __asm__ volatile (
    "1: dec %0" "\n\t"
    "brne 1b"
    : "=r" (count)
    : "0" (count)
  );
}

// Helper class to disable interrupts
class DisableInterrupts {
  uint8_t sreg_;
 public:
   DisableInterrupts() { sreg_= SREG; }
   ~DisableInterrupts() { SREG = sreg_;}
};

} // namespace midialf

#ifdef MIDIOUT_DEBUG_OUTPUT  
extern void DebugWrite(char c);
extern void DebugWrite(char* s);
extern void DebugWriteHex(char c);
#endif

#endif  // MIDIALF_MIDIALF_H_
