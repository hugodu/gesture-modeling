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
		//cout << x << ", " << y << "; ";

		vals.push_back(x);
		vals.push_back(y);
	}

};



class ContactSetFrame{
public:
	vector<Contact> frame;
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
			frame.push_back(contact);

		}
		//printFrame();
	}

	void clear()
	{
		frame.clear();
	}
	void push_back(Contact c)
	{
		frame.push_back(c);
	}
	vector<double> transform()
	{
		vector<double> transformed;
		for(size_t i = 0; i < frame.size(); i++)
		{
			transformed.push_back(frame.at(i).x);
			transformed.push_back(frame.at(i).y);
		}
		if(transformed.size() != frame.size() * 2)
			cout << "Transformed Vector size isn't right: " << transformed.size() <<
			" Expected: " << frame.size() * 2 <<endl;

		return transformed;

	}

	size_t size()
	{
		return frame.size();
	}


	void printFrame()
	{
		vector<Contact>::iterator contact;
		for( contact = frame.begin(); contact != frame.end(); contact++) {
			cout << "x: " << contact->x << "\ty: " << contact->y << "\t";
		}
		cout << endl;

	}

};

class GestureSample{
public:
	vector<ContactSetFrame> sample;
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
				sample.push_back(frame);
		}
		//printSample();

	}

	void printSample()
	{
		unsigned int i = sample.size();
		cout << "Size: " << i << endl;
		i = 0;
		while(i < sample.size()) {
			(sample.at(i)).printFrame();
			i++;

		}
		cout << "---" << endl;

	}
	void clear()
	{
		sample.clear();
	}
	void push_back(ContactSetFrame f)
	{
		sample.push_back(f);
	}
	size_t size()
	{
		return sample.size();
	}


	vector<vector<double> > transform()
	{
		vector<vector<double> > transformed;
		for(size_t i = 0; i < sample.size(); i++)
		{
			vector<double> tempT  = sample.at(i).transform();
			transformed.push_back(tempT);
		}
		return transformed;

	}
};

#endif
