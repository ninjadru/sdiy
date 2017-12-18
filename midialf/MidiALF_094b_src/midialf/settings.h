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

#ifndef MIDIALF_SETTINGS_H_
#define MIDIALF_SETTINGS_H_

namespace midialf {

enum ClockMode {
  CLOCK_MODE_INTERNAL,
  CLOCK_MODE_EXTERNAL,
};

enum ClockDivision {
  CLOCK_DIVISION_NONE,
  CLOCK_DIVISION_X2,
  CLOCK_DIVISION_X4,
};

enum Direction {
  DIRECTION_FORWARD,
  DIRECTION_BACKWARD,
  DIRECTION_PENDULUM,
  DIRECTION_RANDOM,
};

enum LinkMode {
  LINK_MODE_NONE,
  LINK_MODE_16,
  LINK_MODE_32,
};

enum SeqSwitchMode {
  SEQ_SWITCH_MODE_IMMEDIATE,
  SEQ_SWITCH_MODE_ONSEQEND,
};

enum ProgramChangeMode {
  PROGRAM_CHANGE_NONE = 0x00,
  PROGRAM_CHANGE_RECV = 0x01,
  PROGRAM_CHANGE_SEND = 0x02,
  PROGRAM_CHANGE_BOTH = 0x03,
};

enum ControlChangeMode {
  CONTROL_CHANGE_NONE = 0x00,
  CONTROL_CHANGE_RECV = 0x01,
  CONTROL_CHANGE_SEND = 0x02,
  CONTROL_CHANGE_BOTH = 0x03,
};

enum CvMode {
  CVMODE_NOTE,
  CVMODE_VELO,
  CVMODE_CC1,
  CVMODE_CC2,
  CVMODE_LFO1,
  CVMODE_LFO2,
  CVMODE_ATCH,
  CVMODE_PBND,
  CVMODE_MAX = CVMODE_PBND,
  CVMODE_MASK = 0x0f,
  CVMODE_OFFSET = 0x80,
};

enum GateMode {
  GATEMODE_GATE,
  GATEMODE_STROBE,
  GATEMODE_CLOCK,
  GATEMODE_START,
  GATEMODE_SEQ,
  GATEMODE_LFO1,
  GATEMODE_LFO2,
  GATEMODE_MAX = GATEMODE_LFO2,
  GATEMODE_MASK = 0x0f,
  GATEMODE_INVERT = 0x80,
};

} // namespace midialf

#endif  // MIDIALF_SETTINGS_H_
