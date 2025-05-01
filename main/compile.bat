g++ main.cpp  libs/workaround.cpp -L./libs/src -L./libs -I./libs  -I./libs/src  -D_POSIX_  -std=c++17  -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -pthread  -o RISCV_IDE
./RISCV_IDE
