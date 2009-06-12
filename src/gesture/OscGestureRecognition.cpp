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

/**
 * Read training and test messages from Osc
 * Use recognition helper class
 */
class OscGestureRecognition : public GestureCollector
{
public:
	OscGestureRecognition()
	{

	}
	~OscGestureRecognition()
	{

	}
	virtual void gestureAction(const char* actionString)
	{
		cout << "Action: " << actionString << endl;
	}
};

int main(int argc, char **argv)
{
	cout << "Collector Started" << endl;
	OscGestureRecognition oscRecog;
	initMultitouchOscReceiver(3333, &oscRecog);


}

