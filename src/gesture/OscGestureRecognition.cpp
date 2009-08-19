/*
 * GestureStream.cpp
 *
 *  Created on: Jun 9, 2009
 *      Author: sashikanth
 */

#include <iostream>
#include <cstdlib>

#include "gesture/GestureCollector.h"
#include "touch/osc/OscHandler.h"
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

	virtual vector<string> gestureAction(const char* actionString, const char* actionParam)
	{
		cout << "Action: " << actionString << endl;
		vector<string> result;
		if(strcmp(actionString, "train") == 0)
		{
			if(samples.size() > 2) //Atleast 2 samples to train a gesture
				result = recognizer.trainWithSamples(samples, gestureName);
			samples.clear();
		}
		else if(strcmp(actionString, "classify") == 0)
		{
			//We require atleast 5 frames to classify a gesture.
			//Check if the gestures has no movement to avoid noise.
			if(currGestureSegment.isOnlyStatic() || currGestureSegment.size() < 10)
			{
				cout << "Ignoring Sample: Hasn't moved" << endl;
				return result;
			}
			vector<string> recognized = recognizer.classify(&currGestureSegment);

			result.push_back("recognized");
			for(size_t i = 0; i < recognized.size(); i++)
				result.push_back(recognized[i]);

			cout << "!! *** !! Recognized : " << recognized[0] << endl;
			currGestureSegment.clear();
			samples.clear(); //No reason to hold on to samples currently
		}
		else if(strcmp(actionString, "save") == 0)
		{
			cout << "Saving GestureSet as: " << actionParam << endl;
			recognizer.saveGestureSet(actionParam);
			result.push_back("saved");
			result.push_back(actionParam);
		}
		else if(strcmp(actionString, "load") == 0)
		{
			cout << "Loading GestureSet: " << actionParam << endl;
			recognizer.loadGestureSet(actionParam);
			result.push_back("loaded");
			result.push_back(actionParam);
		}
		else if(strcmp(actionString, "clear") == 0)
		{
			cout << "Clearing current GestureSet" <<endl;
			recognizer.clearGestureSet();
		}
		else
		{
			result.push_back("Action not supported");
		}
		return result;
	}

	virtual map<string, vector<double> > parameterize()
	{
		//Pick the latest frame of currGestureSegment, and return the parameterized map.
		return recognizer.parameterize(currGestureSegment.sample[currGestureSegment.size() - 1]);
	}
};

int main(int argc, char **argv)
{
	cout << "Collector Started" << endl;
	OscGestureRecognition oscRecog;
	initMultitouchOscReceiver(&oscRecog);
}

