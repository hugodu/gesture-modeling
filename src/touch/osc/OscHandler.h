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

#include "gesture/Gestures.h"

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
	bool isParameterizing;


	OscHandler()
	{
		cout << "Initing osc streams" << endl;
		initOutStream();
		gestrSampleStart = false;
		isParameterizing = false;
	}

	~OscHandler()
	{
	}

	void handleGestrSample(osc::ReceivedMessageArgumentIterator & arg,
			const osc::ReceivedMessage & m)
	{
		const char *smpl = (arg++)->AsString();
		if (strcmp(smpl, "start") == 0)
		{
			const char *gestureName = (arg++)->AsString();
			gestrSampleStart = true;
			listener->startSample(gestureName);
		}
		else if (strcmp(smpl, "end") == 0)
		{
			gestrSampleStart = false;
			listener->endSample();
		}
		else if (strcmp(smpl, "set") == 0)
		{
			Contact contact = createContactFromOscArgs(arg, m);
			gestrFrame.push_back(contact);
		}
		else if (strcmp(smpl, "fseq") == 0)
		{
			listener->updateFrame(gestrFrame);
			gestrFrame.clear();
		}
		else
			cout << "MsgParseError";

	}

	void handleGestrAction(osc::ReceivedMessageArgumentIterator & arg,
			const osc::ReceivedMessage & m)
	{
		const char *actionString 	= (arg++)->AsString();
		vector<string> actionParams;
		while (arg != m.ArgumentsEnd())
			actionParams.push_back((arg++)->AsString());

		vector<string> actionResult = listener->gestureAction(actionString, actionParams);
		sendGestrActionResults(actionString, actionResult);
	}

	void handleTUIO(osc::ReceivedMessageArgumentIterator arg,
			const osc::ReceivedMessage & m)
	{
		const char *param = (arg++)->AsString();
		if (strcmp(param, "set") == 0)
		{
			Contact contact = createContactFromOscArgs(arg, m);
			cloneContactToOutStream(contact);
			liveFrame.push_back(contact);
		}
		else if (strcmp(param, "alive") == 0)
		{
			vector<int> currIds = getCurrIdsAndClone(arg, m);

			//Check for change in fingers on screen.
			bool fingersRaised = isFingerRaised(currIds);
			bool fingersLowered = isFingerLowered(currIds);

			if(fingersLowered)
			{
//				cout << "FingersLowered" << endl;
				listener->startSample("");
			}
			if(fingersRaised)
			{
//				cout << "FingersRaised" << endl;
				isParameterizing = false;
			}
			liveIds = currIds;

			if(currIds.size() == 0 && listener->sampleSize() < 10)
			{
				listener->clearSample();
				return; //Had nothing, have nothing worth checking, nothing to do.
			}


			bool currSegmentIsNowStatic = false;
			if(listener->sampleSize() > 10) //Don't check if only 10 frames are collected
			{
				if (listener->sampleIsNowStatic()) //Check to see if sample has stopped moving
				{
					if(listener->sampleIsOnlyStatic()) //Entire sample hasn't moved.
						listener->startSample(""); //Restart collection of segment
					else //Delimiter found. Sample is valid if non-static portion of gesture is > 10 frames
					{
						currSegmentIsNowStatic = listener->sampleSize() > 10;
					}
				}
//				else
//					cout << "Frame is not Static, collected: " << listener->sampleSize() << endl;
				//Two delimiters. currSegmentIsNowStatic or fingersRaised
				if (currSegmentIsNowStatic || (fingersRaised && listener->sampleSize() > 10))
				{
					cout << "Valid Segment with size: " << listener->sampleSize() << endl;

					//Segment should be classified
					listener->endSample();
					outStream->Clear();
					const char* gestrAction = "classify";
					vector<string> actionResult = listener->gestureAction(gestrAction, vector<string>());
					if (actionResult.size() > 2)
					{
						sendGestrActionResults(gestrAction, actionResult);

						if(actionResult[1] == "rotateRight") // Test case
						{
							isParameterizing = true;
						}
					}
					listener->startSample(""); //Allow a new segment to begin.
				}
			}
		}
		else if (strcmp(param, "fseq") == 0)
		{
			if (!outStream->IsBundleInProgress())
				*outStream << osc::BeginBundleImmediate;

			*outStream << osc::BeginMessage("/tuio/2Dcur") << "fseq" << (arg++)->AsInt32() << osc::EndMessage;
			*outStream << osc::EndBundle;
			if (!gestrSampleStart && liveFrame.size() > 0)
			{
				listener->updateFrame(liveFrame);
				if(isParameterizing)
				{
					map<string, vector<double> > params = listener->parameterize();
					sendGestrParams(params);
				}

				liveFrame.clear();
			}
			sendStream();
		}

	}

	void ProcessMessage(const osc::ReceivedMessage & m,
			const IpEndpointName & remoteEndpoint)
	{
		try
		{
			osc::ReceivedMessageArgumentIterator arg = m.ArgumentsBegin();
			if (strcmp(m.AddressPattern(), "/gestr/sample") == 0)
			{
				handleGestrSample(arg, m);
			}
			else if (strcmp(m.AddressPattern(), "/gestr/action") == 0)
			{
				handleGestrAction(arg, m);
			}
			else if (strcmp(m.AddressPattern(), "/tuio/2Dcur") == 0)
			{
				handleTUIO(arg, m);
			}
		} catch (osc::Exception& e)
		{
			cout << "error while parsing message: " << m.AddressPattern()
					<< ": " << e.what() << endl;
		}
	}

private:
	typedef map<string, vector<double> > paramValMapT;
	void initOutStream()
	{
		outStream = new osc::OutboundPacketStream(buffer, OUTPUT_BUFFER_SIZE);
	}

	void sendStream()
	{
		UdpTransmitSocket socket = UdpTransmitSocket(IpEndpointName(ADDRESS,
				OUTPUT_PORT));
		socket.Send(outStream->Data(), outStream->Size());
		outStream->Clear();
	}

	bool isFingerLowered(vector<int> & currIds)
	{
		bool fingerLowered = false;
		for (size_t i = 0; i < currIds.size(); i++)
			if (find(liveIds.begin(), liveIds.end(), currIds[i]) == liveIds.end())
				fingerLowered = true;
		return fingerLowered;
	}
	bool isFingerRaised(vector<int> & currIds)
	{
		bool fingerRaised = false;
		for (size_t i = 0; i < liveIds.size(); i++)
			if (find(currIds.begin(), currIds.end(), liveIds[i]) == currIds.end())
				fingerRaised = true;
		return fingerRaised;
	}
	void cloneContactToOutStream(Contact & contact)
	{
		if (!outStream->IsBundleInProgress())
			*outStream << osc::BeginBundleImmediate;
		*outStream << osc::BeginMessage("/tuio/2Dcur") << "set";
		*outStream << contact.id << contact.x << contact.y << contact.dx
				<< contact.dy << 0.0 << contact.width << contact.height;
		*outStream << osc::EndMessage;
	}

	Contact createContactFromOscArgs(
			osc::ReceivedMessageArgumentIterator & arg,
			const osc::ReceivedMessage & m)
	{
		long int numArgs = m.ArgumentCount();
		Contact contact;
		if(numArgs == 9) //Most probably from CCV (including the 'set' message)
		{
			contact.id = (arg++)->AsInt32();
			contact.x = (arg++)->AsFloat();
			contact.y = (arg++)->AsFloat();
			contact.dx = (arg++)->AsFloat();
			contact.dy = (arg++)->AsFloat();
			arg++;
			contact.width = (arg++)->AsFloat();
			contact.height = (arg++)->AsFloat();
			contact.pressure = 0;
		}
		else if(numArgs == 7) //From TUIO Simulator (Profile: /tuio/2Dcur set s x y X Y m)
		{
			contact.id = (arg++)->AsInt32();
			contact.x = (arg++)->AsFloat();
			contact.y = (arg++)->AsFloat();
			contact.dx = (arg++)->AsFloat();
			contact.dy = (arg++)->AsFloat();
			arg++; //We don't really use motion acceleration ... yet.
		}

		if (arg != m.ArgumentsEnd())
			throw osc::ExcessArgumentException();
		return contact;
	}
	vector<int> getCurrIdsAndClone(osc::ReceivedMessageArgumentIterator arg,
			const osc::ReceivedMessage & m)
	{
		if (!outStream->IsBundleInProgress()
				&& !outStream->IsMessageInProgress())
			*outStream << osc::BeginBundleImmediate;

		*outStream << osc::BeginMessage("/tuio/2Dcur") << "alive";
		vector<int> currIds;
		while (arg != m.ArgumentsEnd())
		{
			int id = (arg++)->AsInt32();
			*outStream << id;
			currIds.push_back(id);
		}
		*outStream << osc::EndMessage;
		return currIds;
	}

	/**
	 * Generic method for any Gestr Action
	 */
	void sendGestrActionResults(const char *actionString,
			vector<string> actionResult)
	{
		outStream->Clear();
		if (!outStream->IsBundleInProgress())
			*outStream << osc::BeginBundleImmediate;

		*outStream << osc::BeginMessage("/gestr/action");
		cout << "Action performed: " << actionString << "\n\tResult: ";
		for (size_t i = 0; i < actionResult.size(); i++)
		{
			*outStream << actionResult[i].c_str();
			cout << actionResult[i] << ", ";
		}
		*outStream << osc::EndMessage;
		cout << endl;
		sendStream();
	}
	void sendGestrParams(paramValMapT namedParams)
	{
		if (!outStream->IsBundleInProgress())
			*outStream << osc::BeginBundleImmediate;

		BOOST_FOREACH(paramValMapT::value_type &namedParamValPair, namedParams)
		{
			*outStream << osc::BeginMessage("/gestr/action");
			*outStream << "param_update";
			const string &name =  namedParamValPair.first;
			*outStream << name.c_str();
			BOOST_FOREACH(double paramVal, namedParamValPair.second)
			{
				*outStream << paramVal;
			}
			*outStream << osc::EndMessage;
		}
		sendStream();
	}


};

void initMultitouchOscReceiver(GestureCollector *collector)
{
	OscHandler oscListener;
	UdpListeningReceiveSocket s(IpEndpointName(IpEndpointName::ANY_ADDRESS,
			IN_PORT), &oscListener);

	cout << "Now Listening for input on port " << IN_PORT << "..." << endl;

	cout << "Output packets to port: " << OUTPUT_PORT << endl;

	oscListener.listener = collector;
	oscListener.inSock = &s;
	s.RunUntilSigInt();
	cout << "Done listening to OSC.\n";

}

#endif /* MULTITOUCHOSCRECEIVER_H_ */
