#include "Message.h"
#include <sstream>
#include <cmath>
#include <cstdlib>


namespace email {
	
	Message::Message(const Address& from, const std::vector<Address>& to, const std::string& subject, const std::string& body) {
		this->from = from;		
		this->to = to;
		this->id = GenerateId(from.GetMail().substr(from.GetMail().find_last_of('@') + 1, std::string::npos));		
		this->subject = subject;
		this->body = body;
		this->dateTime.Now();

		CalculateBytes();
	}

	Message::~Message(){}

	Message::Message() {		
		this->id = "";
		this->subject = "";
		this->body = "";
		bytes = 0;
		bytesLeft = 0;
	}

	void Message::NewFromPayload(std::string payload) {

		size_t posBegin = std::string::npos;
		size_t posEnd = std::string::npos;

		std::string word;
		size_t begin;
		size_t end;

		if (subject.empty()) {
			word.assign("Subject: ");
			begin = payload.find(word);
			if (begin != std::string::npos) {
				begin += word.length();
				end = payload.find_first_of("\r\n", begin);
				subject = payload.substr(begin, end - begin);

				//postprocessing
				while (subject.find_first_of(' ') == 0) {
					subject.erase(0U, 1U);
				}
			}
		}

		if (from.Empty()) {
			word = "From: ";
			begin = payload.find(word);
			if (begin != std::string::npos) {
				begin += word.length();
				begin = payload.find_first_not_of(' ', begin);
				end = payload.find_first_of("\r\n", begin);

				from.Build(payload.substr(begin, end - begin));				
			}
		}


		if (to.empty()) {
			posBegin = payload.find("To: ");
			if (posBegin != std::string::npos) {				

				posBegin = payload.find_first_not_of(' ', posBegin + 3U);
				
				size_t tempEnd = posBegin;
				size_t end = posBegin;
				size_t nextPart = payload.find(": ", posBegin);

				while (tempEnd != std::string::npos) {
					tempEnd = payload.find_first_of("\r\n", tempEnd + 1U);
					if (tempEnd < nextPart) {
						end = tempEnd;
					}
					else {
						break;
					}
				}
				std::string sub = payload.substr(posBegin, end - posBegin);

				std::vector<std::string> receivers;
				size_t commaPos = 0U;
				size_t lastCommaPos = 0U;
				do {
					lastCommaPos = commaPos;
					commaPos = sub.find_first_of(',', commaPos + 1U);
					receivers.push_back(sub.substr(lastCommaPos, commaPos - lastCommaPos));
				} while (commaPos != std::string::npos);
				

				// clean up
				size_t pos;
				for (unsigned int i = 0; i < receivers.size(); i++) {
					pos = 0U;

					do {
						pos = receivers[i].find_first_of(",\r\n\t");
						if (pos != std::string::npos) {							
							receivers[i].erase(pos, 1U);							
						}
					} while (pos != std::string::npos);

					// then hand over to Address object
					Address addr;
					addr.Build(receivers[i]);
					to.push_back(addr);

				}				
			}
		}

		if (id.empty()) {
			posBegin = payload.find("Message-ID: ");
			if (posBegin != std::string::npos) {
				posBegin = payload.find_first_of("<", posBegin);
				posEnd = payload.find_first_of(">", posBegin);
				id.assign(payload.substr(posBegin + 1U, (posEnd - (posBegin + 1U))));
			}
		}

		posBegin = payload.find("Date: ");
		if (posBegin != std::string::npos) {
			posBegin = payload.find_first_of(" ", posBegin);
			posBegin = payload.find_first_not_of(" ", posBegin);
			posEnd = payload.find_first_of("\r\n", posBegin);		
			dateTime.FromString(payload.substr(posBegin, posEnd - posBegin));
		}

		posBegin = payload.find("\r\n\r\n");
		if (posBegin != std::string::npos) {
			body.assign(payload.substr(posBegin + 4));
		}

		posBegin = std::string::npos;
		posEnd = std::string::npos;

		CalculateBytes();
	}


	const std::string Message::Payload(void) {		
		std::string payload;		
		payload += ("Date: " + dateTime.Now().ToString() + "\r\n");				//8+31
		
		std::string receivers;
		int nrReceivers = to.size();
		if (nrReceivers > 0) {			
			receivers.assign(to[0].GetFull());
			for (int i = 1; i < nrReceivers; i++) {
				receivers.append(", ");				
				receivers.append(to[i].GetFull());
			}
		}
		payload += ("To: " + receivers);										//4+ x * (lenReceiver(x) + 2) + (x-1) * 2
																				//new: 4 + x * to[x].bytes + (x-1) * 2
		payload += "\r\n";
		
		payload += ("From: " + from.GetFull() + "\r\n");						//6 + from.bytes + 2


		payload += ("Message-ID: <" + id + ">\r\n");							//13 + x + 3
		payload += ("Subject: " + subject + "\r\n");							//11+x
		payload += "\r\n";	// empty line regarding to RFC5322 specs			//2
		payload += (body + "\r\n\r\n");											//x+4

		return payload;
	}
	

	const Address& Message::Sender(void) const {
		return from;
	}

	const std::vector<Address>& Message::Receivers(void) const {
		return to;
	}

	const std::string& Message::Subject(void) const {
		return subject;
	}

	const std::string& Message::Body(void) const {
		return body;
	}


	const std::string Message::GenerateId(const std::string& provider) const {
		//std::string id = "dcd7cb36-11db-487a-9f3a-e652a9458efd";	

		const int maxLen = 75;
		const char characters[42] = { "abcdefghijklmnopqrstuvwxyz0123456789!#$%&" };

		time_t rawtm;
		time(&rawtm);

#ifdef _MSC_VER		
		struct tm gtmS;
		gmtime_s(&gtmS, &rawtm);
		struct tm* gtm = &gtmS;
#else
		struct tm* gtm = gmtime(&rawtm);
#endif

		std::stringstream ss;
		ss << (gtm->tm_year + 1900);
		if (gtm->tm_mon < 10) {
			ss << "0";
		}
		ss << (gtm->tm_mon + 1);
		if (gtm->tm_mday < 10) {
			ss << "0";
		}
		ss << gtm->tm_mday << "-";

		if (gtm->tm_hour < 10) {
			ss << "0";
		}
		ss << gtm->tm_hour;
		if (gtm->tm_min < 10) {
			ss << "0";
		}
		ss << gtm->tm_min;
		if (gtm->tm_sec < 10) {
			ss << "0";
		}
		ss << gtm->tm_sec << "-";

		int spacesLeft = (maxLen - 9 - 7 - 1 - provider.length()) - 10;	//shorter id for safety
		srand(static_cast<unsigned int>(time(NULL)));
		for (int i = 0; i < spacesLeft; i++) {
			ss << characters[rand() % 41];
		}

		ss << "@" << provider;

		return ss.str();		
	}


	void Message::CalculateBytes(void) {
		bytes = 39;

		bytes += 4;
		for (unsigned int i = 0; i < to.size(); i++) {
			if (i > 0) {
				bytes += 2;
			}			
			bytes += to[i].GetBytes();
			
		}
		bytes += 2;
				
		bytes += (6 + from.GetBytes() + 2);

		bytes += 13 + id.length() + 3;	//Message-ID
		bytes +=  (11 + subject.size()) + 2 + (body.size() + 4);		

		bytesLeft = bytes;
	}

}