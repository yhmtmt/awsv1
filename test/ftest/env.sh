#!/bin/sh

cur_path=`pwd`

export PATH=${PATH}:${cur_path}/../bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${cur_path}/../lib
