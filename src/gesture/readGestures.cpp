#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <cstdlib>


#include "touch/Touch.hpp"


using namespace std;

void Tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

int main(int argc, char *argv[])
{
  ifstream readFile;
  
  cout << "Reading: " << argv[1] << " " << endl;
  if (argc == 1)
  {
      cout << "No Input files" << endl;
      return 0;
  }
  int fileNum = 0;
  while (fileNum++ < argc)
  {
      readFile.open(argv[1]);
      string line;
      
      vector<GestureSample> readSamples;
  
      GestureSample sample;
      ContactSetFrame frame;
      Contact contact;

      if (readFile.is_open())
      {
	  while(!readFile.eof())
	  {
	      vector<string> frames;
	      sample.clear();
	      getline(readFile,line);
	      
	      
	      Tokenize(line, frames, "];");
	      for (int i = 0; i < frames.size(); ++i)
	      {
		  string frameStr = frames[i];
		  int pos = frameStr.find("[");
		  while(pos >=0)
		  {
		      frameStr.replace(pos, 1, "");
		      pos = frameStr.find("[", pos);
		  }
	          //each is a frame
		  //Num samples = number of tokens / 7;
		  vector<string> vals;
		  frame.clear();
		  
		  Tokenize(frameStr, vals);
		  for (int j = 0; j < vals.size(); j+=7)
		  {
		      
		      //each contact
		      frame.push_back(
			  Contact(j/7, atof(vals[j].c_str()), atof(vals[j+1].c_str()), //id, x, y
				  atof(vals[j+2].c_str()), atof(vals[j+3].c_str()),     //dx,dy
				  atof(vals[j+4].c_str()), atof(vals[j+5].c_str()),     //w,h
				  atof(vals[j+6].c_str())));                   //pressure
		      
		  }
		  sample.push_back(frame);
	      }
	      readSamples.push_back(sample);
	      
	      cout << "Num Frames: " << sample.size() << endl;
	      
              //ostream_iterator<string> out_it (cout, ", ");
	      //copy(frames.begin(), frames.end(), out_it);
	  }
	  cout << "Total Samples: " << readSamples.size() <<endl;
	  
	  readFile.close();
      }    
  }
  
  return 0;
}


