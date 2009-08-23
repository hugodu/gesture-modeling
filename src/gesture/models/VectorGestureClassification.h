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
#include <iostream>
#include <string>
#include <algorithm>
#include <math.h>
#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

#include <ame/patterns/serialize/task/filtered_classification.hpp>
#include <ame/observations/serialize/dynamic_vector.hpp>
#include <ame/patterns/serialize/model/ahmm.hpp>

#include <ame/patterns/task/filtered_classification.hpp>
//#include <ame/patterns/model/chain_skip_hmm.hpp>
#include <ame/patterns/model/gesture_hmm.hpp>
#include <ame/observations/training/dynamic_vector.hpp>
#include <ame/observations/training/normal.hpp>



using namespace std;

class reordering_filter
{
public:
	static const double  PI = 3.14159265;
	string feature[3]; //Number of features.
	vector<int> reassignments;
	int selectedFeat;

	reordering_filter()
	{
		feature[0]		= "Hor";
		feature[1]		= "Ver";
		feature[2]		= "Ang";
		selectedFeat 	= -1;

	}

	/**
	 * Frame consists of a sequence of x,y values of the contacts in the frame.
	 */
	vector<double> reorderFrame(const vector<double> & frame)
	const
	{
		vector<double> orderedFrame = frame;
		//Reorder the frame using reassignments.
		if(frame.size() == reassignments.size() * 2) // Since we use 2 features.
		{
			for(size_t i = 0; i*2 + 1< reassignments.size(); i++)
			{
				orderedFrame[reassignments[i] * 2] = frame[i * 2];
				orderedFrame[reassignments[i] * 2 + 1] = frame[i * 2 + 1];
			}
		}
//		else cout <<"Not reordering" <<endl;
		return orderedFrame;
	}

	/**
	 * allows reordering of a contact set frame
	 */
	ContactSetFrame reorderFrame(ContactSetFrame & frame)
	{
		ContactSetFrame orderedFrame = frame;
		if(frame.size() == reassignments.size())
		{
			for(size_t i = 0; i < reassignments.size(); i++)
			orderedFrame.frame[reassignments[i]] = frame.frame[i];
		}
		return orderedFrame;
	}

	double getScatterRatio(vector<vector<double> > featureValues)
	{
		unsigned int numFingers = featureValues[0].size();
		unsigned int numSamples = featureValues.size();
		vector<double> means;

		for(size_t sampleNum = 0; sampleNum < numSamples; sampleNum++ )
			for(size_t fingNum = 0; fingNum < numFingers; fingNum++)
			{
				if(sampleNum == 0)
					means.push_back(0);
				means[fingNum] += featureValues[sampleNum][fingNum];
			}
		double meanOfMeans = 0;
		for(size_t fingNum = 0; fingNum < numFingers; fingNum++)
		{
			means[fingNum] /= numSamples;
			meanOfMeans += means[fingNum];
		}
		meanOfMeans /= numFingers;

		double sWithin 	= 0;
		double sBetween = 0;
		for(size_t fingNum = 0; fingNum < numFingers; fingNum++)
		{
			for(size_t sampleNum = 0; sampleNum < numSamples; sampleNum++ )
			{
				double d = featureValues[sampleNum][fingNum] - means[fingNum];
				sWithin += d*d;
			}
			double meanDiff = means[fingNum] - meanOfMeans;
			sBetween +=  meanDiff * meanDiff * numSamples;
		}

		double scatterRatio = (sWithin > 1e-5) ? sBetween / sWithin : 0;
		cout << /*"Between: " << sBetween << "\tWithin: " << sWithin << */"\tScatter : " << scatterRatio << endl;
		return scatterRatio;
	}

	/**
	 * Select which ordering heuristic to use for this new gesture.
	 * Feature is selected by maximum ratio of Scatter-between fingers to scatter-within finger across samples.
	 */
	void selectFeature(const vector<vector<vector<double> > > &allSamples)
	{
		vector<double> scatterRatios;
		//Assuming Samples have been translated and scaled
		for(size_t featureNum = 0; featureNum < 3; featureNum++)
		{
			vector<vector<double> > featureVals;
			//Transform and extract feature values for all 3 heuristics.
			for(size_t sampleNum = 0; sampleNum < allSamples.size(); sampleNum++)
			{
				vector<double> frame = allSamples[sampleNum][0]; //Frame 1 of each sample
				vector<double> fVals = getFeatures(frame, featureNum);
				sort(fVals.begin(), fVals.end());
				featureVals.push_back(fVals);
			}
			scatterRatios.push_back(getScatterRatio(featureVals));
		}
		//		cout << "Scatter Ratios: ";
		//		for(size_t i =0; i < scatterRatios.size(); i++)
		//			cout << scatterRatios[i] << ", ";
		//		cout << endl;
		//Pick max scatterRatio
		if(scatterRatios[0] > scatterRatios[1] && scatterRatios[0] > scatterRatios[2])
			selectedFeat = 0;
		else if(scatterRatios[1] > scatterRatios[0] && scatterRatios[1] > scatterRatios[2])
			selectedFeat = 1;
		else
			selectedFeat = 2;
		cout << "---\tOrdering samples by: " << feature[selectedFeat] <<endl;

	}

	vector<double> getFeatures(const vector<double> &frame, int featureNum)
	{
		vector<double> fVals;
		for(size_t fingNum = 0; fingNum < frame.size(); fingNum += 2)
		{
			double value;
			switch(featureNum)
			{
			//feature = Hor
			case 0:	value = frame[fingNum]; break;//x
			case 1: value = frame[fingNum + 1]; break;//y
			case 2: value = atan2(frame[fingNum], frame[fingNum + 1]) * 180 / PI + 180; break; //angle of the contact from origin (mean)
			}
			fVals.push_back(value);
		}
		return fVals;
	}

	void setReassignments(vector<double> & frame1)
	{
		if(selectedFeat >= 0)
		{
			reassignments.clear();
			//cout << "Reassignments: ";
			vector<double> featureVals = getFeatures(frame1, selectedFeat);
			vector<double> sortedVals = featureVals;
			sort(sortedVals.begin(), sortedVals.end());
			for(size_t i = 0; i < featureVals.size(); i++)
				for(size_t j = 0; j < sortedVals.size(); j++)
					if(featureVals[i] == sortedVals[j]) //index i has to be shifted to index j
					{
						reassignments.push_back(j);
						//cout << " " << j;
					}
			//cout << endl;
		}
	}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & feature;
		ar & selectedFeat;
	}
};

class multitouch_filter
{
public:


	//Additional parameter required by the angular heuristic
	double angle;
	typedef vector<double> result_type;
	double xBounds, yBounds;
	double sX, sY; //Scale Params
	double tX, tY; // Translate Params
	reordering_filter reorder_filter;
	int numFingers;

	multitouch_filter(){}
	/**
	 * Instantiates the required parameters for this gesture.
	 * Determines ordering heuristic
	 */
	multitouch_filter(const vector<vector<vector<double> > >  &allSamples)
	{
		//Use the first frame of the first (representative) sample to set scale parameters for the filter.
		initParams(allSamples);
		reorder_filter.selectFeature(allSamples);
	}

	/**
	 * Performs the filter over each frame of the sample
	 */
	vector<double> operator()(const vector<double> &_frame) const
	{
		vector<double> frame = _frame;
		for(size_t i = 0; i + 1< frame.size(); i+=2)
		{
			frame[i] 		= frame[i] + tX;
			frame[i + 1] 	= frame[i + 1] + tY;
			frame[i] 		*= sX;
			frame[i + 1] 	*= sY;
		}
		vector<double> reorderedFrame = reorder_filter.reorderFrame(frame);
		return reorderedFrame;
	}

	/**
	 * Determines the translate and scale parameters for this sample
	 * The translate params ensure that the origin of the first frame is the mean of the points
	 * The scale params ensures that the first frame has the same bounds as xBounds and yBounds
	 */
	void reset_params_for(const vector<vector<double> > &sample)
	{
		//xy pairs
		//pick minx, miny, maxx, maxy
		vector<double> frame1 = sample[0];
		double meanX = 0, meanY = 0.;
		double minY = numeric_limits<double>::max();
		double minX = minY;
		double maxX = -minY;
		double maxY = -minY;
		//Iterate over contacts for frame1
		for(size_t i = 0; i + 1< frame1.size(); i+=2)
		{
			meanX += frame1[i];
			meanY += frame1[i + 1];
			minX = minX > frame1[i] 	? frame1[i] 	: minX;
			minY = minY > frame1[i + 1] ? frame1[i + 1] : minY;
			maxX = maxX < frame1[i] 	? frame1[i] 	: maxX;
			maxY = maxY < frame1[i + 1] ? frame1[i + 1] : maxY;
		}
		//cout << "\t\tminX:"<< minX << " minY:" << minY << " maxX:" << maxX << " maxY:" << maxY << endl;

		meanX /= frame1.size()/2.; //As of now size() is 2*numContacts.
		meanY /= frame1.size()/2.;
		//translate and scale. origin = meanX and meanY of points in frame1
		// Assuming that the samples are not centered.
		tX = -meanX; //If off center, add by tX,tY to center the frame
		tY = -meanY;
		sX = xBounds / (maxX - minX);
		sY = yBounds / (maxY - minY);
		//cout << "Params: sX:" << sX << " sY:" << sY << " tX:" << tX << " tY:" << tY << endl;

		reorder_filter.setReassignments(frame1);

	}

	bool accepts(const vector<vector<double> > &sample)
	{
		if(sample.size() > 1u && sample[0].size() == numFingers * 2u)
			return true;
		else
		{
			//cout << "\n---\nSample Ignored: Expected Dimensions: " << numFingers * 2 << ", have " << sample[0].size() << " with size: " << sample.size() << endl;
			return false;
		}
	}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & angle;
		ar & xBounds;
		ar & yBounds;
		ar & sX;
		ar & sY;
		ar & tX;
		ar & tY;
		ar & numFingers;
		ar & reorder_filter;
	}
private:
	void initParams(const vector<vector<vector<double> > > allSamples)
	{
		double xBMean = 0;
		double yBMean = 0;
		for(size_t i = 0; i < allSamples.size(); i++)
		{
			//Foreach sample, get bbox. Assign params to mean bbox size.
			vector<double> frame1 = allSamples[i][0]; // First frame of sample i;
			numFingers = frame1.size() / 2;
			//set xBounds and yBounds
			double bbox[2];
			boundingBox(frame1, bbox);
			xBMean += bbox[0];
			yBMean += bbox[1];
		}
		xBMean /= allSamples.size();
		yBMean /= allSamples.size();

		xBounds = xBMean;
		yBounds = yBMean;
		//Assuming that only x,y dimensions for each finger are being used.

		sX = sY = 1.;
		tX = tY = 0.;
		angle = 0.;
		cout << "NumFingers: " << numFingers << ". Bounds: [" << xBounds << ", " << yBounds << "]" << endl;
	}

	void boundingBox(const vector<double> frame1, double* bbox)
	{

		double minY = numeric_limits<double>::max();
		double minX = minY;
		double maxX = -minY;
		double maxY = -minY;
		for(size_t i = 0; i + 1 < frame1.size(); i += 2)
		{
			minX = minX > frame1[i] 	? frame1[i] 	: minX;
			minY = minY > frame1[i + 1] ? frame1[i + 1] : minY;
			maxX = maxX < frame1[i] 	? frame1[i] 	: maxX;
			maxY = maxY < frame1[i + 1] ? frame1[i + 1] : maxY;
		}
		bbox[0] = maxX - minX;
		bbox[1] = maxY - minY;
	}
};

namespace {

    typedef ame::patterns::model::gesture_hmm<ame::observations::dynamic_vector<std::vector<ame::observations::normal> >, long double> gesture_model_type;
    typedef ame::patterns::filtered_classification_task<gesture_model_type, multitouch_filter, ame::patterns::best_match_training> gesture_task_type;
    typedef std::vector<std::vector<double> > recording_type;
    typedef std::vector<recording_type> recordings_type;
}
class VectorGestureClassification
{
public:
    VectorGestureClassification()
    {
    	mClassificationTask = new gesture_task_type;
    	mLastRecognition = -1;
    }
    ~VectorGestureClassification()
    {
        delete static_cast<gesture_task_type *>(mClassificationTask);
    }

    void addGestureWithExamplesAndFilter(const vector<vector<vector<double> > > &examples, int num_states, multitouch_filter filter)
    {
    	addGestureWithExamples(examples, num_states);
    	    //Add the a filter for the model learnt above. It should be the last model.
    	    static_cast<gesture_task_type *>(mClassificationTask)
    	            ->
    					add_filter_for_pattern(filter,
    											static_cast<gesture_task_type *>(mClassificationTask) -> models().size() - 1);
    }
    void addGestureWithExamples(const vector<vector<vector<double> > > &examples, int num_states)
    {
    	 static_cast<gesture_task_type *>(mClassificationTask)
    	        ->
    	            add_pattern_with_examples
    	            (
    	                num_states,
    	                examples,
    	                ame::observations::training::dynamic_vector
    	                <
    	                    std::vector<ame::observations::normal>
    	                >
    	                (
    	                    ame::observations::training::normal(0.1)
    	                )
    	            );
    }
    int classify(const vector<vector<double> > &gesture)
    {
    	//Call reset params for all pairs with this sample
    		//Each filter is aware of what's to be done with the incoming sample
    		for(size_t i = 0; i <  static_cast<gesture_task_type *>(mClassificationTask)->get_num_pairs(); i++)
    		{
    			multitouch_filter* filter = &(static_cast<gesture_task_type *>(mClassificationTask)->get_filter(i));
    			if(filter->accepts(gesture))
    				filter->reset_params_for(gesture);
    		}

    	    return mLastRecognition =
    	        static_cast<gesture_task_type *>(mClassificationTask)
    	            ->
    	                classify(gesture);
    }

    void* getFilter(int filterIndex)
    {
    	return &(static_cast<gesture_task_type *>(mClassificationTask)->get_filter(filterIndex));
    }
    int numGestures() const
    {
        return
            static_cast<gesture_task_type *>(mClassificationTask)
                ->
                    get_num_pairs();
    }
    int lastRecognition() const
    {
    	return mLastRecognition;
    }
    const vector<long double> &probabilities() const
    {
        return static_cast<gesture_task_type *>(mClassificationTask)
            ->
                probabilities();
    }

    void clear()
    {
    	mClassificationTask = new gesture_task_type;
    	mLastRecognition = -1;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & *(static_cast<gesture_task_type *>(mClassificationTask));
    	ar & mLastRecognition;
    }

private:
    void *mClassificationTask;
	int mLastRecognition;
};

#endif
