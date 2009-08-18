/**
 * Touch utils
 *
 * Author: sashikanth
 */

#ifndef _TOUCH_H
#define _TOUCH_H

#include <vector>
#include <cmath>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
using namespace std;

class Contact {
public:
	const static int num_dims = 7;
	int id;
	float x, y, dx, dy, width, height, pressure;

	Contact(){};
	Contact(int _id, vector<string> vals)
	{
		id = _id;

		x  = atof(vals[id * num_dims].c_str());
		y  = atof(vals[id * num_dims + 1].c_str());
		dx = atof(vals[id * num_dims + 2].c_str());
		dy = atof(vals[id * num_dims + 3].c_str());

		width    = atof(vals[id * num_dims + 4].c_str());
		height   = atof(vals[id * num_dims + 5].c_str());
		pressure = atof(vals[id * num_dims + 6].c_str());

	}

	Contact(int _id, float _x, float _y,
			float _dx, float _dy,
			float _width, float _height,
			float _pressure)
	: id(_id), x(_x), y(_y), dx(_dx), width(_width), height(_height),pressure(_pressure)
	{

	}
};


const static int num_dims = 7;

class ContactSetFrame{
public:
	vector<Contact> frame;
	ContactSetFrame(){};
	ContactSetFrame(string &frameStr)
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
		boost::tokenizer<> tok(frameStr);
		for(boost::tokenizer<>::iterator token=tok.begin(); token != tok.end(); ++token)
			vals.push_back(*token);

		for (size_t j = 0; j < vals.size(); j+=num_dims)
		{
			contact = Contact(j/num_dims, vals);
			frame.push_back(contact);
		}
		//printFrame();
	}
	void clear()
	{
		frame.clear();
	}
	void push_back(Contact &c)
	{
		frame.push_back(c);
	}

	Contact getContact(int contactIndex)
	{
		return frame[contactIndex];
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

	bool isWithinTolerance(ContactSetFrame& otherFrame, double tolerance)
	{
		bool isStatic = true;
		if(frame.size() != otherFrame.size())
			return false;

		for(size_t fingerNum = 0; fingerNum < frame.size(); fingerNum++)
		{
			Contact c 	= frame[fingerNum];
			Contact lc 	= otherFrame.getContact(fingerNum);
			if(abs(c.x - lc.x) > tolerance || abs(c.y - lc.y) > tolerance)
				isStatic = false;
		}
		return isStatic;
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
	const static double move_tolerance = .003; //Will vary from screen to screen.
	vector<ContactSetFrame> sample;
	GestureSample(){};
	GestureSample(string sampleStr)
	{
		vector<string> frames;
		ContactSetFrame frame;


		boost::char_separator<char> sep("];");
		boost::tokenizer<boost::char_separator<char> > tokens(sampleStr, sep);

		BOOST_FOREACH(string frameStr, tokens)
		{
			frame = ContactSetFrame(frameStr);
			if(frame.size() > 0)
				sample.push_back(frame);
		}
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
	void push_back(ContactSetFrame &f)
	{
		sample.push_back(f);
	}

	/**
	 * Number of fingers in the last frame
	 */
	size_t lastFrameSize()
	{
		if(sample.size() <= 0)
			return 0;
		return sample[sample.size() - 1].size();
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

	bool isStatic(int numFrames)
	{
		return isStatic(move_tolerance, numFrames);
	}

	bool isStatic(double tolerance, unsigned int numFrames)
	{
		//iterate through frames, check if all points are within tolerance.
		if(sample.size() < numFrames)
			return false; // samples of frame size < 10 are not considered static
		ContactSetFrame lastFrame = sample[sample.size() - 1]; // Check with the latest frame

		bool isStatic = true;
		for(size_t frameNum = sample.size() - 2; frameNum >= sample.size() - numFrames && isStatic == true; frameNum--)
		{
			//For each finger, the x,y vals should be within tolerance of lastFrame
			isStatic = sample[frameNum].isWithinTolerance(lastFrame, tolerance);
		}
		if(isStatic)
		{
			//Clear the last numFrames of the sample, since they haven't moved
			sample.erase(sample.end() - (numFrames + 1), sample.end());
		}
		return isStatic;
	}

	bool isOnlyStatic()
	{
		if(sample.size() == 0)
			return false;
		bool result = isStatic(sample.size());
		if(result)
			cout << "Sample is only Static" << endl;
		return result;
	}
};

#endif
