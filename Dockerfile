FROM ubuntu:20.04

RUN apt update
RUN dpkg --add-architecture i386
RUN apt update && apt install -y git python3 python3-pip

RUN mkdir /sdks
RUN git clone https://github.com/alliedmodders/sourcemod --recurse-submodules -b 1.11-dev
RUN git clone https://github.com/alliedmodders/metamod-source --recurse-submodules -b 1.11-dev
RUN git clone https://github.com/alliedmodders/hl2sdk --recurse-submodules -b tf2 /sdks/hl2sdk-tf2
RUN git clone https://github.com/alliedmodders/hl2sdk --recurse-submodules -b sdk2013 /sdks/hl2sdk-sdk2013
RUN git clone https://github.com/alliedmodders/ambuild --recurse-submodules

RUN apt install -y \
    g++-multilib \
    libtool \
    nasm \
    libiberty-dev:i386 \
    libelf-dev:i386 \
    libboost-dev:i386 \
    libbsd-dev:i386 \
    libunwind-dev:i386 \
    lib32stdc++-10-dev \
    lib32z1-dev \
    libc6-dev:i386 \
    linux-libc-dev:i386 \
    libopus-dev:i386

RUN git clone https://github.com/xiph/opus.git -b v1.3.1 --depth 1
RUN cd opus && ./autogen.sh && ./configure CFLAGS="-m32 -g -O2" LDFLAGS=-m32 && make && make install && cd ..

RUN pip install ./ambuild