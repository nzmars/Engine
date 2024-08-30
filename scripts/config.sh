#!/BIN/bash
#
# find_package(Python REQUIRED COMPONENTS Development)
# sudo apt-get install python3-dev

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
export ORE=`pwd -P`                 # required
ORE_ROOT_DIR=$ORE
ORE_DIR=$ORE_ROOT_DIR
ORE_SWIG_DIR=`realpath ../ORE-SWIG`


BOOST_ROOT=`realpath ../boost_1_86_0`
BOOST=$BOOST_ROOT
export BOOST_INC=$BOOST             # required
export BOOST_LIB=$BOOST/stage/lib   # required
# BOOST_LIBRARYDIR=$BOOST_LIB


# export LDFLAGS='-Wl,-Bstatic'
# export CXXFLAGS='-NDEBUG'


GENERATOR=Ninja
BUILD=build
BUILD_TYPE=Release
BIN=`pwd -P`/bin

# ------------------------------------------------------------------
# flag_use_vcpkg: 1 (use vcpkg), 0(use boost)
# ------------------------------------------------------------------
flag_use_vcpkg=0

cores=$(nproc)
CPU_N=$((cores/2))
CPU_N=10


echo ORE_ROOT_DIR=$ORE_ROOT_DIR
echo BUILD=$BUILD   BIN=$BIN
