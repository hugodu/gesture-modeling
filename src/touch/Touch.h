/**
 * Touch utils
 *
 * Author: sashikanth
 */

#ifndef _TOUCH_H
#define _TOUCH_H

using namespace std;

class Contact {
public:
	int id;
	float x, y, dx, dy, width, height, pressure;

	Contact(){};
	~Contact(){};
	Contact(int _id, float _x, float _y, float _dx, float _dy, float _width, float _height, float _pressure){
	  id = _id;
	  x = _x;
	  y = _y;
	  dx = _dx;
	  dy = _dy;
	  width = _width;
	  height = _height;
	  pressure = _pressure;
	}
};

#include <vector>

class ContactSetFrame : public vector<Contact>{
public:
    ContactSetFrame(){};
    ~ContactSetFrame(){};
    void printFrame()
    {
	vector<Contact>::iterator contact;
	for( contact = begin(); contact != end(); contact++) {
	    cout << "x: " << contact->x << "y: " << contact->y;
	}
    }

};

class GestureSample : public vector<ContactSetFrame> {
public:
    GestureSample(){};
    ~GestureSample(){};
    void printSample()
    {
	unsigned int i = size();
	cout << "Size: " << i << endl;
	i = 0;
	while(i++ < size()) {
	    (at(i)).printFrame();
	}
	cout << "---" << endl;
    }

};

#endif
