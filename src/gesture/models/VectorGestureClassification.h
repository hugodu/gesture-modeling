/*
 *  Author:
 *  	Sashikanth Damaraju
 *      &
 *      Stjepan Rajko
 *      Arts, Media and Engineering Program
 *      Arizona State University
 *
 *  Copyright 2008 Arizona Board of Regents.
 *
 *  This file is part of the AME Patterns openFrameworks addon.
 *
 *  The AME Patterns openFrameworks addon is free software: you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, either version 3 of the License,
 *  or (at your option) any later version.
 *
 *  The AME Patterns openFrameworks addon is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the AME Patterns openFrameworks addon.
 *  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef VECTORGESTURECLASSIFICATION
#define VECTORGESTURECLASSIFICATION

#include <vector>
#include <limits>

class scale_filter
{
public:
	typedef std::vector<double> result_type;
	double xBounds, yBounds;
	double sX, sY; //Scale Params
	double tX, tY; // Translate Params

	/**
	 * Sets the bounds of the first frame of the 'representative' sample
	 */

	scale_filter(const std::vector<double> &frame1)
	{
		//set xBounds and yBounds
		double minY = std::numeric_limits<double>::max();
		double minX = minY;
		double maxX = -minY;
		double maxY = -minY;
		for(size_t i = 0; i + 1 < frame1.size(); i += 2)
		{
			minX = minX > frame1[i] 	? frame1[i] 	: minX;
			minY = minY > frame1[i + 1] ? frame1[i + 1] : minY;
			maxX = maxX < frame1[i] 	? frame1[i] 	: maxX;
			minY = minY < frame1[i + 1] ? frame1[i + 1] : minY;
		}
		xBounds = maxX - minX;
		yBounds = maxY - minY;
		sX = sY = 1.;
		tX = tY = 0.;
	}

	/**
	 * Performs the filter over each frame of the sample
	 */
	std::vector<double> operator()(const std::vector<double> &_frame) const
	{
		std::vector<double> frame = _frame;
		for(size_t i = 0; i + 1< frame.size(); i+=2)
		{
			frame[i] 		+= tX;
			frame[i + 1] 	+= tY;
			frame[i] 		*= sX;
			frame[i + 1] 	*= sY;
		}
		return frame;
	}

	/**
	 * Determines the translate and scale parameters for this sample
	 * The translate params ensure that the origin of the first frame is the mean of the points
	 * The scale params ensures that the first frame has the same bounds as xBounds and yBounds
	 */
	void reset_params_for(const std::vector<std::vector<double> > &sample)
	{
		//xy pairs
		//pick minx, miny, maxx, maxy
		std::vector<double> frame1 = sample[0];
		double meanX = 0, meanY = 0.;
		double minY = std::numeric_limits<double>::max();
		double minX = minY;
		double maxX = -minY;
		double maxY = -minY;
		for(size_t i; i + 1< frame1.size(); i+=2)
		{
			meanX += frame1[i];
			meanY += frame1[i + 1];
			minX = minX > frame1[i] ? frame1[i] : minX;
			minY = minY > frame1[i + 1] ? frame1[i + 1] : minY;
			maxX = maxX < frame1[i] ? frame1[i] : maxX;
			minY = minY < frame1[i + 1] ? frame1[i + 1] : minY;
		}
		meanX /= frame1.size()/2.; //As of now size() is 2*numContacts.
		meanY /= frame1.size()/2.;
		//translate and scale ? origin = meanX and meanY of points in frame1
		// Assuming that the samples are not centered.
		tX = -meanX; //If off center, add by tX,tY to center the frame
		tY = -meanY;
		sX = xBounds / (maxX - minX);
		sY = yBounds / (maxY - minY);
	}
};

class VectorGestureClassification
{
public:
    VectorGestureClassification();
    ~VectorGestureClassification();

    void addGestureWithExamplesAndFilter(const std::vector<std::vector<std::vector<double> > > &examples, int num_states, scale_filter filter);
    void addGestureWithExamples(const std::vector<std::vector<std::vector<double> > > &examples, int num_states);
    int classify(const std::vector<std::vector<double> > &gesture);
    int numGestures() const;
    int lastRecognition() const
    {   return mLastRecognition; }
    const std::vector<long double> &probabilities() const;
private:
    void *mClassificationTask;
    int mLastRecognition;
};

#endif
