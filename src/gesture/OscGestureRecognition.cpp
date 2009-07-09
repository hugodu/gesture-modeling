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
			if(samples.size() > 2)
				result = recognizer.trainWithSamples(samples, gestureName);
			samples.clear();
		}
		else if(strcmp(actionString, "classify") == 0)
		{
			vector<string> recognized = recognizer.classify(currSample);
			result.push_back("recognized");
			for(size_t i = 0; i < recognized.size(); i++)
				result.push_back(recognized[i]);

			cout << "!! *** !! Recognized : " << recognized[0] << endl;
			currSample.clear();
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

		return result;
	}
};

int main(int argc, char **argv)
{
	cout << "Collector Started" << endl;
	OscGestureRecognition oscRecog;
	initMultitouchOscReceiver(&oscRecog);
}

