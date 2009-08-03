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

	RecognitionHelper recognizer;
	string gidPre = "32";
	string trainUID = "22";
	string testUID = "22";

	for (int i = 1; i <= 4; ++i)
	{
		string gestName = gidPre + boost::lexical_cast<std::string>(i);
		vector<GestureSample> train = readGestureSet(gestName, trainUID);
		cout << "Training Gesture " << i << endl;//" with " << trnsfTrain.size() << " samples";
		recognizer.trainWithSamples(train, gestName);
	}
	int totalNum = 0, correct = 0;
	for (int i = 1; i <= 4; ++i)
	{
		string gestName = gidPre + boost::lexical_cast<std::string>(i);
		vector<GestureSample> testSamples = readGestureSet(gestName, testUID);
		for (size_t sampleNum = 0; sampleNum < testSamples.size(); sampleNum++)
		{
			totalNum++;
			vector<string> classifiedAs = recognizer.classify(&testSamples[sampleNum]);
			cout << "Gesture: " << i << " Sample: " << sampleNum << "\tClassified as: " << classifiedAs[0];
			vector<long double > probs = recognizer.probabilities();
			for(size_t p = 0; p < probs.size(); p++)
				cout << "\t(" << probs[p] << ")";
			cout << endl;
			if (strcmp(classifiedAs[0].c_str(), gestName.c_str()) == 0)
				correct++;
		}
	}
	cout << "Overall Accuracy: " << float(correct) / totalNum << endl << endl;
	cout << "\n-------\nSaving Gestures" << endl;
	recognizer.saveGestureSet("tempSet.gestr");

	RecognitionHelper loaded;
	cout << "\n-------\nLoading Gestures" << endl;
	loaded.loadGestureSet("tempSet.gestr");

	totalNum = 0;
	correct = 0;
	for (int i = 1; i <= 4; ++i)
	{
		string gestName = gidPre + boost::lexical_cast<std::string>(i);
		vector<GestureSample> testSamples = readGestureSet(gestName, testUID);
		for (size_t sampleNum = 0; sampleNum < testSamples.size(); sampleNum++)
		{
			totalNum++;
			vector<string> classifiedAs = loaded.classify(&testSamples[sampleNum]);
			cout << "Gesture: " << i << " Sample: " << sampleNum << "\tClassified as: " << classifiedAs[0];
			vector<long double > probs = loaded.probabilities();
			for(size_t p = 0; p < probs.size(); p++)
				cout << "\t(" << probs[p] << ")";
			cout << endl;
			if (strcmp(classifiedAs[0].c_str(), gestName.c_str()) == 0)
				correct++;
		}
	}

	cout << "Overall Accuracy: " << float(correct) / totalNum << endl << endl;

}
