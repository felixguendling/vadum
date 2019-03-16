# Cross Compile

On Ubuntu 18.04.

Install mingw:
```bash
sudo apt install mingw-w64
```

Get and unpack Boost, then:
```bash
./booststrap.sh
echo "using gcc : : x86_64-w64-mingw32-g++ ;" > user-config.jam
./b2 -j6 --user-config=user-config.jam \
                       toolset=gcc-mingw \
                       target-os=windows \
                       link=static \
                       threadapi=win32 \
                       address-model=64 \
                       variant=release \
        --with-system \
        --with-filesystem \
        --with-iostreams \
        --with-program_options \
        --with-thread \
        --with-date_time \
        --with-regex \
        --with-serialization \
        -s NO_BZIP2=1
```

Create toolchain for cmake:
```
set(CMAKE_SYSTEM_NAME Windows)

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})
```

In project root:
```bash
mkdir build-mingw && cd build-mingw
BOOST_ROOT=~/lib/boost_1_69_0-mingw cmake .. -DCMAKE_TOOLCHAIN_FILE=~/code/mingw-toolchain.cmake -DCMAKE_CXX_FLAGS=-static -DCMAKE_BUILD_TYPE=MinSizeRel
make
```
