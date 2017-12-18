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

#include "midialf/sysex_handler.h"
#include "midialf/display.h"
#include "midialf/state.h"
#include "midialf/ui.h"

namespace midialf {

using namespace avrlib;

/* extern */
SysExHandler sysex_handler;

/* <static> */
uint16_t SysExHandler::bytes_received_;
uint8_t SysExHandler::buffer_[kSysExBlockSize];
uint8_t SysExHandler::state_;
uint8_t SysExHandler::checksum_;
uint8_t SysExHandler::command_[2];
/* </static> */

static const prog_char header[] PROGMEM = {
  0xf0,  // <SysEx>
  0x29,  // PPG manufacturer ID
  'A','L','F',0x00, // ALF Product ID
  // Then:
  // * Command byte
  // * Argument byte
  // * Payload bytes
  // * Checksum byte
  // 0xf7 EOX byte
};

///////////////////////////////////////////////////////////////////////////////
// Send SysEx routines
///////////////////////////////////////////////////////////////////////////////

/* static */
void SysExHandler::SendSeq(uint8_t seq_index) {
  seq.CopySeqData(seq_index, *((SeqData*)&buffer_));
  SendSysExHeader(SYSEXCMD_SEQUENCEDATA, seq_index);
  SendSysExData(buffer_, sizeof(SeqData));
}

/* static */
void SysExHandler::SendPgm() {
  uint8_t cb = PreparePgmData();
  SendSysExHeader(SYSEXCMD_PROGRAMDATA);
  SendSysExData(buffer_, cb);
}

/* static */
void SysExHandler::SendPgm(uint8_t slot) {
  state.Save();
  seq.LoadFromStorage(slot);
  SendPgm();
  state.Load();
}

/* static */
void SysExHandler::SendAll(ProgressCallback callback) {
  state.Save();
  uint16_t total = storage.num_slots();
  for (uint16_t n = 0; n < total; n++) {
    if (!(*callback)(n, total))
      break;
    seq.LoadFromStorage(n);
    uint8_t cb = PreparePgmData();
    SendSysExHeader(SYSEXCMD_PROGRAMDATA, 1);
    SendSysExData(buffer_, cb);
  }
  state.Load();
}

/* static */
void SysExHandler::SendSysExHeader(uint8_t cmd, uint8_t arg) {
  // Send SysEx header
  for (uint8_t i = 0; i < sizeof(header); i++) {
    SendByte(pgm_read_byte(header + i));
  }
  // Send SysEx cmd code and arg
  SendByte(cmd);
  SendByte(arg);
}

/* static */
void SysExHandler::SendSysExData(const void* data, uint16_t size) {
  const uint8_t* ptr = static_cast<const uint8_t*>(data);

  // Send the data
  uint8_t checksum = 0;
  for (uint16_t i = 0; i < size; ++i) {
    checksum += ptr[i];
    SendByte(U8ShiftRight4(ptr[i]));
    SendByte(ptr[i] & 0x0f);
  }

  // Send checksum
  SendByte(U8ShiftRight4(checksum));
  SendByte(checksum & 0x0f);

  // Send EOX
  SendByte(0xf7);
  
  // Space blocks by 100ms
  ConstantDelay(100);
}

/* static */
uint8_t SysExHandler::PreparePgmData() {
  uint8_t cb = 0;
  // Add sequence info
  buffer_[cb++] = sizeof(SeqInfo);
  seq.SaveSeqInfo(*((SeqInfo*)&buffer_[cb])); cb+= sizeof(SeqInfo);
  // Add sequence data
  buffer_[cb++] = sizeof(SeqData) * 4;
  for (uint8_t n = 0; n < 4; n++) {
    seq.CopySeqData(n, *((SeqData*)&buffer_[cb]));
    cb+= sizeof(SeqData);
  }
  return cb;
}

///////////////////////////////////////////////////////////////////////////////
// Receive SysEx routines
///////////////////////////////////////////////////////////////////////////////

/* static */
void SysExHandler::Receive(uint8_t byte) {
  if (byte == 0xf0) {
    bytes_received_ = 0;
    state_ = RECEIVING_HEADER;
  }
  switch (state_) {
    case RECEIVING_HEADER:
      if (pgm_read_byte(header + bytes_received_) == byte) {
        bytes_received_++;
        if (bytes_received_ >= sizeof(header)) {
          state_ = RECEIVING_COMMAND;
          bytes_received_ = 0;
        }
      } else {
        state_ = RECEPTION_ERROR;
      }
      break;

    case RECEIVING_COMMAND:
      command_[bytes_received_++] = byte;
      if (bytes_received_ == 2) {
        state_ = RECEIVING_DATA;
        bytes_received_ = 0;
      }
      break;

    case RECEIVING_DATA:
      if (byte != 0xf7) {
        uint16_t i = bytes_received_ >> 1;
        if (bytes_received_ & 1) {
          buffer_[i] |= byte & 0xf;
        } else {
          buffer_[i] = U8ShiftLeft4(byte);
        }
        bytes_received_++;
      } else {
        if (!bytes_received_ || bytes_received_ & 1) {
          state_ = RECEPTION_ERROR;
        } else {
          uint16_t cb = (bytes_received_ >> 1) - 1;
          if (CalcCheckSum(buffer_, cb) == buffer_[cb]) {
            state_ = RECEPTION_OK;
            bytes_received_ = cb;
            AcceptCommand();
          } else
            state_ = RECEPTION_ERROR;
        }
      }
    break;
  }
}

/* static */
void SysExHandler::AcceptCommand() {
  seq.Stop();
  switch (command_[0]) {
    case SYSEXCMD_SEQUENCEDATA:
      RecvSeq();
      break;
    case SYSEXCMD_PROGRAMDATA:
      RecvPgm();
      break;
    case SYSEXCMD_REQCURSEQUENCEDATA:
      if (bytes_received_ == 0) {
        Ui::AddRequest(REQUEST_SENDCURSEQUENCE);
      }
      break;
    case SYSEXCMD_REQCURPROGRAMDATA:
      if (bytes_received_ == 0) {
        Ui::AddRequest(REQUEST_SENDCURPROGRAM);
      }
      break;
    case SYSEXCMD_REQPROGRAMDATA:
      if (bytes_received_ == 1) {
        Ui::AddRequest(REQUEST_SENDPROGRAM, buffer_[0]);
      }
      break;
    case SYSEXCMD_REQALLPROGRAMDATA:
      if (bytes_received_ == 0) {
        Ui::AddRequest(REQUEST_SENDALLPROGRAMS);
      }
      break;
    case SYSEXCMD_REQSEQUENCEDATA_SAVE:
      if (bytes_received_ == 0) {
        seq.SaveToStorage();
      }
      break;
    case SYSEXCMD_REQPROGRAMDATA_SAVE:
      if (bytes_received_ == 1) {
        seq.SaveToStorage(buffer_[0]);
      }
      break;
  }
}

/* static */
uint8_t SysExHandler::CalcCheckSum(const uint8_t* data, uint16_t size) {
  uint8_t checksum = 0;
  for (uint16_t i = 0; i < size; ++i) {
    checksum += data[i];
  }
  return checksum;
}

uint8_t SysExHandler::SendAllProgressCallback(uint16_t done, uint16_t total) {
  Ui::Clear();

  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  
  static const prog_char cmdCancel[] PROGMEM = "[Cancel]";

  memcpy_P(&line1[0], PSTRN("Sending program:"));
  memcpy_P(&line1[kLcdWidth - lengof(cmdCancel)], cmdCancel, lengof(cmdCancel));

  uint8_t name[kNameLength]; storage.ReadSlotName(done, name);
  Ui::PrintNumb(&line2[0], 1 + done);
  memcpy(&line2[4], name, kNameLength);
  
  Ui::RedrawScreen();

  // Check if cancelled
  if (Ui::GetEncoderState(ENCODER_B) == 0)
    return 0;

  return 1;
}

/* static */
void SysExHandler::RecvSeq() {
  SeqData data;
  memcpy(&data, buffer_, min(sizeof(data), bytes_received_));
  seq.LoadSeqData(data);
  Ui::RequestRefresh();
}

/* static */
void SysExHandler::RecvPgm() {
  uint8_t slot = seq.slot();
  uint16_t i = 0;

  // Load sequence info
  uint8_t cb = buffer_[i++];
  SeqInfo info;
  memcpy(&info, &buffer_[i], min(sizeof(info), cb));
  if (command_[1] == 0) info.slot_ = slot;
  seq.LoadSeqInfo(info);
  i+= cb;

  // Load sequence data
  cb = buffer_[i++] / 4;
  for (uint8_t n = 0; n < 4; n++) {
    SeqData data;
    memcpy(&data, &buffer_[i], min(sizeof(data), cb));
    seq.LoadSeqData(n, data);
    i+= cb;
  }

  // Save program if requested and refresh screen
  if (command_[1] == 1) {
    seq.SaveToStorage();
  }
  
  Ui::AddRequest(REQUEST_SHOWPAGE, PAGE_RECV_SYSEX);
}

} // namespace midialf
