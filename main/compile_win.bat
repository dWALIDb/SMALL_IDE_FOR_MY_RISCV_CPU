g++ main.cpp -D_WIN32_ %CD%\libs\workaround.cpp -lraylib -lopengl32 -lcomdlg32 -lgdi32 -L%CD%\libs -L%CD%\libs\src -I%CD%\libs -I%CD%\libs\src -lwinmm -o RISCV_IDE.exe
RISCV_IDE