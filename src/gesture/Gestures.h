/*
 * Gestures.h
 *
 *  Created on: Jun 8, 2009
 *      Author: damaraju
 */

#ifndef GESTURES_H_
#define GESTURES_H_

#include <map>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <touch/Touch.h>
#include <gesture/models/VectorGestureClassification.h>
#include <gesture/GestureParameterization.h>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <boost/ptr_container/serialize_ptr_map.hpp>

using namespace std;
//For debug and test
string base = "data/reOrdered/";

void printTransform(vector<vector<double> > transformed)
{
	cout << "Transformed Size: " << transformed.size() << endl;

	for(size_t i = 0; i < transformed.size(); ++i)
	{
		for (size_t j = 0; j < transformed.at(i).size(); ++j)
			cout << i << ": " << transformed.at(i).at(j) << "\t";
		cout << endl;
	}
}

vector<GestureSample> readFile(string fileName)
{
	vector<GestureSample> readSamples;
	ifstream readFile;

	cout << "Reading: " << fileName << " " << endl;
	readFile.open(fileName.c_str());
	string line;


	if (readFile.is_open())
	{
		while(!readFile.eof())
		{
			getline(readFile,line);
			GestureSample sample = GestureSample(line);

			if(sample.size() > 0)
				readSamples.push_back(sample);
		}
		cout << "Num Samples: " << readSamples.size() << endl;
		readFile.close();
	}
	else
	{
		cout << "Unable to read file: " << fileName << endl;
	}
	return readSamples;

}
vector<GestureSample> readGestureSet(string gid, string uid)
{
	string fileName = "gid_" + gid + "_uid_" + uid + ".seqs";
	return readFile(base + fileName);
}


vector<vector<vector<double> > > transformSamples(vector<GestureSample> samples)
{
	vector<vector<vector<double> > > transformedSet;
	//cout << "Transforming " << samples.size() << " samples" << endl;

	if (samples.size() > 0)
		for (size_t i = 0; i < samples.size(); ++i)
			transformedSet.push_back(samples.at(i).transform());
	else
		cout << "No Samples" << endl;

	return transformedSet;
}

class RecognitionHelper
{
public:
	typedef boost::ptr_map<string, gesture_parameterization> mapGP;

	RecognitionHelper()
	:currentParameterization(0),reorderFilter(0),paused(false),resetOrdering(false)
	{}
	vector<string> trainWithSamples(const vector<GestureSample> trainingSet, string gestureName)
	{
		vector<string> result;
		VectorGestureClassification tempClassifier;

		vector<vector<vector<double> > > trnsfTrain = transformSamples(trainingSet);
		//Magical filter takes care of all preprocessing, and live processing of the incoming sample(s).
		multitouch_filter filter = multitouch_filter(trnsfTrain);

		//Ensure the training set is appropriately scaled for training the model.
		vector<vector<vector<double> > > filteredTraining = trnsfTrain;
		for(size_t i = 0; i < trnsfTrain.size() - 1; i++)
		{
			filter.reset_params_for(trnsfTrain[i]);
			boost::copy(trnsfTrain[i] | boost::adaptors::transformed(filter),filteredTraining[i].begin());
		}

		cout << "Training With: " << filteredTraining.size() << " samples." << endl;
		const unsigned int MIN_STATE_SIZE = 2;
		const unsigned int MAX_STATE_SIZE = 9;
		cout << "States[";
		for(size_t i = MIN_STATE_SIZE; i <= MAX_STATE_SIZE; i++)
		{
			tempClassifier.addGestureWithExamplesAndFilter(filteredTraining, i, filter);
			cout << i << ", ";
			flush(cout);
		}
		cout << "]" << endl;
		int classIndex = tempClassifier.classify(trnsfTrain[trnsfTrain.size()-1]);

		vector<long double> probs = tempClassifier.probabilities();
		cout << "Probabilities for last sample is :: ";
		for(unsigned int i = 0 ; i < probs.size(); i++)
			cout << probs[i] << ", ";

		classifier.addGestureWithExamplesAndFilter(filteredTraining, classIndex + MIN_STATE_SIZE, filter);

		gestureNameMap.insert(pair<int, string>(classifier.numGestures() - 1, gestureName));
		cout << "\nAdded new gesture(" << classifier.numGestures() - 1 << "): " << gestureName << " [" << classIndex + MIN_STATE_SIZE << " states]\n"<<endl;
		//char* stateString;
		//sprintf(stateString, "%d", classIndex + MIN_STATE_SIZE);
		result.push_back("trained");
		result.push_back(gestureName);
		result.push_back(boost::lexical_cast<std::string>(classIndex + MIN_STATE_SIZE));

		return result;
	}

	/**
	 * parameterStrings are of the following format, with 3 strings:
	 * gesture_name parameter_name parameter_string
	 */
	typedef pair<string, string> namedPairT;
	vector<string> addParameterToGesture(vector<string> parameterStrings)
	{
    	mapGP::iterator iter;
    	string gestureName = parameterStrings[0];
    	iter = gestureNameToParameterizationMap.find(gestureName);
    	namedPairT namedParamStringPair(parameterStrings[1], parameterStrings[2]);
    	if(iter == gestureNameToParameterizationMap.end())
    	{
    		//No parameters exist for this gesture. Create a gesture_parameterization object with this parameter
//    		gesture_parameterization parameterization(namedParamStringPair);
    		gestureNameToParameterizationMap.insert(gestureName, new gesture_parameterization(namedParamStringPair));
    	}
    	else
    		iter->second->addParameter(namedParamStringPair);

    	vector<string> result;
    	result.push_back("Parameter " + parameterStrings[1] +
    			" successfully added to gesture: " + parameterStrings[0] +
    			" with parameter string: " + parameterStrings[2]);
    	return result;
	}

	/**
	 *
	 */
	vector<string> classify(GestureSample* sample)
	{
		vector<string> result;
		vector<vector<double> > transformed = sample->transform();

		int classIndex = classifier.classify(transformed);
		vector<long double> probs = classifier.probabilities();

		bool allZero = true;
		for(unsigned int i = 0 ; i < probs.size(); i++)
		{
			if(allZero && probs[i] > 0)
				allZero = false;
			cout << probs[i] << ", ";
		}
		cout << endl;

		if(!allZero && classIndex >= 0 && ((unsigned int)classIndex) < gestureNameMap.size())
		{
			multitouch_filter* filter = static_cast<multitouch_filter *>(classifier.getFilter(classIndex));
			string lastGesture = gestureNameMap[classIndex];

	    	mapGP::iterator iter = gestureNameToParameterizationMap.find(lastGesture);
	    	if(iter != gestureNameToParameterizationMap.end())
	    	{
	    		currentParameterization = iter->second; //Set the object to parameterize from now.
	    		reorderFilter			= &(filter->reorder_filter);
	    		cout << "Gesture is parameterized with: " << currentParameterization->namedParamsMap.size() << " parameters." << endl;
	    	}
	    	else
	    		cout << "No parameters for this gesture" << endl;

			result.push_back(lastGesture);
			result.push_back(boost::lexical_cast<string>(-filter->tX));
			result.push_back(boost::lexical_cast<string>(-filter->tY));
		}
		else //classIndex == -1 || allZero probabilities when sample doesn't match any filter-model pair
			result.push_back("None");
		cout << "Parameterization begin. Ordering by: " << reorderFilter->feature[reorderFilter->selectedFeat] << endl;
		return result;

	}

	/**
	 * The current frame is converted to a sequence of parameters
	 */
    map<string, vector<double> > parameterize(ContactSetFrame & frame)
    {

    	if(currentParameterization && !paused)
    	{
    		if(resetOrdering)
    		{
    			vector<double> transformedFrame = frame.transform();
    			reorderFilter->setReassignments(transformedFrame);
    			resetOrdering = false;
    		}
    		ContactSetFrame orderedFrame = reorderFilter->reorderFrame(frame);
   			return currentParameterization->operator()(orderedFrame);
    	}
    	return map<string, vector<double> >();
    }

    void unParameterize()
    {
    	currentParameterization = 0;
    	reorderFilter 			= 0;
    }
    bool isParameterizationPaused()
    {
    	return isCurrentlyParameterized() && paused;
    }
    void pauseParameterization()
    {
    	paused = true;
    }
    void unpauseParameterization()
    {
    	paused = false;
    	resetOrdering = true;
    }
	const vector<long double> &probabilities() const
	{
	    return classifier.probabilities();
	}
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & classifier;
        ar & gestureNameMap;
        ar & gestureNameToParameterizationMap;
    }

    void saveGestureSet(string appName)
    {
    	ofstream storage(appName.c_str(), ios::out | ios::binary);
    	boost::archive::binary_oarchive out(storage);
    	out << *this;
    }

    void loadGestureSet(string appName)
    {
    	ifstream storage(appName.c_str(), ios::in | ios::binary);
    	boost::archive::binary_iarchive in(storage);
    	in >> *this;
    }
    void clearGestureSet()
    {
    	gestureNameMap.clear();
    	classifier.clear();
    }

    bool isCurrentlyParameterized()
    {
    	return (currentParameterization != 0);
    }
private:
	VectorGestureClassification 	classifier;
	map<int, string> 				gestureNameMap;
	mapGP						 	gestureNameToParameterizationMap;

	//Temporary vars for parameterization state handling
	gesture_parameterization*		currentParameterization;
	reordering_filter*				reorderFilter;
	bool							paused;
	bool							resetOrdering;
};

#endif /* GESTURES_H_ */
