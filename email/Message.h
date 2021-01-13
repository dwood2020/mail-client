#pragma once
#include <string>
#include <ctime>
#include <vector>
#include "DateTime.h"
#include "Address.h"


namespace email {

	class Message {
	protected:
		Address from;		
		std::vector<Address> to;
		//TODO: CC
		std::string id;
		std::string subject;
		std::string body;
		
	public:
		size_t bytes;
		size_t bytesLeft;
		DateTime dateTime;

	public:
		Message(const Address& from, const std::vector<Address>& to, const std::string& subject, const std::string& body);
		Message();
		virtual ~Message();
		void NewFromPayload(std::string payload);

		const std::string Payload(void);
		const Address& Sender(void) const;
		const std::vector<Address>& Receivers(void) const;
		const std::string& Subject(void) const;
		const std::string& Body(void) const;

	protected:		
		const std::string GenerateId(const std::string& provider) const;
		void CalculateBytes(void);
		
	};

}

