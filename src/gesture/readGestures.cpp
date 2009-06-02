#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <cstdlib>

#include "touch/Touch.h"
#include "gesture/models/VectorGestureClassification.h"
#include "boost/lexical_cast.hpp"

using namespace std;

void printTransform(vector<vector<double> > transformed)
{
    cout << "Transformed Size: " << transformed.size() << endl;

    for(int i = 0; i < transformed.size(); ++i)
    {
	for (int j = 0; j < transformed.at(i).size(); ++j)
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
  string base = "data/reOrdered/";
  string fileName = "gid_" + gid + "_uid_" + uid + ".seqs";
  return readFile(base + fileName);
}


vector<vector<vector<double> > > transformSamples(vector<GestureSample> samples)
{
	vector<vector<vector<double> > > transformedSet;
        cout << "Transforming " << samples.size() << " samples" << endl;

	if (samples.size() > 0)
	{
	    for (int i = 0; i < samples.size(); ++i)
	        transformedSet.push_back(samples.at(i).transform());
	    // if(transformed.size() > 0)
	    // 	printTransform(transformed);
	    // else
	    // 	cout << "Nothing's transformed" <<endl;

	    //Transformed gesture set obtained.
	    //Pass to classifier as example set
	}

	else
	    cout << "No Samples" << endl;

	return transformedSet;

}

int readFiles(int argc, char *argv[])
{


    char* fileName;
    int fileNum = 1;
    cout << "Args: " << argc << endl << endl;


    if (argc < 2)
    {
	cout << "No Input files. Using ../../data/reOrdered/gid_201_uid_1.seqs" << endl;
	//return 0;
	fileName = "../../data/reOrdered/gid_201_uid_1.seqs";

    }
    else
    {
	fileName = argv[fileNum];
	cout << "Reading : " << fileName;
    }
    vector<GestureSample> readSamples;
    //cout << "0: " << argv[0] << " 1: " << argv[1] << " args" << endl;
    if (argc == 2)
    {
	fileNum++;
	readSamples = readFile(fileName);
	vector<vector<vector<double> > > transformedSet = transformSamples(readSamples);
	cout << "TransformedSet Size: " << transformedSet.size() << endl;
    }
    return 0;
}



int main(int argc, char *argv[])
{
//    return readFiles(argc, argv);
  VectorGestureClassification classifier;
  string gidPre = "20";
  for (int i = 1; i <= 5; ++i)
  {
    vector<GestureSample> train = readGestureSet(gidPre + boost::lexical_cast<std::string>(i), "3");
    vector<vector<vector<double> > > trnsfTrain = transformSamples(train);
    cout << "Training Gesture " << i << endl;//" with " << trnsfTrain.size() << " samples";
    for(int k = 0; k < trnsfTrain.size(); k++)
      printTransform(trnsfTrain[k]);
    //cout << "Now" << endl;
    if(!trnsfTrain.empty())
      classifier.addGestureWithExamples(trnsfTrain, 10);
  }

  for (int i = 1; i <= 5; ++i)
  {
    vector<GestureSample> test = readGestureSet(gidPre + static_cast<char>(i), "2");
    vector<vector<vector<double> > > transformTest = transformSamples(test);
    for(int sampleNum = 0; sampleNum < transformTest.size(); sampleNum++)
    {
      vector<vector<double> > sample = transformTest.at(sampleNum);
      int classifiedAs = classifier.classify(sample);
      cout << "Gesture: " << i << "Sample: " << sampleNum << "\tClassified as: " << classifiedAs;
    }
  }
}
