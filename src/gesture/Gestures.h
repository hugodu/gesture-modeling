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
		//Use the first sample as the representative
		scale_filter filter = scale_filter(trnsfTrain[0][0]);
		//Ensure the training set is appropriately scaled
		vector<vector<vector<double> > > trnsfScaled = trnsfTrain;
		for(size_t i = 0; i < trnsfTrain.size(); i++)
		{
			boost::copy(trnsfTrain[i] | boost::adaptors::transformed(filter),trnsfScaled[i]);
		}

		classifier.addGestureWithExamples(trnsfTrain, 10);
		gestureNameMap.insert(pair<int, string>(classifier.numGestures() - 1, gestureName));
		cout << "Added: " << gestureName << " as: " << classifier.numGestures() - 1;
	}

	string classify(GestureSample sample)
	{
		int classIndex = classifier.classify(sample.transform());
		return gestureNameMap[classIndex];
	}
};

#endif /* GESTURES_H_ */
