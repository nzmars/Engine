#!/bin/bash
# ============================================================================
#  ORE build configuration - Linux / macOS
#
#  Every value below is a DEFAULT ONLY. To override, set the environment
#  variable before invoking the build script, e.g.:
#
#      BOOST_ROOT=/opt/boost_1_86_0 BUILD_TYPE=Debug ./scripts/build_linux.sh
#
#  A fresh checkout normally only needs BOOST_ROOT (unless building with
#  vcpkg: set flag_use_vcpkg=1 and put vcpkg at ../vcpkg).
#
#  Requires: CMake >= 3.15, a C++20 compiler, ninja, python3 + python3-dev,
#            and for the wheel: swig 4.x.  Boost 1.86 built static, or vcpkg.
# ============================================================================

# ---- Compiler -------------------------------------------------------------
export CC="${CC:-gcc}"
export CXX="${CXX:-g++}"

# ---- Repo layout (derived from this script's location) --------------------
: "${ORE_ROOT_DIR:=$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")/.." && pwd -P)}"
export ORE="$ORE_ROOT_DIR"
ORE_DIR="$ORE_ROOT_DIR"
: "${ORE_SWIG_DIR:=$ORE_ROOT_DIR/ORE-SWIG}"

# ---- Boost ---------------------------------------------------------------
: "${BOOST_ROOT:=$(cd "$ORE_ROOT_DIR/.." 2>/dev/null && pwd -P)/boost_1_86_0}"
BOOST="$BOOST_ROOT"
export BOOST_INC="${BOOST_INC:-$BOOST}"
export BOOST_LIB="${BOOST_LIB:-$BOOST/stage/lib}"

# ---- Build options -----------------------------------------------------
: "${GENERATOR:=Ninja}"
: "${BUILD:=build}"
: "${BUILD_TYPE:=Release}"
: "${BIN:=$ORE_ROOT_DIR/bin}"

# flag_use_vcpkg: 1 = use vcpkg (../vcpkg), 0 = use Boost at BOOST_ROOT
: "${flag_use_vcpkg:=0}"

# Parallel build jobs (default: half the cores, min 1; override with CPU_N=N)
_cores="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
: "${CPU_N:=$(( _cores > 1 ? _cores / 2 : 1 ))}"

echo "ORE_ROOT_DIR=$ORE_ROOT_DIR"
echo "ORE_SWIG_DIR=$ORE_SWIG_DIR"
echo "BOOST_ROOT=$BOOST_ROOT"
echo "GENERATOR=$GENERATOR  BUILD=$BUILD  BUILD_TYPE=$BUILD_TYPE  CPU_N=$CPU_N  vcpkg=$flag_use_vcpkg"
echo "BIN=$BIN"
