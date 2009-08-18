/*
 * GestureLearner.h
 *
 * Sample class implementing comm functionality with MultitouchListener.
 *
 *  Created on: Mar 27, 2009
 *      Author: sashikanth
 */

#ifndef GESTURECOLLECTOR_H_
#define GESTURECOLLECTOR_H_

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
	GestureSample currGestureSegment;
	string gestureName;
	std::vector<GestureSample> samples;

	void updateFrame(ContactSetFrame c)
	{
//		//If we are saving a segment, ensure subsequent frame has same number of fingers.
//		if(currGestureSegment.size() > 0 && currGestureSegment.lastFrameSize() != c.size())
//			return; //Ignore frames with different number of fingers.
		if(appendFrames)
			currGestureSegment.push_back(c);
	}

	void endSample()
	{
		appendFrames = false;

		if(currGestureSegment.size() > 0)
		{
			samples.push_back(currGestureSegment);
			cout << "Collected: " << samples.size() << " samples. Last sample size:" << currGestureSegment.size() << endl;
		}
		//		currSample.printSample();
	}

	void clearSample()
	{
		currGestureSegment.clear();
		appendFrames = false;
	}

	void startSample(const char* gestName)
	{
		//clear sample and start appending frames
		gestureName = gestName;
		cout << "Start Sample, Had frames: " << currGestureSegment.size() << endl;
//		for(size_t i = 0; i < currSample.size(); i++)
//			cout << "\t" << i << ": " << currSample.sample[i].size() << endl;
		currGestureSegment.clear();
		appendFrames = true;
	}

	/**
	 * The sample currently in the buffer has stopped moving.
	 * Check last frame with previous n frames with a tolerance.
	 */
	bool sampleIsNowStatic()
	{
		return currGestureSegment.isStatic(10);
	}

	/**
	 * If the sample consists only of static frames.
	 */
	bool sampleIsOnlyStatic()
	{
		return currGestureSegment.isOnlyStatic();
	}
	size_t sampleSize()
	{
		return currGestureSegment.size();
	}
	/*
	 * Default implementation of gesture actions
	 */
	virtual vector<string> gestureAction(const char* actionString, const char* actionParam)
	{
		return vector<string>();
	}

	virtual vector<double> parameterize()
	{
		return vector<double>();
	}
};
#endif /* GESTURELEARNER_H_ */
