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

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

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
	VectorGestureClassification classifier;
	map<int, string> 			gestureNameMap;
	map<string, vector<string> > gestureNameToParametersMap;

	RecognitionHelper()
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
	 *
	 */
	vector<string> classify(GestureSample* sample)
	{
		vector<string> result;
		vector<vector<double> > transformed = sample->transform();
		//printTransform(transformed);
		int classIndex = classifier.classify(transformed);
		vector<long double> probs = classifier.probabilities();

		bool allZero = true;
		for(unsigned int i = 0 ; i < probs.size(); i++)
		{
			if(probs[i] > 0)
				allZero = false;
			cout << probs[i] << ", ";
		}
		cout << endl;

		if(!allZero && classIndex >= 0 && ((unsigned int)classIndex) < gestureNameMap.size())
		{
			//FIXME: More structure may help future cases
			multitouch_filter* filter = static_cast<multitouch_filter *>(classifier.getFilter(classIndex));
			result.push_back(gestureNameMap[classIndex]);

			result.push_back(boost::lexical_cast<std::string>(-filter->tX));
			result.push_back(boost::lexical_cast<std::string>(-filter->tY));

		}
		else //classIndex == -1 || allZero probabilities when sample doesn't match any filter-model pair
			result.push_back("None");
		return result;

	}

	const std::vector<long double> &probabilities() const
	{
	    return classifier.probabilities();
	}
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & classifier;
        ar & gestureNameMap;
    }

    void saveGestureSet(const char* appName)
    {
    	ofstream storage(appName, ios::out | ios::binary);
    	boost::archive::binary_oarchive out(storage);
    	out << *this;
    }

    void loadGestureSet(const char* appName)
    {
    	ifstream storage(appName, ios::in | ios::binary);
    	boost::archive::binary_iarchive in(storage);
    	in >> *this;
    }
    void clearGestureSet()
    {
    	gestureNameMap.clear();
    	classifier.clear();
    }
};

/**
 * Instances of this class will allow a vector to be translated into
 * a sequence of parameters
 */
class gesture_parameterization
{
public:

	gesture_parameterization(){}

	gesture_parameterization(const char* paramString)
	{
		//Temporary test for only distance
		cout << "Initing gesture Parameters: " << paramString << endl;
		//TODO: parse the parameter specification scheme.
		//fing_dist 0 1 	| will generate a parameter providing distance between two fingers
		//fing_x 0 			| will make x coord of finger 0 a parameter
		//fing_y 1			| will make y coord of finger 1 a parameter
		//fing_angle 0 1	| will calculate angle made by line from 0 to 1 with the positive x.
		//delta fing_x 1	| will keep track of last value of x, and send only the change in fing_x
		//dist 0 mean_xy 1 2| will first calculate the mean of fingers 1,2 and then calculate dist between mean and finger 0
	}

	/**
	 * Accepts the input frame, processes and converts to a sequence of
	 * requested parameters
	 */
	vector<double> operator()(ContactSetFrame & contactFrame) const
	{
		vector<double> result;
		vector<Contact> frame = contactFrame.frame;

		if(frame.size() < 2) // atleast two fingers required for this test
			return result;
		double dx = (frame[0].x - frame[1].x);
		double dy = (frame[0].y - frame[1].y);
		double dist = sqrt(dx*dx + dy*dy);
		result.push_back(dist);
		cout << "Dist Param: " << dist << endl;
		return result;
	}
};

#endif /* GESTURES_H_ */
