#!/bin/bash


log() {
  echo -n "INFO ---- "
  echo "$@"
}


. scripts/config.sh


echo "CPU_N=${CPU_N}"

if [ $flag_use_vcpkg -eq 1 ]; then
  VCPKG_ROOT=/mnt/d/code/model/ore/vcpkg
else
  echo BOOST_INC=${BOOST_ROOT}
  echo BOOST_LIB=${BOOST_INC}/stage/lib
fi



if [ $flag_use_vcpkg -eq 1 ]; then
  VCPKG_TOOLCHAIN=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
  VCPKG_TRIPLET=x64-linux

  pushd ${VCPKG_ROOT}
  ./vcpkg install --triplet=${VCPKG_TRIPLET} \
          boost-regex boost-serialization boost-system boost-test boost-thread boost-timer \
          boost-date-time boost-math boost-headers boost-ublas boost-accumulators boost-multi-array
  # ./vcpkg install boost
  popd

  CMAKE=${VCPKG_ROOT}/downloads/tools/cmake-3.29.2-linux/cmake-3.29.2-linux-x86_64/bin/cmake
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
if [ $flag_use_vcpkg -eq 1 ]; then
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
         -DBOOST_LIBRARYDIR=${BOOST_LIB}
fi

if [ $# -gt 0 ]; then
  echo build specific target with "$@"
  for var in "$@"
  do
    echo INFO === build project with $var
    ${CMAKE} --build ${BUILD} --parallel ${CPU_N} --verbose --target $var
  done
else
  ${CMAKE} --build ${BUILD} --parallel ${CPU_N} --verbose
fi

# unset CC
# unset CXX
