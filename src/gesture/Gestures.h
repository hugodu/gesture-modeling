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

#include <touch/Touch.h>
#include <gesture/models/VectorGestureClassification.h>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>

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

	RecognitionHelper()
	{}
	void trainWithSamples(vector<GestureSample> trainingSet, string gestureName)
	{
		vector<vector<vector<double> > > trnsfTrain = transformSamples(trainingSet);
		//Magical filter takes care of all preprocessing, and live processing of the incoming sample(s).
		multitouch_filter filter = multitouch_filter(trnsfTrain);

		//Ensure the training set is appropriately scaled for training the model.
		vector<vector<vector<double> > > filteredTraining = trnsfTrain;
		for(size_t i = 0; i < trnsfTrain.size(); i++)
		{
			filter.reset_params_for(trnsfTrain[i]);
			boost::copy(trnsfTrain[i] | boost::adaptors::transformed(filter),filteredTraining[i].begin());
		}

		cout << "Training With: " << filteredTraining.size() << " samples" << endl;
		classifier.addGestureWithExamplesAndFilter(filteredTraining, 5, filter);
		gestureNameMap.insert(pair<int, string>(classifier.numGestures() - 1, gestureName));
		cout << "\nAdded new gesture(" << classifier.numGestures() - 1 << "): " << gestureName << "\n"<<endl;
	}

	string classify(GestureSample sample)
	{
		vector<vector<double> > transformed = sample.transform();
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
			return gestureNameMap[classIndex];
		else //classIndex == -1 || allZero probabilities when sample doesn't match any filter-model pair
			return "None";
	}

	const std::vector<long double> &probabilities() const
	{
	    return classifier.probabilities();
	}

};

#endif /* GESTURES_H_ */
