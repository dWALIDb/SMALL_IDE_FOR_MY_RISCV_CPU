# SMALL_IDE_FOR_MY_RISCV_CPU
This repository holds an assembler, mif generator , and a serial monitor for UART comunication with other devices.
# TOOLS USED:
**RAYLIB:** was a big motivator to create this gui application, it makes working with a window much simpler.  
**RAYGUI:** with the immediate UI accelerated productivity with easy and simple to use UI.  
**WIN32 API:** the api helps with setting up a COM port, creating files, opening file dialogs...  
**MY OWN ASSEMBLER:** this was provided with the RISCV CORE that I made in VHDL, with some tweaks to handle errors and exeptions from the main codedirectly.  
**ADDED** flags for valid code and valid mif generation , and an error message that is empty as long as the code/mifs are valid, else it gives the problem with the code.
