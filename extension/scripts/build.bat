COPY C:\extension\scripts\setup.py C:\ambuild\setup.py
pip install C:\ambuild

mkdir build
cd build

python ..\configure.py -s tf2,sdk2013 --sm-path C:\sourcemod --mms-path C:\metamod-source --hl2sdk-root C:\sdks --enable-optimize
ambuild