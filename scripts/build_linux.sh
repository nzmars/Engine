#!/bin/bash


log() {
  echo -n "INFO ---- "
  echo "$@"
}


_HERE="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd -P)"
. "$_HERE/config.sh"
cd "$ORE_ROOT_DIR" || exit 1

_OS="$(uname -s)"          # Linux | Darwin
_ARCH="$(uname -m)"        # x86_64 | arm64 | aarch64

echo "CPU_N=${CPU_N}  OS=${_OS}/${_ARCH}"

if [ "${flag_use_vcpkg:-0}" -eq 1 ]; then
  # portable abspath (no 'realpath' on stock macOS)
  VCPKG_ROOT="${VCPKG_ROOT:-$(cd "$ORE_ROOT_DIR/../vcpkg" 2>/dev/null && pwd -P)}"
  [ -x "${VCPKG_ROOT}/vcpkg" ] || { echo "ERROR: vcpkg not found (set VCPKG_ROOT or flag_use_vcpkg=0)" >&2; exit 1; }
  echo VCPKG_ROOT=$VCPKG_ROOT
else
  echo BOOST_INC=${BOOST_ROOT}
  echo BOOST_LIB=${BOOST_INC}/stage/lib
fi



if [ "${flag_use_vcpkg:-0}" -eq 1 ]; then
  VCPKG_TOOLCHAIN=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
  case "$_OS" in
    Darwin) case "$_ARCH" in arm64) VCPKG_TRIPLET=arm64-osx ;; *) VCPKG_TRIPLET=x64-osx ;; esac ;;
    *)      VCPKG_TRIPLET="${VCPKG_TRIPLET:-x64-linux}" ;;
  esac

  pushd ${VCPKG_ROOT}
  ./vcpkg install --triplet=${VCPKG_TRIPLET} \
          boost-regex boost-serialization boost-system boost-test boost-thread boost-timer \
          boost-date-time boost-math boost-headers boost-ublas boost-accumulators boost-multi-array
  # ./vcpkg install boost
  popd

  CMAKE="${CMAKE:-cmake}"        # use system cmake (vcpkg's bundled path is Linux-only)
  echo "=========== Build ORE project using vcpkg"
  echo "VCPKG_ROOT=${VCPKG_ROOT}"
else
  CMAKE=cmake
  echo "=========== Build ORE project using boost"
  echo "BOOST_INC=${BOOST_INC}"
  echo "BOOST_LIB=${BOOST_LIB}"
fi


${CMAKE} --version


         # -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${BIN}              \
         # -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${BIN}              \
         # -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${BIN}
if [ "${flag_use_vcpkg:-0}" -eq 1 ]; then
${CMAKE} -B $BUILD -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN}  \
         -DVCPKG_TARGET_TRIPLET=${VCPKG_TRIPLET}              \
         -DORE_BUILD_DOC=OFF                                  \
         -DORE_BUILD_TESTS=OFF                                \
         -DQL_BUILD_BENCHMARK=OFF                             \
         -DQL_BUILD_EXAMPLES=OFF                              \
         -DQL_BUILD_TEST_SUITE=OFF                            \
         -DQL_ENABLE_SESSIONS=ON                              \
         -DQL_USE_STD_CLASSES=OFF                             \
         -DBUILD_SHARED_LIBS=OFF                              \
         -DBoost_NO_SYSTEM_PATHS=ON
else 
# cmake_policy(SETCMP0144 NEW) is required for  BOOST_ROOT variable

         # -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${BIN}              \
         # -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${BIN}              \
         # -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${BIN}              \
${CMAKE} -B $BUILD                                            \
         -DORE_BUILD_DOC=OFF                                  \
         -DORE_BUILD_EXAMPLES=OFF                             \
         -DORE_BUILD_TESTS=OFF                                \
         -DORE_USE_ZLIB=OFF                                   \
         -DQL_BUILD_BENCHMARK=OFF                             \
         -DQL_BUILD_EXAMPLES=OFF                              \
         -DQL_BUILD_TEST_SUITE=OFF                            \
         -DQL_ENABLE_SESSIONS=ON                              \
         -DQL_USE_STD_CLASSES=OFF                             \
         -DBUILD_SHARED_LIBS=OFF                              \
         -DBOOST_ROOT=${BOOST_INC} -DBoost_ROOT=${BOOST_INC}  \
         -DBoost_NO_SYSTEM_PATHS=ON                           \
         -DCMAKE_INSTALL_PREFIX=$ORE                          \
         -DCMAKE_BUILD_TYPE=$BUILD_TYPE                       \
         -DBOOST_LIBRARYDIR=${BOOST_LIB}
fi

if [ $# -gt 0 ]; then
  echo build specific target with "$@"
  for var in "$@"
  do
    echo INFO === build project with $var
    ${CMAKE} --build ${BUILD} --parallel ${CPU_N} --verbose --target $var --config $BUILD_TYPE
  done
else
  ${CMAKE} --build ${BUILD} --parallel ${CPU_N} --verbose --config $BUILD_TYPE
fi

# unset CC
# unset CXX
