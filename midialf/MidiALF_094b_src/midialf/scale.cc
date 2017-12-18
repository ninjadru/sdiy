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
// Scale functions.

#include "midialf/scale.h"

#include "avrlib/op.h"

namespace midialf {

using namespace avrlib;

/////////////////////////////////////////////////////////////////////////////
// Scale table is courtesy of midibox.org, used with kind permission of 
// Thorsten Klose (tk@midibox.org). The original tables are located here:
// svn://svnmios.midibox.org/mios32/trunk/apps/processing/midi_force_to_scale

typedef union {
  struct {
    uint8_t c:4;
    uint8_t cd:4;
    uint8_t d:4;
    uint8_t dd:4;
    uint8_t e:4;
    uint8_t f:4;
    uint8_t fd:4;
    uint8_t g:4;
    uint8_t gd:4;
    uint8_t a:4;
    uint8_t ad:4;
    uint8_t b:4;
    char name[kScaleNameSize + 1];
  };

  struct {
    uint8_t notes[6]; // unfortunately "unsigned notes:4[12]" doesn't work
  };
} seq_scale_entry_t;

const seq_scale_entry_t seq_scale_table[] PROGMEM = {
//	        C 	C#	D 	D#	E 	F 	F#	G 	G#	A 	A#	B
//	        0 	1 	2 	3 	4 	5 	6 	7 	8 	9 	10	11 	Semitone
//              1 	b2	2 	b3	3 	4 	b5	5 	b6	6 	b7	7	Minor Tone
//              1 	#1	2 	#2	3 	4 	#4	5 	#5	6 	#6	7 	Augmented Tone
// 1..10                                                                                                         <------------------>
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	11,	11,	"Major               " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	11,	11,	"Harmonic Minor      " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	9,	9,	11,	11,	"Melodic Minor       " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"Natural Minor       " },
  {             0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	"Chromatic           " },
  {             0,	2,	2,	4,	4,	6,	6,	8,	8,	10,	10,	10,	"Whole Tone          " },
  {             0,	2,	2,	4,	4,	7,	7,	7,	9,	9,	9,	9,	"Pentatonic Major    " },
  {             0,	0,	3,	3,	3,	5,	7,	7,	7,	10,	10,	10,	"Pentatonic Minor    " },
  {             0,	0,	3,	3,	3,	5,	6,	7,	7,	10,	10,	10,	"Pentatonic Blues    " },
  {             0,	2,	2,	2,	5,	5,	7,	7,	7,	10,	10,	10,	"Pentatonic Neutral  " },
// 11..20                                                                                                        <------------------>
  {             0,	1,	1,	3,	4,	6,	6,	7,	9,	9,	10,	10,	"Octatonic (H-W)     " },
  {             0,	2,	2,	3,	3,	5,	6,	6,	8,	9,	11,	11,	"Octatonic (W-H)     " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	11,	11,	"Ionian              " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	9,	9,	10,	10,	"Dorian              " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"Phrygian            " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	9,	9,	11,	11,	"Lydian              " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	10,	10,	"Mixolydian          " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"Aeolian             " },
  {             0,	1,	1,	3,	3,	5,	6,	6,	8,	8,	10,	10,	"Locrian             " },
  {             0,	2,	2,	3,	3,	5,	6,	7,	8,	8,	11,	11,	"Algerian            " },
// 21..30                                                                                                        <------------------>
  {             0,	2,	2,	3,	3,	5,	6,	8,	8,	9,	11,	11,	"Arabian (A)         " },
  {             0,	2,	2,	4,	4,	5,	6,	8,	8,	10,	10,	10,	"Arabian (B)         " },
  {             0,	3,	3,	3,	4,	6,	6,	8,	8,	11,	11,	11,	"Augmented           " },
  {             0,	2,	2,	3,	3,	5,	6,	8,	8,	9,	11,	11,	"Auxiliary Diminished" },
  {             0,	2,	2,	4,	4,	6,	6,	8,	8,	10,	10,	10,	"Auxiliary Augmented " },
  {             0,	1,	1,	3,	4,	6,	6,	7,	9,	9,	10,	10,	"Auxiliary Diminished" },
  {             0,	1,	1,	3,	3,	7,	7,	7,	8,	8,	8,	8,	"Balinese            " },
  {             0,	0,	3,	3,	3,	5,	6,	7,	7,	10,	10,	10,	"Blues               " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	8,	8,	11,	11,	"Byzantine           " },
  {             0,	0,	4,	4,	4,	6,	6,	7,	7,	11,	11,	11,	"Chinese             " },
// 31..40                                                                                                        <------------------>
  {             0,	2,	2,	4,	4,	7,	7,	7,	9,	9,	9,	9,	"Chinese Mongolian   " },
  {             0,	2,	2,	4,	4,	7,	7,	7,	9,	9,	9,	9,	"Diatonic            " },
  {             0,	2,	2,	3,	3,	5,	6,	6,	8,	9,	11,	11,	"Diminished          " },
  {             0,	1,	1,	3,	4,	6,	6,	7,	9,	9,	10,	10,	"Diminished, Half    " },
  {             0,	2,	2,	3,	3,	5,	6,	6,	8,	9,	11,	11,	"Diminished, Whole   " },
  {             0,	1,	1,	3,	4,	6,	6,	6,	8,	8,	10,	10,	"Diminished WholeTone" },
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	10,	10,	"Dominant 7th        " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	8,	8,	11,	11,	"Double Harmonic     " },
  {             0,	2,	2,	2,	5,	5,	7,	7,	7,	10,	10,	10,	"Egyptian            " },
  {             0,	1,	1,	3,	4,	5,	6,	6,	8,	8,	10,	10,	"Eight Tone Spanish  " },
// 41..50                                                                                                        <------------------>
  {             0,	1,	1,	4,	4,	6,	6,	8,	8,	10,	10,	11,	"Enigmatic           " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	11,	11,	"Ethiopian (A raray) " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"Ethiopian Geez Ezel " },
  {             0,	1,	1,	3,	3,	5,	6,	6,	8,	8,	10,	10,	"Half Dim (Locrian)  " },
  {             0,	2,	2,	3,	3,	5,	6,	6,	8,	8,	10,	10,	"Half Dim 2 (Locrian)" },
  {             0,	2,	2,	3,	3,	5,	7,	7,	9,	9,	11,	11,	"Hawaiian            " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	8,	8,	10,	10,	"Hindu               " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	8,	8,	10,	10,	"Hindustan           " },
  {             0,	2,	2,	3,	3,	7,	7,	7,	8,	8,	8,	8,	"Hirajoshi           " },
  {             0,	3,	3,	3,	4,	6,	6,	7,	9,	9,	10,	10,	"Hungarian Major     " },
// 51..60                                                                                                        <------------------>
  {             0,	2,	2,	3,	3,	6,	6,	7,	8,	8,	11,	11,	"Hungarian Gypsy     " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	8,	8,	11,	11,	"Hungarian G. Persian" },
  {             0,	2,	2,	3,	3,	6,	6,	7,	8,	8,	11,	11,	"Hungarian Minor     " },
  {             0,	1,	1,	1,	5,	5,	7,	7,	8,	8,	8,	8,	"Japanese (A)        " },
  {             0,	2,	2,	2,	5,	5,	7,	7,	8,	8,	8,	8,	"Japanese (B)        " },
  {             0,	2,	2,	4,	4,	5,	6,	7,	9,	9,	11,	11,	"Japan. (Ichikosucho)" },
  {             0,	2,	2,	4,	4,	5,	6,	7,	9,	9,	10,	11,	"Japan. (Taishikicho)" },
  {             0,	1,	1,	3,	3,	5,	7,	7,	9,	9,	10,	10,	"Javanese            " },
  {             0,	1,	2,	3,	3,	5,	7,	7,	9,	9,	10,	10,	"Jewish(AdonaiMalakh)" },
  {             0,	1,	1,	4,	4,	5,	7,	7,	8,	8,	10,	10,	"Jewish (Ahaba Rabba)" },
// 61..70                                                                                                        <------------------>
  {             0,	1,	1,	3,	4,	6,	6,	8,	8,	10,	10,	11,	"Jewish (Magen Abot) " },
  {             0,	2,	2,	3,	3,	7,	7,	7,	9,	9,	9,	9,	"Kumoi               " },
  {             0,	2,	2,	4,	4,	6,	6,	8,	8,	10,	10,	11,	"Leading Whole Tone  " },
  {             0,	2,	2,	4,	4,	6,	6,	8,	8,	9,	11,	11,	"Lydian Augmented    " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	8,	8,	10,	10,	"Lydian Minor        " },
  {             0,	2,	2,	3,	3,	6,	6,	7,	9,	9,	11,	11,	"Lydian Diminished   " },
  {             0,	2,	2,	4,	4,	5,	6,	6,	8,	8,	10,	10,	"Major Locrian       " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	11,	11,	"Mohammedan          " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	8,	8,	11,	11,	"Neopolitan          " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	9,	9,	11,	11,	"Neoploitan Major    " },
// 71..80                                                                                                        <------------------>
  {             0,	1,	1,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"Neopolitan Minor    " },
  {             0,	2,	2,	3,	4,	6,	6,	7,	8,	9,	11,	11,	"Nine Tone Scale     " },
  {             0,	1,	1,	4,	4,	5,	6,	6,	8,	8,	10,	10,	"Oriental (A)        " },
  {             0,	1,	1,	4,	4,	5,	6,	6,	9,	9,	10,	10,	"Oriental (B)        " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	9,	9,	10,	10,	"Overtone            " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	9,	9,	10,	10,	"Overtone Dominant   " },
  {             0,	1,	1,	3,	3,	7,	7,	7,	8,	8,	8,	8,	"Pelog               " },
  {             0,	1,	1,	4,	4,	5,	6,	6,	8,	8,	11,	11,	"Persian             " },
  {             0,	2,	2,	4,	4,	6,	6,	6,	9,	9,	10,	10,	"Prometheus          " },
  {             0,	1,	1,	4,	4,	6,	6,	6,	9,	9,	10,	10,	"PrometheusNeopolitan" },
// 81..90                                                                                                        <------------------>
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"Pure Minor          " },
  {             0,	1,	1,	4,	4,	6,	6,	7,	8,	8,	11,	11,	"Purvi Theta         " },
  {             0,	2,	2,	3,	3,	6,	6,	7,	9,	9,	10,	10,	"Roumanian Minor     " },
  {             0,	1,	1,	4,	4,	5,	8,	8,	8,	9,	9,	9,	"Six Tone Symmetrical" },
  {             0,	1,	1,	4,	4,	5,	7,	7,	8,	8,	10,	10,	"Spanish Gypsy       " },
  {             0,	1,	1,	3,	4,	6,	6,	8,	8,	10,	10,	10,	"Super Locrian       " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"Theta, Asavari      " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	11,	11,	"Theta, Bilaval      " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	8,	8,	11,	11,	"Theta, Bhairav      " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"Theta, Bhairavi     " },
// 91..100                                                                                                        <------------------>
  {             0,	2,	2,	3,	3,	5,	7,	7,	9,	9,	10,	10,	"Theta, Kafi         " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	9,	9,	11,	11,	"Theta, Kalyan       " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	10,	10,	"Theta, Khamaj       " },
  {             0,	1,	1,	4,	4,	6,	6,	7,	9,	9,	11,	11,	"Theta, Marva        " },
  {             0,	1,	1,	3,	3,	6,	6,	7,	8,	8,	11,	11,	"Todi Theta          " },
  {             0,	1,	2,	2,	5,	5,	7,	7,	8,	9,	9,	9,	"M. Bhavapriya 44    " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	9,	9,	10,	10,	"M. Chakravakam 16   " },
  {             0,	3,	3,	3,	4,	5,	7,	7,	10,	10,	10,	11,	"M. Chalanata 36     " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	8,	8,	10,	10,	"M. Charukesi 26     " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	10,	10,	10,	11,	"M. Chitrambari 66   " },
// 101..110                                                                                                        <------------------>
  {             0,	2,	2,	3,	3,	6,	6,	7,	9,	9,	11,	11,	"M. Dharmavati 59    " },
  {             0,	3,	3,	3,	4,	6,	6,	7,	8,	8,	11,	11,	"M. Dhatuvardhani 69 " },
  {             0,	1,	1,	4,	4,	6,	6,	7,	8,	9,	9,	9,	"M. Dhavalambari 49  " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	8,	8,	11,	11,	"M. Dhenuka 9        " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	11,	11,	"M. Dhirasankarabhara" },
  {             0,	1,	1,	3,	3,	6,	6,	7,	10,	10,	10,	11,	"M. Divyamani 48     " },
  {             0,	1,	1,	4,	4,	6,	6,	7,	9,	9,	11,	11,	"M. Gamanasrama 53   " },
  {             0,	1,	2,	2,	5,	5,	7,	7,	8,	8,	11,	11,	"M. Ganamurti 3      " },
  {             0,	3,	3,	3,	4,	5,	7,	7,	8,	8,	11,	11,	"M. Gangeyabhusani 33" },
  {             0,	2,	2,	3,	3,	5,	7,	7,	9,	9,	11,	11,	"M. Gaurimanohari 23 " },
// 111..120                                                                                                        <------------------>
  {             0,	1,	1,	3,	3,	6,	6,	7,	8,	9,	9,	9,	"M. Gavambodhi 43    " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	8,	9,	9,	9,	"M. Gayakapriya 13   " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"M. Hanumattodi 8    " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	9,	9,	10,	10,	"M. Harikambhoji 28  " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	10,	10,	10,	11,	"M. Hatakambari 18   " },
  {             0,	2,	2,	3,	3,	6,	6,	7,	9,	9,	10,	10,	"M. Hemavati 58      " },
  {             0,	1,	2,	2,	6,	6,	6,	7,	8,	8,	10,	10,	"M. Jalarnavam 38    " },
  {             0,	1,	2,	2,	6,	6,	6,	7,	9,	9,	10,	10,	"M. Jhalavarali 39   " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	9,	9,	10,	10,	"M. Jhankaradhvani 19" },
  {             0,	3,	3,	3,	4,	6,	6,	7,	8,	8,	10,	10,	"M. Jyotisvarupini 68" },
// 121..127                                                                                                        <------------------>
  {             0,	1,	1,	4,	4,	6,	6,	7,	8,	8,	11,	11,	"M. Kamavardhani 51  " },
  {             0,	1,	2,	2,	5,	5,	7,	7,	8,	9,	9,	9,	"M. Kanakangi 1      " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	8,	9,	9,	9,	"M. Kantamani 61     " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	9,	9,	10,	10,	"M. Kharaharapriya 22" },
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	11,	11,	"M. Kiravani 21      " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	9,	9,	11,	11,	"M. Kokilapriya 11   " },
  {             0,	3,	3,	3,	4,	6,	6,	7,	9,	9,	11,	11,	"M. Kosalam 71       " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	8,	8,	11,	11,	"M. Latangi 63       " },

  {             0,	1,	2,	2,	5,	5,	7,	7,	9,	9,	11,	11,	"M. Manavati 5       " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	8,	9,	9,	9,	"M. Mararanjani 25   " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	8,	9,	9,	9,	"M. Mayamalavagaula 1" },
  {             0,	2,	2,	4,	4,	6,	6,	7,	9,	9,	11,	11,	"M. Mechakalyani 65  " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	10,	10,	10,	11,	"M. Naganandini 30   " },
  {             0,	1,	1,	4,	4,	6,	6,	7,	8,	8,	10,	10,	"M. Namanarayani 50  " },
  {             0,	3,	3,	3,	4,	6,	6,	7,	9,	9,	10,	10,	"M. Nasikabhusani 70 " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	8,	8,	10,	10,	"M. Natabhairavi 20  " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	9,	9,	10,	10,	"M. Natakapriya 10   " },
  {             0,	1,	2,	2,	6,	6,	6,	7,	9,	9,	10,	10,	"M. Navanitam 40     " },
  {             0,	2,	2,	3,	3,	6,	6,	7,	10,	10,	10,	11,	"M. Nitimati 60      " },
  {             0,	1,	2,	2,	6,	6,	6,	7,	9,	9,	11,	11,	"M. Pavani 41        " },
  {             0,	3,	3,	3,	4,	5,	7,	7,	8,	8,	10,	10,	"M. Ragavardhani 32  " },
  {             0,	1,	2,	2,	6,	6,	6,	7,	10,	10,	10,	11,	"M. Raghupriya 42    " },
  {             0,	1,	1,	4,	4,	6,	6,	7,	9,	9,	10,	10,	"M. Ramapriya 52     " },
  {             0,	3,	3,	3,	4,	6,	6,	7,	10,	10,	10,	11,	"M. Rasikapriya 72   " },
  {             0,	1,	2,	2,	5,	5,	7,	7,	8,	8,	10,	10,	"M. Ratnangi 2       " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	8,	8,	10,	10,	"M. Risabhapriya 62  " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	10,	10,	10,	11,	"M. Rupavati 12      " },
  {             0,	1,	1,	3,	3,	6,	6,	7,	9,	9,	10,	10,	"M. Sadvidhamargini 4" },
  {             0,	1,	2,	2,	6,	6,	6,	7,	8,	9,	9,	9,	"M. Salagam 37       " },
  {             0,	2,	2,	3,	3,	6,	6,	7,	8,	8,	10,	10,	"M. Sanmukhapriya 56 " },
  {             0,	2,	2,	4,	4,	5,	7,	7,	8,	8,	11,	11,	"M. Sarasangi 27     " },
  {             0,	1,	1,	3,	3,	5,	7,	7,	8,	9,	9,	9,	"M. Senavati 7       " },
  {             0,	2,	2,	3,	3,	6,	6,	7,	8,	8,	11,	11,	"M. Simhendramadhyama" },
  {             0,	1,	1,	3,	3,	6,	6,	7,	8,	8,	11,	11,	"M. Subhapantuvarali " },
  {             0,	3,	3,	3,	4,	6,	6,	7,	8,	9,	9,	9,	"M. Sucharitra 67    " },
  {             0,	3,	3,	3,	4,	5,	7,	7,	9,	9,	11,	11,	"M. Sulini 35        " },
  {             0,	1,	1,	4,	4,	5,	7,	7,	9,	9,	11,	11,	"M. Suryakantam 17   " },
  {             0,	1,	2,	2,	6,	6,	6,	7,	9,	9,	11,	11,	"M. Suvarnangi 47    " },
  {             0,	2,	2,	3,	3,	6,	6,	7,	8,	9,	9,	9,	"M. Syamalangi 55    " },
  {             0,	1,	2,	2,	5,	5,	7,	7,	10,	10,	10,	11,	"M. Tanarupi 6       " },
  {             0,	2,	2,	4,	4,	6,	6,	7,	9,	9,	10,	10,	"M. Vaschaspati 64   " },
  {             0,	3,	3,	3,	4,	5,	7,	7,	9,	9,	10,	10,	"M. Vagadhisvari 34  " },
  {             0,	3,	3,	3,	4,	5,	7,	7,	8,	8,	10,	10,	"M. Vakulabharanam 14" },
  {             0,	1,	2,	2,	5,	5,	7,	7,	9,	9,	10,	10,	"M. Vanaspati 4      " },
  {             0,	2,	2,	3,	3,	5,	7,	7,	10,	10,	10,	11,	"M. Varunapriya 24   " },
  {             0,	3,	3,	3,	4,	6,	6,	7,	10,	10,	10,	11,	"M. Visvambari 54    " },
  {             0,	3,	3,	3,	4,	5,	7,	7,	8,	9,	9,	9,	"M. Yagapriya 31     " }
};

/* static */
uint8_t Scale::count() {
  return numbof(seq_scale_table);
}

/* static */
uint8_t Scale::get_note(uint8_t scale, uint8_t index) {
  uint8_t byte = pgm_read_byte(&seq_scale_table[scale].notes[index >> 1]);
  return (index & 1) ? U8ShiftRight4(byte) : (byte & 0xf);
}

/* static */
uint8_t Scale::GetScaledNote(uint8_t scale, uint8_t note) {
  // Normalize note
  uint8_t octave = 0;
  while(note >= 12) {
    note -= 12;
    ++octave;
  }

  // Get scaled note
  note = get_note(scale, note);

  // Restore octave
  while (octave > 0) {
    note += 12;
    --octave;
  }
  return note;
}

/* static */
uint8_t Scale::GetNextScaledNote(uint8_t scale, uint8_t note) {
  uint8_t scaled_note = note;
  uint8_t original_note = note;
  while (note < 127) {
    scaled_note = GetScaledNote(scale, ++note);
    if (scaled_note != original_note)
      break;
  }
  return scaled_note;
}

/* static */
uint8_t Scale::GetPrevScaledNote(uint8_t scale, uint8_t note) {
  uint8_t scaled_note = note;
  uint8_t original_note = note;
  while (note > 0) {
    scaled_note = GetScaledNote(scale, --note);
    if (scaled_note != original_note)
      break;
  }
  return scaled_note;
}

/* static */
void Scale::GetScaleName(uint8_t scale, uint8_t* buffer) {
  memcpy_P(buffer, &seq_scale_table[scale].name[0], kScaleNameSize);
}

} // namespace midialf
