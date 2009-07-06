/*
 * MultitouchOscReceiver.h
 *
 *      Author: sashikanth
 */

#ifndef MULTITOUCHOSCRECEIVER_H_
#define MULTITOUCHOSCRECEIVER_H_

#include "osc/OscReceivedElements.h"
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"

#define ADDRESS "127.0.0.1"
#define OUTPUT_BUFFER_SIZE 4096
#define OUTPUT_PORT 3336


using namespace std;
ContactSetFrame frame;
class MultitouchOscReceiver: public osc::OscPacketListener
{
public:

	ContactSetFrame currFrame;
	unsigned int numContacts;
	GestureCollector *listener;
	UdpTransmitSocket* transmitSocket;
	osc::OutboundPacketStream* packetStream;

	MultitouchOscReceiver()
	{
		cout << "Initing output osc stream" << endl;
		UdpTransmitSocket _socket( IpEndpointName( ADDRESS, OUTPUT_PORT) );
		char buffer[OUTPUT_BUFFER_SIZE];
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		transmitSocket 	= &_socket;
		packetStream 	= &p;
		cout << "Socket isBound: " << transmitSocket->IsBound() << endl;
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
				const char* actionString 	= (arg++)->AsString();
				const char* actionParam = "none";
				if (arg != m.ArgumentsEnd())
				{
					actionParam = (arg++)->AsString();
				}

				listener->gestureAction(actionString, actionParam);
			}

			else if (strcmp(m.AddressPattern(), "/tuio2d/frm") == 0)
			{
				//New frame starts now
				currFrame.clear();
			}
			else if (strcmp(m.AddressPattern(), "/tuio/2Dcur") == 0)
			{
				const char* param = (arg++)->AsString();
				if(strcmp(param, "set") == 0)
				{
					Contact contact;

					contact.id = (arg++)->AsInt32();
					contact.x = (arg++)->AsFloat();
					contact.y = (arg++)->AsFloat();
					contact.dx = (arg++)->AsFloat();
					contact.dy = (arg++)->AsFloat();
					arg++; // Something ccv is sending, don't care.
					contact.width = (arg++)->AsFloat();
					contact.height = (arg++)->AsFloat();
					//contact.pressure = (arg++)->AsFloat();
					*packetStream 	<< osc::BeginBundleImmediate
										<< osc::BeginMessage("/tuio2d/cur")
											<< contact.id << contact.x << contact.y
											<< contact.dx << contact.dy << contact.width << contact.height
										<< osc::EndMessage
									<< osc::EndBundle;
					cout << "Sending" << endl;

					if(!transmitSocket->IsBound())
					{
						transmitSocket->Connect(IpEndpointName(ADDRESS, OUTPUT_PORT));
						cout << "Reconnect" << endl;
					}

					transmitSocket->Send(packetStream->Data(), packetStream->Size());

					if (arg != m.ArgumentsEnd())
						throw osc::ExcessArgumentException();

					currFrame.push_back(contact);
				}
				else if(strcmp(param, "alive") == 0)
				{
					//Sent at end of frame.
					numContacts = m.ArgumentCount();
					//Send Frame to listener
					listener->updateFrame(currFrame);
				}
				else if(strcmp(param, "fseq") == 0)
				{

					//frame ends now
					currFrame.clear();
				}
			}
//			else if (strcmp(m.AddressPattern(), "/tuio/2Dcur") == 0)
//			{
//				//Sent at end of frame.
//				numContacts = m.ArgumentCount();
//				//Send Frame to listener
//				listener->updateFrame(currFrame);
//			}
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

	cout << "Cloning tuio packets to port: " << OUTPUT_PORT << endl;

	oscListener.listener = collector;
	s.RunUntilSigInt();

	cout << "Done listening to OSC.\n";

}

#endif /* MULTITOUCHOSCRECEIVER_H_ */
