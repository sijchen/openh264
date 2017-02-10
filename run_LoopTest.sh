#!/bin/bash

CFLAGS=-Werror make -B ENABLE64BIT=Yes BUILDTYPE=Debug   all plugin test
 
for((i=0; i<500; i++))
do
  echo "loop Idx is $i"
  ./codec_unittest --gtest_filter=*CThreadPoolTest*
  echo "after lopp Idx $i"
done
