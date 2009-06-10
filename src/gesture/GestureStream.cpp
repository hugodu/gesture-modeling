/*
 * GestureStream.cpp
 *
 *  Created on: Jun 9, 2009
 *      Author: sashikanth
 */

#include <iostream>

#include <cstdlib>

#include "gesture/GestureCollector.h"
#include "touch/osc/MultitouchOSCReceiver.h"

using namespace std;

int main(int argc, char **argv)
{
	cout << "Collector Started" << endl;
	GestureCollector collector;
	initMultitouchOscReceiver(3333, collector);


}

