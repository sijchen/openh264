#!/bin/bash

 
for((i=0; i<500; i++))
do
  echo "loop Idx is $i"
  ./codec_unittest --gtest_filter=*CThreadPoolTest*
  echo "after lopp Idx $i"
done
