# Copyright 2011 Olivier Gillet.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

MCU_NAME       = 644
TARGET         = midialf
PACKAGES       = avrlib avrlib/i2c avrlib/devices midialf midialf/ui_pages midialf/cv
RESOURCES      = midialf/resources
# -DMIDIOUT_DEBUG_OUTPUT
# -DMIDILED_DEBUG_OUTPUT
# -DENABLE_CV_OUTPUT
EXTRA_DEFINES  = -DDISABLE_DEFAULT_UART_RX_ISR -DENABLE_CV_OUTPUT -DxMIDIALF_V01

LOCK           = 2f
LFUSE          = ff
HFUSE          = d6
EFUSE          = fd

include avrlib/makefile.mk

include $(DEP_FILE)
