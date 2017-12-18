@call \DIY\SetVars.cmd
 make -f midialf\makefile %1
 if errorlevel 1 goto EOF
 if {%1} == {clean} goto EOF
 avr-objdump -h -S build\midialf\midialf.elf > build\midialf\midialf.lst
 set path=%path%;C:\Program Files (x86)\Atmel\AVR Tools\STK500
 stk500 -cUSB -dATmega644pa -I4Mhz -e -ifbuild\midialf\midialf.hex -pf -vf
:EOF




