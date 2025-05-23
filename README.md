# GameBoy
An original GameBoy emulator in C++ using SDL graphics with debugging support.

## Accuracy

This emulator is *reasonably* accurate; it passes Blargg's timing and CPU tests. It can also run most games I've tested without issues.
I've not attempted to implement hardware bugs or anything that a game reasonably shouldn't rely on (eg. missing objects during OAM DMA).

Consider [Sameboy](https://sameboy.github.io/) for accurate hardware emulation.

## Sound

Emulation is functional but noticeably imperfect.

## Controls

The default control mappings are:
| Function | Keyboard mapping |
| ------------- | ------------- |
| A | W |
| B | Q |
| SELECT | Enter |
| START | Space |
| D-PAD | Arrow Keys |
| Toggle Framecap | S |

## Screenshots
<img src="./screenshots/CPU_INSTRS.png" alt="Passes Blargg's CPU Instructions test" width="200"/>|<img src="./screenshots/INSTR_TIMING.png" alt="Passes Blargg's Instruction timing test" width="200"/>
| ------------- | ------------- |
<img src="./screenshots/MEM_TIMING.png" alt="Passes Blargg's Memory timing test" width="200"/>|<img src="./screenshots/STREET_FIGHTER.png" alt="Runs Street Fighter 2" width="200"/>

## Useful links

The following links point to documentation that I found useful.
- [Pan Docs](http://bgb.bircd.org/pandocs.htm)
- [GameBoy CPU manual](http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf)
- [CPU opcode reference](https://rednex.github.io/rgbds/gbz80.7.html)
- [GameBoy opcodes](https://pastraiser.com/cpu/gameboy/gameboy_opcodes.html)
- [Accurate timing information](https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf)

