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
#include <boost/lexical_cast.hpp>
using namespace std;

class Contact {
public:
	const static int num_dims = 7;
	int id;
	float x, y, dx, dy, width, height, pressure;

	Contact(){};
	Contact(int _id, const vector<string> &vals)
	{
		id = _id;

		x = boost::lexical_cast<float>(vals[id * num_dims]);
		y  = boost::lexical_cast<float>(vals[id * num_dims + 1]);
		dx = boost::lexical_cast<float>(vals[id * num_dims + 2]);
		dy = boost::lexical_cast<float>(vals[id * num_dims + 3]);

		width    = boost::lexical_cast<float>(vals[id * num_dims + 4]);
		height   = boost::lexical_cast<float>(vals[id * num_dims + 5]);
		pressure = boost::lexical_cast<float>(vals[id * num_dims + 6]);

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
		//each is a frame
		//Num samples = number of tokens / NUM_DIMS;
		vector<string> vals;
		boost::char_separator<char> sep("[ ");
		boost::tokenizer<boost::char_separator<char> > tok(frameStr, sep);

		std::copy(tok.begin(),tok.end(),std::back_inserter (vals));

		for (size_t j = 0; j < vals.size(); j+=num_dims)
			frame.push_back(Contact(j/num_dims, vals));

		//printFrame();
	}
	void clear()
	{
		frame.clear();
	}
	void push_back(const Contact &c)
	{
		frame.push_back(c);
	}

	const Contact &getContact(int contactIndex) const
	{
		return frame[contactIndex];
	}

	vector<double> transform() const
	{
		vector<double> transformed;
		BOOST_FOREACH(Contact c, frame)
		{
			transformed.push_back(c.x);
			transformed.push_back(c.y);
		}
		if(transformed.size() != frame.size() * 2)
			cout << "Transformed Vector size isn't right: " << transformed.size() <<
			" Expected: " << frame.size() * 2 <<endl;

		return transformed;
	}

	bool isWithinTolerance(const ContactSetFrame& otherFrame, double tolerance) const
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

	size_t size() const
	{
		return frame.size();
	}
	void printFrame() const
	{
		BOOST_FOREACH(Contact contact, frame)
			cout << "(" << contact.id << ") x: " << contact.x << "\ty: " << contact.y << "\t";
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

	int numFingers() const
	{
		return sample[0].frame.size();
	}
	void printSample() const
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
	size_t lastFrameSize() const
	{
		if(sample.empty())
			return 0;
		return sample.back().size();
	}

	size_t size() const
	{
		return sample.size();
	}

	vector<vector<double> > transform() const
	{
		vector<vector<double> > transformed;
		for(size_t i = 0; i < sample.size(); i++)
		{
			vector<double> tempT  = sample[i].transform();
			transformed.push_back(tempT);
		}
		return transformed;

	}

	bool checkIfStaticAndTrimFrames(int numFrames)
	{
		return checkIfStaticAndTrim(move_tolerance, numFrames);
	}

	bool checkIfStaticAndTrim(double tolerance, unsigned int numFrames)
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

	bool checkIfOnlyStaticAndTrim()
	{
		if(sample.empty())
			return false;
		bool result = checkIfStaticAndTrimFrames(sample.size());
		if(result)
			cout << "Sample is only Static" << endl;
		return result;
	}
};

#endif
