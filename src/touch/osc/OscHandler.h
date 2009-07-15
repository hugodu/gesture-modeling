/*
 * MultitouchOscReceiver.h
 *
 *      Author: sashikanth
 */

#ifndef MULTITOUCHOSCRECEIVER_H_
#define MULTITOUCHOSCRECEIVER_H_

#include <vector>
#include <algorithm>
#include "osc/OscReceivedElements.h"
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"

#define ADDRESS "127.0.0.1"
#define OUTPUT_BUFFER_SIZE 4096
#define IN_PORT 3330
#define OUTPUT_PORT 3333


using namespace std;
ContactSetFrame frame;
class OscHandler: public osc::OscPacketListener
{
public:

	ContactSetFrame gestrFrame;
	ContactSetFrame liveFrame;
	GestureCollector *listener;
	osc::OutboundPacketStream* outStream;
	UdpListeningReceiveSocket* inSock;
	bool gestrSampleStart;

	char buffer[OUTPUT_BUFFER_SIZE];

	vector<int> liveIds;

	OscHandler()
	{
		cout << "Initing osc streams" << endl;
		initOutStream();
		gestrSampleStart = false;
	}

	~OscHandler()
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
				//A Sample is being received, Send event to listeners.
				const char* smpl = (arg++)->AsString();
//				cout << "sample: " << smpl << "." << endl;
				if (strcmp(smpl, "start") == 0)
				{
					const char* gestureName = (arg++)->AsString();
					gestrSampleStart = true;
					listener->startSample(gestureName);
				}
				else if (strcmp(smpl, "end") == 0)
				{
					gestrSampleStart = false;
					listener->endSample();
				}
				else if(strcmp(smpl, "set") == 0)
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
					contact.pressure = 0;

					if (arg != m.ArgumentsEnd())
						throw osc::ExcessArgumentException();

					gestrFrame.push_back(contact);
				}
				else if(strcmp(smpl, "fseq") == 0)
				{
					//Sent at end of frame. Send Frame to listener
					listener->updateFrame(gestrFrame);
					//frame ends now
					gestrFrame.clear();
				}
				else
					cout << "MsgParseError";
			}
			else if (strcmp(m.AddressPattern(), "/gestr/action") == 0)
			{
				const char* actionString 	= (arg++)->AsString();
				const char* actionParam 	= "none";
				if (arg != m.ArgumentsEnd())
					actionParam = (arg++)->AsString();
				outStream->Clear();
				vector<string> actionResult = listener->gestureAction(actionString, actionParam);

				if(!outStream->IsBundleInProgress())
					*outStream << osc::BeginBundleImmediate;
				*outStream 	<< osc::BeginMessage("/gestr/action");
				cout << "Action performed: " << actionString << "\n\tResult: ";
				for(size_t i = 0; i < actionResult.size(); i++)
				{
					*outStream << actionResult[i].c_str();
					cout << actionResult[i] << ", ";
				}
				*outStream 	<< osc::EndMessage;
				cout << endl;
				//These are important messages. Send the stream asap.
				sendStream();

			}
			//Handle stream from CCV
			//This section handles only the cloning of the stream to OUTPUT_PORT
			else if (strcmp(m.AddressPattern(), "/tuio/2Dcur") == 0)
			{

				const char* param = (arg++)->AsString();
				if(strcmp(param, "set") == 0)
				{
					//Set is the first message in the bundle. If bundle isn't ready. Reinitialize it.
					if(!outStream->IsBundleInProgress())
						*outStream << osc::BeginBundleImmediate;

					Contact contact;

					contact.id = (arg++)->AsInt32();
					contact.x = (arg++)->AsFloat();
					contact.y = (arg++)->AsFloat();
					contact.dx = (arg++)->AsFloat();
					contact.dy = (arg++)->AsFloat();
					arg++; // Something ccv is sending, don't care.
					contact.width = (arg++)->AsFloat();
					contact.height = (arg++)->AsFloat();
					contact.pressure = 0;

					*outStream 	<< osc::BeginMessage("/tuio/2Dcur") << "set";
					*outStream 	<< contact.id << contact.x << contact.y << contact.dx << contact.dy << 0.0
								<< contact.width << contact.height;
//					*outStream 	<< (arg++)->AsFloat();
//					*outStream 	<< (arg++)->AsFloat(); 	// id, x, y
//					*outStream 	<< (arg++)->AsFloat() << (arg++)->AsFloat() << (arg++)->AsFloat(); 	//dx, dy, (?)
//					*outStream 	<< (arg++)->AsFloat() << (arg++)->AsFloat(); 						//width, height
					*outStream 	<< osc::EndMessage;

					liveFrame.push_back(contact);

					if (arg != m.ArgumentsEnd())
						throw osc::ExcessArgumentException();
				}
				else if(strcmp(param, "alive") == 0)
				{
					if(!outStream->IsBundleInProgress() && !outStream->IsMessageInProgress()) // In case of no contacts
						*outStream  << osc::BeginBundleImmediate;

					*outStream << osc::BeginMessage("/tuio/2Dcur") << "alive";
					vector<int> currIds;
					while(arg != m.ArgumentsEnd())
					{
						int id = (arg++)->AsInt32();
						*outStream << id;
						currIds.push_back(id);
					}
					*outStream << osc::EndMessage;

					bool newSample = false;
					bool endSample = false;
					for(size_t i = 0; i < currIds.size(); i++)
						if(find(liveIds.begin(), liveIds.end(), currIds[i]) == liveIds.end()) //new touch. restart sample.
							newSample = true;
					for(size_t i = 0; i < liveIds.size(); i++)
						if(find(currIds.begin(), currIds.end(), liveIds[i]) == currIds.end()) //lost a contact. end sample
							endSample = true;
					liveIds = currIds;
					if(newSample)
						listener->startSample("");
					else if(endSample) //Don't end a sample when starting it.
					{
						listener->endSample();
						listener->gestureAction("classify", "");
					}
				}
				else if(strcmp(param, "fseq") == 0)
				{
					if(!outStream->IsBundleInProgress() )
						*outStream  << osc::BeginBundleImmediate; // In case we missed the alive packet for this bundle
					*outStream << osc::BeginMessage("/tuio/2Dcur") << "fseq" << 0 << osc::EndMessage;
					*outStream << osc::EndBundle;
					if(!gestrSampleStart) //If we're receiving a gestr sample, ignore the live stream.
					{
						listener->updateFrame(liveFrame);
						liveFrame.clear();
					}
					sendStream();
				}
			}
		} catch (osc::Exception& e)
		{
			cout << "error while parsing message: " << m.AddressPattern()
			<< ": " << e.what() << endl;
		}
	}

private:
	void initOutStream()
	{
		outStream 	= new osc::OutboundPacketStream( buffer, OUTPUT_BUFFER_SIZE );
	}

	void sendStream()
	{
		UdpTransmitSocket socket = UdpTransmitSocket( IpEndpointName( ADDRESS, OUTPUT_PORT) );
		socket.Send(outStream->Data(), outStream->Size());
		outStream->Clear();
	}
};

void initMultitouchOscReceiver(GestureCollector *collector)
{
	OscHandler oscListener;
	UdpListeningReceiveSocket s(IpEndpointName(IpEndpointName::ANY_ADDRESS, IN_PORT), &oscListener);

	cout << "Now Listening for input on port " << IN_PORT << "..." <<endl;

	cout << "Output packets to port: " << OUTPUT_PORT << endl;

	oscListener.listener = collector;
	oscListener.inSock	= &s;
	s.RunUntilSigInt();
	cout << "Done listening to OSC.\n";

}

#endif /* MULTITOUCHOSCRECEIVER_H_ */
