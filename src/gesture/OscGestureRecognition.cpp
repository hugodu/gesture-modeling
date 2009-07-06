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
#include "gesture/Gestures.h"


using namespace std;

/**
 * Read training and test messages from Osc
 * Use recognition helper class
 */
class OscGestureRecognition : public GestureCollector
{
	RecognitionHelper recognizer;
public:
	OscGestureRecognition()
	{

	}
	~OscGestureRecognition()
	{

	}

	virtual void gestureAction(const char* actionString, const char* actionParam)
	{
		cout << "Action: " << actionString << endl;
		if(strcmp(actionString, "train") == 0)
		{
			recognizer.trainWithSamples(samples, gestureName);
			samples.clear();
		}
		if(strcmp(actionString, "classify") == 0)
		{
			string recognized = recognizer.classify(currSample);
			cout << "!! *** !! Recognized : " << recognized << endl;
			currSample.clear();
			samples.clear(); //No reason to hold on to samples currently
		}
		if(strcmp(actionString, "save") == 0)
		{
			cout << "Saving GestureSet as: " << actionParam << endl;
			recognizer.saveGestureSet(actionParam);
		}
		if(strcmp(actionString, "load") == 0)
		{
			cout << "Loading GestureSet: " << actionParam << endl;
			recognizer.loadGestureSet(actionParam);
		}
	}
};

int main(int argc, char **argv)
{
	cout << "Collector Started" << endl;
	OscGestureRecognition oscRecog;
	initMultitouchOscReceiver(3333, &oscRecog);
}

