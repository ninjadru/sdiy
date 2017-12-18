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
// SysEx messages handler class.

#ifndef MIDIALF_SYSEX_HANDLER_H_
#define MIDIALF_SYSEX_HANDLER_H_

#include "midialf/midialf.h"
#include "midialf/storage.h"
#include "midialf/seq.h"

namespace midialf {

static const uint16_t kSysExBlockSize = 256 + 1;  // slot data plus checksum

enum SysExReceptionState {
  RECEIVING_HEADER,
  RECEIVING_COMMAND,
  RECEIVING_DATA,
  RECEPTION_OK,
  RECEPTION_ERROR,
};

enum SysExCommand {
 SYSEXCMD_SEQUENCEDATA = 0x01, // SeqData structure
 SYSEXCMD_PROGRAMDATA  = 0x02, // (cb)SeqInfo + (cb)4 x SeqData), arg == 1 saves to storage
 SYSEXCMD_REQCURSEQUENCEDATA = 0x11, // send current sequence data request
 SYSEXCMD_REQCURPROGRAMDATA  = 0x12, // send current program data request
 SYSEXCMD_REQPROGRAMDATA     = 0x13, // send program data request, 1 byte payload is program slot index
 SYSEXCMD_REQALLPROGRAMDATA  = 0x14, // send all programs data request
 SYSEXCMD_REQSEQUENCEDATA_SAVE = 0x21, // save current sequence data request
 SYSEXCMD_REQPROGRAMDATA_SAVE  = 0x22, // save program data request, 1 byte payload is program slot index
};

typedef uint8_t (*ProgressCallback)(uint16_t done, uint16_t total);

class SysExHandler {
 public:

  static void SendSeq(uint8_t seq);
  static void SendSeq() { SendSeq(seq.seq()); }

  static void SendPgm();
  static void SendPgm(uint8_t slot);

  static void SendAll(ProgressCallback callback);
  static void SendAll() { SendAll(SendAllProgressCallback); }

  static void Receive(uint8_t byte);
  
 private:
  static void SendSysExHeader(uint8_t cmd, uint8_t arg = 0);
  static void SendSysExData(const void* data, uint16_t size);
  static void SendByte(uint8_t byte) {
    seq.SendNow(byte);
  }

  static uint8_t PreparePgmData();
  static uint8_t SendAllProgressCallback(uint16_t done, uint16_t total);

  static void AcceptCommand();
  static uint8_t CalcCheckSum(const uint8_t* data, uint16_t size);

  static void RecvSeq();
  static void RecvPgm();

  static uint16_t bytes_received_;
  static uint8_t buffer_[kSysExBlockSize];
  static uint8_t state_;
  static uint8_t checksum_;
  static uint8_t command_[2];
};

extern SysExHandler sysex_handler;

} // namespace midialf:

#endif // MIDIALF_SYSEX_HANDLER_H_
