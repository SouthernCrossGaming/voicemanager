#! /bin/bash

mkdir build
cd build
python3 ../configure.py -s tf2 --sm-path /sourcemod --mms-path /metamod-source --hl2sdk-root /sdks
ambuild