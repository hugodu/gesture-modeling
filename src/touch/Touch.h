/**
 * Touch utils
 *
 * Author: sashikanth
 */

#ifndef _TOUCH_H
#define _TOUCH_H

#include <vector>

using namespace std;

#define NUM_DIMS 7


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



class Contact {
public:
  int id;
  float x, y, dx, dy, width, height, pressure;

  Contact(){};
  Contact(int _id, vector<string> vals)
  {
    id = _id;

    x  = atof(vals[id * NUM_DIMS].c_str());
    y  = atof(vals[id * NUM_DIMS + 1].c_str());
    dx = atof(vals[id * NUM_DIMS + 2].c_str());
    dy = atof(vals[id * NUM_DIMS + 3].c_str());

    width    = atof(vals[id * NUM_DIMS + 4].c_str());
    height   = atof(vals[id * NUM_DIMS + 5].c_str());
    pressure = atof(vals[id * NUM_DIMS + 6].c_str());

  }

  ~Contact(){};
  Contact(int _id, float _x, float _y,
      float _dx, float _dy,
      float _width, float _height,
      float _pressure)
  {
    id = _id;
    x = _x;
    y = _y;
    dx = _dx;
    dy = _dy;
    width = _width;
    height = _height;
    pressure = _pressure;
  }

  void transform(vector<float> vals)
  {
    cout << x << ", " << y << "; ";

    vals.push_back(x);
    vals.push_back(y);
  }

};



class ContactSetFrame : public vector<Contact>{
public:
  ContactSetFrame(){};
  ContactSetFrame(string frameStr)
  {
    int pos = frameStr.find("[");
    while(pos >=0)
      {
        frameStr.replace(pos, 1, "");
        pos = frameStr.find("[", pos);
      }
    //each is a frame
    //Num samples = number of tokens / NUM_DIMS;
    vector<string> vals;
    Contact contact;
    Tokenize(frameStr, vals);

    for (size_t j = 0; j < vals.size(); j+=NUM_DIMS)
      {
        contact = Contact(j/NUM_DIMS, vals);
        push_back(contact);

      }
    //printFrame();
  }

  ~ContactSetFrame(){};

  vector<double> transform()
  {
    vector<double> transformed;
    for(size_t i = 0; i < size(); i++)
      {
        transformed.push_back(at(i).x);
        transformed.push_back(at(i).y);
      }
    if(transformed.size() != size() * 2)
      cout << "Transformed Vector size isn't right: " << transformed.size() <<
      " Expected: " << size() * 2 <<endl;

    return transformed;

  }


  void printFrame()
  {
    vector<Contact>::iterator contact;
    for( contact = begin(); contact != end(); contact++) {
      cout << "x: " << contact->x << "y: " << contact->y;
    }
    cout << endl;

  }

};

class GestureSample : public vector<ContactSetFrame> {
public:
  GestureSample(){};
  GestureSample(string sampleStr)
  {
    vector<string> frames;
    ContactSetFrame frame;

    Tokenize(sampleStr, frames, "];");
    //cout << sampleStr << endl << endl << endl;
    //cout << "---------------------------------" << endl << "Frames: " << frames.size() << endl;

    for (size_t i = 0; i < frames.size(); i++)
      {
        string frameStr = frames[i];
        frame = ContactSetFrame(frameStr);

        if(frame.size() > 0)
          push_back(frame);
      }
    //printSample();

  }

  ~GestureSample(){};
  void printSample()
  {
    unsigned int i = size();
    cout << "Size: " << i << endl;
    i = 0;
    while(i < size()) {
      (at(i)).printFrame();
      i++;

    }
    cout << "---" << endl;

  }

  vector<vector<double> > transform()
  {
    vector<vector<double> > transformed;
    for(size_t i = 0; i < size(); i++)
      {
        vector<double> tempT  = at(i).transform();
        transformed.push_back(tempT);
      }
    return transformed;

  }
};

#endif
