# SMALL_IDE_FOR_MY_RISCV_CPU
This repository holds an assembler, mif generator , and a serial monitor for UART comunication with other devices.  


# TOOLS USED:
**RAYLIB:** was a big motivator to create this gui application, it makes working with a window much simpler.  
**RAYGUI:** with the immediate UI accelerated productivity with easy and simple to use UI.  
**WIN32 API:** the api helps with setting up a COM port, creating files, opening file dialogs... 
**UNISTD API:** for linux, the app was developped on ubunto so not sure if it works on all other flavours.
**MY OWN ASSEMBLER:** this was provided with the RISCV CORE that I made in VHDL, with some tweaks to handle errors and exeptions from the main codedirectly.**ADDED** flags for valid code and valid mif generation , and an error message that is empty as long as the code/mifs are valid else, it gives the problem with the code.  


**BUILD/COMPILATION**  
**WINDOWS**  
To compile the project you need some more steps:  
**FIRST:** you need to copy the whole **src** folder of **raylib(version 5.5)**, and paste it in the libs folder of this project,make sure that libraylib.a is present in the dll files in that src folder, best way to do that is to download raylib from itch.io.
**SECOND:** copy the **raygui** header file and paste inside the raylib src folder that you copied to libs folder of the project.
**LAST:** run the batch file COMPILE_WIN.BAT to compile the project with g++ 

in the end you should get a RISCV_IDE executable that runs the program.
command wmic win32pnp_entity is used to get usb to serial devices

**LINUX**  
on linux you need to install raylib with all its dependencies, for me i cloned the repo of raylib and built it using cmake.  
zenity is used for file dialogs,*udevadm* is used to get usb devices names. after calling *ls* ttyusb and ttyacm, this gets the usb to serial devices that this app can connect to. 
these commands must be supprted
run the compile batch file in otder to compile and execute the program
# THE FINISHED STRUCTURE AFTER ADDING RAYLIB AND RAYGUI 

main (folder)  
&emsp;&emsp;->main.cpp  
&emsp;&emsp;->louis_george_cafe  
&emsp;&emsp;&emsp;->(font files)  
&emsp;&emsp;->libs  
&emsp;&emsp;&emsp;->(assembler and workaround headers)  
&emsp;&emsp;&emsp;->src(copied from raylib version 5.5 and copy raygui header inside too!)    
      
This is kindof a project template for anyone that wants to use it.
