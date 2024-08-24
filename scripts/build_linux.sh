#!/bin/bash


log() {
  echo -n "INFO ---- "
  echo "$@"
}


ORE_ROOT_DIR=`pwd -P`

VCPKG_ROOT=/mnt/d/code/model/ore/vcpkg
VCPKG_TOOLCHAIN=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
VCPKG_TRIPLET=x64-linux

# ------------------------------------------------------------------
# flag_vcpkg: 1 (use vcpkg), 0(use boost)
# ------------------------------------------------------------------
flag_vcpkg=0



if [ $flag_vcpkg -eq 1 ]; then
pushd ${VCPKG_ROOT}
./vcpkg install --triplet=${VCPKG_TRIPLET} \
        boost-regex boost-serialization boost-system boost-test boost-thread boost-timer \
        boost-date-time boost-math boost-headers boost-ublas boost-accumulators boost-multi-array
# ./vcpkg install boost
popd

CMAKE=${VCPKG_ROOT}/downloads/tools/cmake-3.29.2-linux/cmake-3.29.2-linux-x86_64/bin/cmake
echo "=========== Build ORE project using vcpkg"

else
  BOOST_INC=/mnt/d/code/model/ore/lboost/include
  BOOST_LIB=/mnt/d/code/model/ore/lboost/lib
  CMAKE=cmake
echo "=========== Build ORE project using boost"
fi


${CMAKE} --version

echo "VCPKG_ROOT=${VCPKG_ROOT}"
echo "ORE_ROOT_DIR=${ORE_ROOT_DIR}"
if [ $flag_vcpkg -eq 0 ]; then
  echo "BOOST_INC=${BOOST_INC}"
  echo "BOOST_LIB=${BOOST_LIB}"
fi

cores=$(nproc)
cpu_n=$((cores/2))
cpu_n=8
echo "cpu_n=${cpu_n}"

OUT=build-linux
bin=`pwd -P`/bin

echo "OUT=${OUT}   bin=${bin}"


if [ $flag_vcpkg -eq 1 ]; then
${CMAKE} -B $OUT -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN}  \
         -DVCPKG_TARGET_TRIPLET=${VCPKG_TRIPLET}            \
         -DORE_BUILD_DOC=OFF                                \
         -DORE_BUILD_TESTS=OFF                              \
         -DQL_BUILD_BENCHMARK=OFF                           \
         -DQL_BUILD_EXAMPLES=OFF                            \
         -DQL_BUILD_TEST_SUITE=OFF                          \
         -DQL_ENABLE_SESSIONS=ON                            \
         -DQL_USE_STD_CLASSES=OFF                           \
         -DBUILD_SHARED_LIBS=OFF                            \
         -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${bin}            \
         -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${bin}            \
         -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${bin}
else 
# cmake_policy(SETCMP0144 NEW) is required for  BOOST_ROOT variable
${CMAKE} -B $OUT                                            \
         -DORE_BUILD_DOC=OFF                                \
         -DORE_BUILD_TESTS=OFF                              \
         -DQL_BUILD_BENCHMARK=OFF                           \
         -DQL_BUILD_EXAMPLES=OFF                            \
         -DQL_BUILD_TEST_SUITE=OFF                          \
         -DQL_ENABLE_SESSIONS=ON                            \
         -DQL_USE_STD_CLASSES=OFF                           \
         -DBUILD_SHARED_LIBS=OFF                            \
         -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${bin}            \
         -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${bin}            \
         -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${bin}            \
         -DBOOST_ROOT=${BOOST_INC} -DBoost_ROOT=${BOOST_INC}\
         -DBOOST_LIBRARYDIR=${BOOST_LIB}
fi

if [ $# -gt 0 ]; then
  echo build specific target with "$@"
  for var in "$@"
  do
    echo INFO === build project with $var
    ${CMAKE} --build ${OUT} --parallel ${cpu_n} --verbose --target $var
  done
else
  ${CMAKE} --build ${OUT} --parallel ${cpu_n} --verbose
fi

