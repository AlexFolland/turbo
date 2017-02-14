#!/bin/sh
cd -- "$(dirname -- "$0")"

g++ -ffast-math -O3 -o turbo.exe turbo.cpp
echo "Press any keyboard key to close the window."
read