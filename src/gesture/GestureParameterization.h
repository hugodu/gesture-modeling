/*
 * GestureParameterization.h
 *
 *  Created on: Aug 18, 2009
 *      Author: damaraju
 */

#ifndef GESTUREPARAMETERIZATION_H_
#define GESTUREPARAMETERIZATION_H_

#include <touch/Touch.h>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class gesture_parameter
{
public:
	vector<int> fingerIndeces;
	/**
	 * Dummy operator
	 */
	virtual void operator()(ContactSetFrame & contactFrame, vector<double> & result)
	{
		return;
	}

};

class fing_x_parameter : public gesture_parameter
{
public:
	fing_x_parameter(int fing_index)
	{
		fingerIndeces.push_back(fing_index);
	}
	virtual void operator()(ContactSetFrame & contactFrame, vector<double> & result)
	{
		const vector<Contact> &frame  = contactFrame.frame;
		unsigned int fing_index = fingerIndeces[0];
		if(frame.size() <= fing_index)
		{
			cout << "Invalid frame for fing_x with index" << fing_index << endl;
			return;
		}
		result.push_back(frame[fing_index].x);
	}
};

class fing_y_parameter : public gesture_parameter
{
public:
	fing_y_parameter(int fing_index)
	{
		fingerIndeces.push_back(fing_index);
	}
	virtual void operator()(ContactSetFrame & contactFrame, vector<double> & result)
	{
		const vector<Contact> &frame  = contactFrame.frame;
		unsigned int fing_index = fingerIndeces[0];
		if(frame.size() < fing_index)
		{
			cout << "Invalid frame for fing_y with index" << fing_index << endl;
			return;
		}
		result.push_back(frame[fing_index].y);
	}
};
/**
 * the () operator accepts a contactSetFrame and produces a distance measure.
 */
class fing_dist_parameter : public gesture_parameter
{
public:
	fing_dist_parameter(int finger_index1, int finger_index2)
	{
		fingerIndeces.push_back(finger_index1);
		fingerIndeces.push_back(finger_index2);
	}

	virtual void operator()(ContactSetFrame & contactFrame, vector<double> & result)
	{
		const vector<Contact> &frame = contactFrame.frame;

		if(frame.size() < 2) // atleast two fingers required for this test
			return;
		double dx = (frame[0].x - frame[1].x);
		double dy = (frame[0].y - frame[1].y);
		double dist = sqrt(dx*dx + dy*dy);
		result.push_back(dist);
		cout << "Dist Param: " << dist << endl;
	}
};

/**
 * The x,y coordinates of the mean of all fingers on screen produced as two parameters
 */
class all_mean_parameter : public gesture_parameter
{
public:
	all_mean_parameter()
	{

	}

	virtual void operator()(ContactSetFrame & contactFrame, vector<double> & result)
	{
		double meanX = 0;
		double meanY = 0;
		BOOST_FOREACH(Contact c, contactFrame.frame)
		{
			meanX += c.x;
			meanY += c.y;
		}
		meanX /= contactFrame.size();
		meanY /= contactFrame.size();
		result.push_back(meanX);
		result.push_back(meanY);
	}
};

/**
 * Instances of this class will allow a vector to be translated into
 * a parameter
 * fing_dist 0 1 	| will generate a parameter providing distance between two fingers
 * fing_x 0 		| will make x coord of finger 0 a parameter
 * fing_y 1			| will make y coord of finger 1 a parameter
 * fing_angle 0 1	| will calculate angle made by line from 0 to 1 with the positive x.
 * all_mean			| produces 2 parameters, the x,y coords of the mean of all points on screen.
 */
class gesture_parameterization
{
public:
	boost::ptr_vector<gesture_parameter> params;

	gesture_parameterization(){}

	gesture_parameterization(vector<string> paramStrings)
	{
		//Each string is a parameter.
		BOOST_FOREACH(string paramString, paramStrings)
		{
			cout << "Instantiating gesture parameter: " << paramString << endl;

			//Instantiate the appropriate parameter class and add to the params vector.
			boost::tokenizer<> tok(paramString);
			boost::tokenizer<>::iterator token=tok.begin();
			if(*token == "fing_dist")
			{
				int fing_index1 = boost::lexical_cast<int>(*++token);
				int fing_index2 = boost::lexical_cast<int>(*++token);
				params.push_back(new fing_dist_parameter(fing_index1,fing_index2));
			}
			else if(*token == "fing_x")
			{
				int fing_index = boost::lexical_cast<int>(*++token);
				params.push_back(new fing_x_parameter(fing_index));
			}
			else if(*token == "fing_y")
			{
				int fing_index = boost::lexical_cast<int>(*++token);
				params.push_back(new fing_y_parameter(fing_index));
			}
			else if(*token == "all_mean")
			{
				params.push_back(new all_mean_parameter());
			}
		}
	}

	/**
	 * Parameterization call
	 */
	vector<double> operator()(ContactSetFrame & contactFrame)
	{
		vector<double> result;
		BOOST_FOREACH(gesture_parameter &param, params)
		{
			param(contactFrame, result);
		}
		return result;
	}

	//TODO Serialize this class
};
#endif /* GESTUREPARAMETERIZATION_H_ */
