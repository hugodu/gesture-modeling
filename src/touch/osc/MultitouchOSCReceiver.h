/*
 * MultitouchOscReceiver.h
 *
 *      Author: sashikanth
 */

#ifndef MULTITOUCHOSCRECEIVER_H_
#define MULTITOUCHOSCRECEIVER_H_

#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"

using namespace std;
ContactSetFrame frame;
class MultitouchOscReceiver: public osc::OscPacketListener
{
public:

	ContactSetFrame currFrame;
	unsigned int numContacts;
	GestureCollector *listener;

	MultitouchOscReceiver()
	{

	}

	~MultitouchOscReceiver()
	{

	}

	void ProcessMessage(const osc::ReceivedMessage& m,
			const IpEndpointName& remoteEndpoint)
	{
		try
		{
			osc::ReceivedMessageArgumentIterator arg = m.ArgumentsBegin();

			if (strcmp(m.AddressPattern(), "/gestr/sample") == 0)
			{
				//New Sample starts, Send event to listeners.
				//cout << "sample" << endl;
				const char* smpl = (arg++)->AsString();
				if (strcmp(smpl, "start") == 0)
				{
					const char* gestureName = (arg++)->AsString();
					listener->startSample(gestureName);
				}
				else if (strcmp(smpl, "end") == 0)
					listener->endSample();
				else
					cout << "MsgParseError";
			}

			if (strcmp(m.AddressPattern(), "/gestr/action") == 0)
			{
				listener->gestureAction((arg++)->AsString());
			}

			else if (strcmp(m.AddressPattern(), "/tuio2d/frm") == 0)
			{
				//New frame starts now
				currFrame.clear();
			}
			else if (strcmp(m.AddressPattern(), "/tuio2d/cur") == 0)
			{
				Contact contact;

				contact.id = (arg++)->AsInt32();
				contact.x = (arg++)->AsFloat();
				contact.y = (arg++)->AsFloat();
				contact.dx = (arg++)->AsFloat();
				contact.dy = (arg++)->AsFloat();
				contact.width = (arg++)->AsFloat();
				contact.height = (arg++)->AsFloat();
				//contact.pressure = (arg++)->AsFloat();

				if (arg != m.ArgumentsEnd())
					throw osc::ExcessArgumentException();

				currFrame.push_back(contact);
			}
			else if (strcmp(m.AddressPattern(), "/tuio2d/sid") == 0)
			{
				//Sent at end of frame.
				numContacts = m.ArgumentCount();
                //Send Frame to listener
                listener->updateFrame(currFrame);
			}
		} catch (osc::Exception& e)
		{
			cout << "error while parsing message: " << m.AddressPattern()
					<< ": " << e.what() << endl;
		}
	}

};

void initMultitouchOscReceiver(int port, GestureCollector *collector)
{
	MultitouchOscReceiver oscListener;

	UdpListeningReceiveSocket s(IpEndpointName(IpEndpointName::ANY_ADDRESS,
			port), &oscListener);

	cout << "Now Listening for input on port " << port << "..." <<endl;
//	cout << "Collector Type: " <<
	oscListener.listener = collector;
	s.RunUntilSigInt();

	cout << "Done listening to OSC.\n";

}

#endif /* MULTITOUCHOSCRECEIVER_H_ */
