#!/bin/sh

export LD_LIBRARY_PATH=$(pwd)/libs/
echo $LD_LIBRARY_PATH
echo "\n--- Starting Session ---" >> multitouch-gestr.log
date >> multitouch-gestr.log
echo "\n--- ---" >> multitouch-gestr.log
./multitouch-gestr >> multitouch-gestr.log & 


echo "\n--- Starting Session ---" >> mgestr.jar.log
date >> mgestr.jar.log
echo "\n--- ---" >> mgestr.jar.log
java -jar mgestr.jar >> mgestr.jar.log &
