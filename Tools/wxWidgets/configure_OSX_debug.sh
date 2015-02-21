#! /bin/sh

SDK_ROOT=`xcrun -sdk macosx --show-sdk-path`
DEPLOYMENT_TARGET=10.8
CLANG=`xcrun -sdk macosx -find clang`
PREFIX=`pwd`

./trunk/configure \
 --with-osx_cocoa \
 --with-macosx-sdk=$SDK_ROOT \
 --with-macosx-version-min=$DEPLOYMENT_TARGET \
 --enable-universal-binary=i386,x86_64 \
 --disable-compat26 \
 --disable-compat28 \
 --disable-shared \
 --enable-unicode \
 --enable-monolithic \
 --enable-debug \
 --enable-debug_info \
 --enable-debug_gdb \
 --prefix=$PREFIX \
 CXXFLAGS="-std=c++11 -stdlib=libc++" \
 OBJCXXFLAGS="-std=c++11 -stdlib=libc++" \
 LDFLAGS="-stdlib=libc++"
