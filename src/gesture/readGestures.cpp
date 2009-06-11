#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <cstdlib>

#include "touch/Touch.h"
#include "gesture/models/VectorGestureClassification.h"
#include "boost/lexical_cast.hpp"
#include "gesture/Gestures.h"


using namespace std;

int main(int argc, char *argv[])
{

	VectorGestureClassification classifier;
	string gidPre = "50";
	for (int i = 1; i <= 5; ++i)
	{
		vector<GestureSample> train = readGestureSet(gidPre
				+ boost::lexical_cast<std::string>(i), "5");
		vector<vector<vector<double> > > trnsfTrain = transformSamples(train);

		cout << "Training Gesture " << i << endl;//" with " << trnsfTrain.size() << " samples";
		//		for(int k = 0; k < trnsfTrain.size(); k++)
		//			printTransform(trnsfTrain[k]);
		//cout << "Now" << endl;
		if (!trnsfTrain.empty())
			classifier.addGestureWithExamples(trnsfTrain, 10);
	}
	int totalNum = 0, correct = 0;
	for (int i = 1; i <= 5; ++i)
	{
		vector<GestureSample> test = readGestureSet(gidPre
				+ boost::lexical_cast<std::string>(i), "3");
		vector<vector<vector<double> > > transformTest = transformSamples(test);
		for (size_t sampleNum = 0; sampleNum < transformTest.size(); sampleNum++)
		{
			totalNum++;
			vector<vector<double> > sample = transformTest.at(sampleNum);
			int classifiedAs = classifier.classify(sample) + 1;
			cout << "Gesture: " << i << " Sample: " << sampleNum
					<< "\tClassified as: " << classifiedAs << endl;
			if (classifiedAs == i)
				correct++;
		}
	}
	cout << "Overall Accuracy: " << float(correct) / totalNum << endl;

}
