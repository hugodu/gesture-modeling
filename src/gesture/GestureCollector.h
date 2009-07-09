/*
 * GestureLearner.h
 *
 * Sample class implementing comm functionality with MultitouchListener.
 *
 *  Created on: Mar 27, 2009
 *      Author: sashikanth
 */

#ifndef GESTURECOLLECTOR_H_
#define GESTUREECOLLECTOR_H_

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <cstdlib>

#include "touch/Touch.h"

class GestureCollector
{
	bool appendFrames;
public:

	GestureCollector()
	{
	}
	~GestureCollector()
	{
	}
	GestureSample currSample;
	string gestureName;
	std::vector<GestureSample> samples;

	void updateFrame(ContactSetFrame c)
	{
		if(appendFrames)
			currSample.push_back(c);
	}

	void endSample()
	{
		appendFrames = false;
		samples.push_back(currSample);
		cout << "Collected: " << samples.size() << " samples" << endl;
//		std::cout << "\n\nPrinting Sample:\n" << std::endl;
//		currSample.printSample();
	}

	void startSample(const char* gestName)
	{
		//clear sample and start appending frames
		gestureName = gestName;
		currSample.clear();
		appendFrames = true;
	}

	/*
	 * Default implementation of gesture actions
	 */
	virtual vector<string> gestureAction(const char* actionString, const char* actionParam)
	{
		vector<string> result;
		return result;
	}
};
#endif /* GESTURELEARNER_H_ */
