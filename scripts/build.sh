#! /bin/bash

mkdir build
cd build
python3 ../configure.py -s tf2,sdk2013 --sm-path /sourcemod --mms-path /metamod-source --hl2sdk-root /sdks --enable-optimize
ambuild