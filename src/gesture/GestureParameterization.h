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
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>

//FIXME Check input strings before proceeding. Provide helpful error messages.
class gesture_parameter
{
public:
	vector<int> fingerIndeces;

	/**
	 * Dummy operator
	 */
	virtual vector<double> operator()(ContactSetFrame & contactFrame)
	{
		return vector<double>();
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & fingerIndeces;
    }
};

class fing_x_parameter : public gesture_parameter
{
public:
	fing_x_parameter(int fing_index)
	{
		fingerIndeces.push_back(fing_index);
	}
	virtual vector<double> operator()(ContactSetFrame & contactFrame)
	{
		vector<double> result;
		const vector<Contact> &frame  = contactFrame.frame;
		unsigned int fing_index = fingerIndeces[0];
		if(frame.size() <= fing_index)
		{
			cout << "Invalid frame for fing_x with index" << fing_index << endl;
			return result;
		}
		result.push_back(frame[fing_index].x);
		return result;
	}
};

class fing_y_parameter : public gesture_parameter
{
public:
	fing_y_parameter(int fing_index)
	{
		fingerIndeces.push_back(fing_index);
	}
	virtual vector<double> operator()(ContactSetFrame & contactFrame)
	{
		vector<double> result;
		const vector<Contact> &frame  = contactFrame.frame;
		unsigned int fing_index = fingerIndeces[0];
		if(frame.size() < fing_index)
		{
			cout << "Invalid frame for fing_y with index" << fing_index << endl;
			return result;
		}
		result.push_back(frame[fing_index].y);
		return result;
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

	virtual vector<double> operator()(ContactSetFrame & contactFrame)
	{
		vector<double> result;
		const vector<Contact> &frame = contactFrame.frame;

		if(frame.size() < 2) // atleast two fingers required for this test
			return result;
		double dx = (frame[0].x - frame[1].x);
		double dy = (frame[0].y - frame[1].y);
		double dist = sqrt(dx*dx + dy*dy);
		result.push_back(dist);
		return result;
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

	virtual vector<double> operator()(ContactSetFrame & contactFrame)
	{
		vector<double> result;
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
		return result;
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
	typedef pair<string, string> namedPairT;
	typedef pair<string, vector<double> > namedParamValPairT;
	typedef map<string, vector<double> > namedParamValMapT;
	typedef vector<namedParamValPairT> namedParamVectorT;
	typedef pair<string, gesture_parameter> paramPairT;
	typedef boost::ptr_map<string, gesture_parameter> namedParamMapT;

	namedParamMapT 	namedParamsMap;
	namedParamValMapT 	prevVals;
	bool 			delta;

	gesture_parameterization()
	:delta(false)
	{}

	gesture_parameterization(namedPairT namedPair)
	: delta(false)
	{
		addParameter(namedPair);
	}


	void addParameter(namedPairT namedPair)
	{
		string paramName = namedPair.first;
		string paramString = namedPair.second;

		//Note: Boost tokenizer uses _ as a token seperator by default ! workaround this...
		boost::char_separator<char> sep(" ");
		boost::tokenizer<boost::char_separator<char> > 				tok(paramString, sep);
		boost::tokenizer<boost::char_separator<char> >::iterator 	token=tok.begin();


		//Instantiate the appropriate parameter class and add to the params vector.

		if(*token == "delta") //Caches previous value and sends difference only
		{
			delta = true;
			token++;
		}

		if(*token == "fing_dist")
		{
			int fing_index1 = boost::lexical_cast<int>(*++token);
			int fing_index2 = boost::lexical_cast<int>(*++token);
			namedParamsMap.insert(paramName , new fing_dist_parameter(fing_index1,fing_index2));
		}
		else if(*token == "fing_x")
		{
			int fing_index = boost::lexical_cast<int>(*++token);
			namedParamsMap.insert(paramName ,  new fing_x_parameter(fing_index));
		}
		else if(*token == "fing_y")
		{
			int fing_index = boost::lexical_cast<int>(*++token);
			namedParamsMap.insert(paramName ,  new fing_y_parameter(fing_index));
		}
		else if(*token == "all_mean")
		{
			namedParamsMap.insert(paramName , new all_mean_parameter());
		}
		cout << "Instantiated gesture parameter: " << paramString << ". Gesture now has " << namedParamsMap.size() <<  " params" << endl;

	}
	/**
	 * Parameterization call
	 */
	namedParamValMapT operator()(ContactSetFrame & contactFrame)
	{
		namedParamValMapT resultMap;

		for( namedParamMapT::iterator param = namedParamsMap.begin(), e = namedParamsMap.end(); param != e; ++param )
		{
			string 				paramName 		= param->first;
			gesture_parameter*	parameter 		= param->second;
			vector<double> 		currParamValues = parameter->operator()(contactFrame);
			namedParamValPairT 	paramVals(paramName, currParamValues );
			if(delta)
			{
				if(prevVals.empty())
					prevVals.insert(paramVals);
				else
				{
					vector<double> deltaVals;
					vector<double> prevParamValues = prevVals[paramName];
					for(size_t i = 0; i < prevParamValues.size(); i++) //prevParamValues and currParamValues should have same size.
						deltaVals.push_back(currParamValues[i] - prevParamValues[i]);

					paramVals.second = deltaVals; //Set result to the difference in vals;
					prevVals[paramName] = currParamValues;
				}

			}

			resultMap.insert(paramVals);
		}
		return resultMap;
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & namedParamsMap;
    }
};
#endif /* GESTUREPARAMETERIZATION_H_ */
