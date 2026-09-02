#!/bin/bash

. scripts/config.sh

pushd $ORE_SWIG_DIR

echo BUILD ORESWIG

cd ${ORE_SWIG_DIR}/OREAnalytics-SWIG/Python

cmake -Wno-dev -B $BUILD -G $GENERATOR                            \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE                            \
        -DORE=$ORE_ROOT_DIR -DORE_BUILD=$ORE_ROOT_DIR/$BUILD      \
        -DORE_USE_ZLIB=OFF -DBoost_NO_SYSTEM_PATHS=ON             \
        -DBOOST_LIBRARYDIR=$BOOST_LIB                             \
        -DBOOST_ROOT=$BOOST -DBoost_ROOT=$BOOST                   \
        -DBUILD_SHARED_LIBS=OFF -DBoost_USE_STATIC_LIBS=ON
        # -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS_RELEASE='-O1 -DNDEBUG'

cmake --build $BUILD --parallel 2 --config $BUILD_TYPE --verbose 
# cmake --build $BUILD --parallel $CPU_N --config $BUILD_TYPE --verbose

python setup.py wrap
python setup.py build
python setup.py bdist_wheel

popd
