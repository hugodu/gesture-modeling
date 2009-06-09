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

ContactSetFrame frame;
class MultitouchOscReceiver : public osc::OscPacketListener {
public:

	ContactSetFrame currFrame;
	unsigned int numContacts;
	GestureCollector gestureListener;

	MultitouchOscReceiver() {

	}

	~MultitouchOscReceiver() {
	}

	void ProcessMessage( const osc::ReceivedMessage& m,
			const IpEndpointName& remoteEndpoint)
	{
		try{
			osc::ReceivedMessageArgumentIterator arg = m.ArgumentsBegin();

			if(strcmp(m.AddressPattern(), "/gestr/sample") == 0){
				//New Sample starts, Send event to listeners.
				std::cout << "sample\n";

				const char* smpl = (arg++)->AsString();
				if(strcmp(smpl, "start"))
				{
					gestureListener.startSample();
					std::cout << "sample\n";
				}
				else if(strcmp(smpl, "end"))
				{
					gestureListener.endSample();
					std::cout << "sample\n";
				}
				else
					std::cout << "MsgParseError";

			}

			else if(strcmp(m.AddressPattern(), "/tuio2d/frm") == 0){
				//New frame starts now
				currFrame.clear();
			}
			else if(strcmp(m.AddressPattern(), "/tuio2d/cur") == 0){
				std::cout << " *\t";
				Contact contact;

				contact.id		= (arg++)->AsInt32();
				contact.x 	= (arg++)->AsFloat();
				contact.y 	= (arg++)->AsFloat();
				contact.dx 	= (arg++)->AsFloat();
				contact.dy 	= (arg++)->AsFloat();
				contact.width = (arg++)->AsFloat();
				contact.height = (arg++)->AsFloat();
				//contact.pressure = (arg++)->AsFloat();

				if( arg != m.ArgumentsEnd() )
					throw osc::ExcessArgumentException();

//				if(currFrame.size() < numContacts - 1)
//				{
					currFrame.push_back(contact);
					//Send to listener
					gestureListener.updateFrame(currFrame);
//				}
//				else
//					std::cout << "number of contacts inconsistent with frm msg";
			}
			else if(strcmp(m.AddressPattern(), "/tuio2d/sid") == 0){
				numContacts = m.ArgumentCount();
			}
		}catch( osc::Exception& e){
			std::cout << "error while parsing message: "
			<< m.AddressPattern() << ": " << e.what() << "\n";
		}
	}


};


void initMultitouchOscReceiver(int port, GestureCollector collector )
{
	MultitouchOscReceiver oscListener;

	UdpListeningReceiveSocket s(IpEndpointName( IpEndpointName::ANY_ADDRESS, port ), &oscListener );

	std::cout << "Listening for input on port " << port << "...\n";

	oscListener.gestureListener = collector;
	s.RunUntilSigInt();

	std::cout << "Done listening to OSC.\n";


}


#endif /* MULTITOUCHOSCRECEIVER_H_ */
