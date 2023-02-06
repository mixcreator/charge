#!/bin/sh


LD_LIBRARY_PATH=./libs/arm  ./chademo can0 /dev/ttyUSB0
#https://eax.me/valgrind/
#LD_LIBRARY_PATH=./libs/arm  valgrind --leak-check=full --show-leak-kinds=all ./chademo can0 /dev/ttyUSB0

#LD_LIBRARY_PATH=./libs/arm  valgrind --leak-check=full --track-origins=yes ./chademo can0 /dev/ttyUSB0