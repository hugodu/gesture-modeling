#!/bin/sh

export LD_LIBRARY_PATH=$(pwd)/libs/
echo $LD_LIBRARY_PATH
./multitouch-gestr
