#pragma once
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include "Message.h"


namespace email {

	class Client {
	protected:
		const std::string username;
		const std::string password;
		const std::string smtpUrl;
		const std::string imapUrl;
		std::string lastCurlError;
		bool curlDebugOutput;

	public:
		std::map<unsigned int, Message> Inbox;
		const unsigned int inboxMaxElements;

	public:
		Client(const std::string username, const std::string pw, const std::string smtpUrl, const std::string imapUrl, unsigned int inboxMaxElements = 50U);
		virtual ~Client();

		bool Send(Message& message);
		bool Fetch(void);
		const std::string& GetLastCurlError(void);
		void SetCurlDebugOutput(bool mode);

	protected:
		static size_t PayloadSource(void* ptr, size_t size, size_t nmemb, void* userp);
		bool GetInboxUids(std::vector<unsigned int>& uids);
		static size_t UidResponseTarget(void* ptr, size_t size, size_t nmemb, void* userp);
		static size_t PayloadTarget(void* ptr, size_t size, size_t nmemb, void* userp);

	private:
		Client();

	};

}