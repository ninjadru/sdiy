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
// Main definitions.

#ifndef MIDIALF_HARDWARE_CONFIG_H_
#define MIDIALF_HARDWARE_CONFIG_H_

#include "avrlib/base.h"
#include "avrlib/gpio.h"
#include "avrlib/parallel_io.h"
#include "avrlib/serial.h"
#include "avrlib/spi.h"

namespace midialf {

using avrlib::Gpio;
using avrlib::LSB_FIRST;
using avrlib::MSB_FIRST;
using avrlib::PARALLEL_DOUBLE_HIGH;
using avrlib::PARALLEL_NIBBLE_HIGH;
using avrlib::ParallelPort;
using avrlib::PortA;
using avrlib::PortB;
using avrlib::PortC;
using avrlib::PortD;
using avrlib::SerialPort0;
using avrlib::SpiMaster;
using avrlib::SpiSS;

// Useful constants
static const uint8_t kLcdWidth = 40;
static const uint8_t kLcdHeight = 2;

static const uint16_t kBankSize = 8192;
static const uint16_t kMaxNumBanks = 8;

// MIDI
typedef SerialPort0 MidiPort;

// LCD
typedef Gpio<PortD, 3> LcdRsLine;
typedef Gpio<PortD, 2> LcdEnableLine;
typedef ParallelPort<PortD, PARALLEL_NIBBLE_HIGH> LcdDataBus;

// Encoder A
typedef Gpio<PortC, 2> EncoderA_A;
typedef Gpio<PortC, 3> EncoderA_B;
typedef Gpio<PortC, 4> EncoderA_C;

// Encoder B
typedef Gpio<PortC, 5> EncoderB_A;
typedef Gpio<PortC, 6> EncoderB_B;
typedef Gpio<PortC, 7> EncoderB_C;

// IO
#ifdef MIDIALF_V01
typedef Gpio<PortB, 0> IOClock;
typedef Gpio<PortB, 1> Encoder_Load;
typedef Gpio<PortB, 2> Encoder_SerialB;
typedef Gpio<PortB, 3> Encoder_SerialA;
typedef Gpio<PortB, 4> Encoder_SerialC;
typedef Gpio<PortB, 5> PortMOSI;
typedef Gpio<PortB, 6> PortMISO;
typedef Gpio<PortB, 7> PortSCK;
#else
typedef Gpio<PortB, 0> Encoder_Load;
typedef Gpio<PortB, 1> Encoder_SerialB;
typedef Gpio<PortB, 2> Encoder_SerialA;
typedef Gpio<PortB, 3> Encoder_SerialC;
typedef Gpio<PortB, 4> IOClock;
typedef Gpio<PortB, 5> PortMOSI;
typedef Gpio<PortB, 6> PortMISO;
typedef Gpio<PortB, 7> PortSCK;
#endif

typedef Gpio<PortA, 0> StepSwitch_Load;
typedef Gpio<PortA, 1> StepSwitch_Serial;
typedef Gpio<PortA, 2> SideSwitch_Load;
typedef Gpio<PortA, 3> SideSwitch_Serial;
typedef Gpio<PortA, 4> Led_Load;
typedef Gpio<PortA, 5> Led_Serial;
typedef Gpio<PortA, 6> IOPort1;
typedef Gpio<PortA, 7> IOPort2;

// MIDI LEDs
#ifndef ENABLE_CV_OUTPUT
typedef PortMOSI LedOut;
typedef PortSCK LedIn;
#endif

// CV
#ifdef ENABLE_CV_OUTPUT
typedef SpiMaster<SpiSS, MSB_FIRST, 2> SPIInterface;
typedef ParallelPort<PortA, PARALLEL_DOUBLE_HIGH> SSSelector;
typedef Encoder_Load SSEnable;  // Sharing output port!
#endif

} // namespace midialf

#endif  // MIDIALF_HARDWARE_CONFIG_H_
