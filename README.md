# SMALL_IDE_FOR_MY_RISCV_CPU
This repository holds an assembler, mif generator , and a serial monitor for UART comunication with other devices.  


# TOOLS USED:
**RAYLIB:** was a big motivator to create this gui application, it makes working with a window much simpler.  
**RAYGUI:** with the immediate UI accelerated productivity with easy and simple to use UI.  
**WIN32 API:** the api helps with setting up a COM port, creating files, opening file dialogs...  
**MY OWN ASSEMBLER:** this was provided with the RISCV CORE that I made in VHDL, with some tweaks to handle errors and exeptions from the main codedirectly.**ADDED** flags for valid code and valid mif generation , and an error message that is empty as long as the code/mifs are valid else, it gives the problem with the code.  


**BUILD/COMPILATION**
To compile the project you need some more steps:  
**FIRST:** you need to copy the whole **src** folder of **raylib(version 5.5)**, and paste it in the libs folder of this project.
**SECOND:** copy the **raygui** header file and paste inside the raylib src folder that you copied to libs folder of the project.
**LAST:** run the following command in the terminal this links and compiles the project with g++ compiler:  
######
    g++ main.cpp %CD%\libs\workaround.cpp -lraylib -lopengl32 -lcomdlg32 -lgdi32 -L%CD%\libs -L%CD%\libs\src -I%CD%\libs -I%CD%\libs\src -lwinmm -mwindows -o RISCV_IDE.exe   

in the end you should get a RISCV_IDE executable that runs the program.

# THE FINISHED STRUCTURE AFTER ADDING RAYLIB AND RAYGUI 

main (folder)  
&emsp;->main.cpp  
&emsp;->louis_george_cafe  
&emsp;&emsp;->(font files)  
&emsp;->libs  
&emsp;&emsp;->(assembler and workaround headers)  
&emsp;&emsp;->src    
&emsp;&emsp;&emsp;->(raylib src and raygui header)  

      
  This is kindof a project template for anyone that wants to use it.
