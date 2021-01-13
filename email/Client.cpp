#include "Client.h"
#include <iostream>
#include <curl/curl.h>


namespace email {

	Client::Client(const std::string username, const std::string pw, const std::string smtpUrl, const std::string imapUrl, unsigned int inboxMaxElements):
		username(username), password(pw), smtpUrl(smtpUrl), imapUrl(imapUrl), inboxMaxElements(inboxMaxElements), curlDebugOutput(false) {
		lastCurlError = "";
	}

	Client::Client(): username(""), password(""), smtpUrl(""), imapUrl(""), inboxMaxElements(0), curlDebugOutput(false) {}

	Client::~Client() {}


	bool Client::Send(Message& message) {

		CURL* curl;
		CURLcode res = CURLE_OK;
		curl_slist* recipients = NULL;

		curl = curl_easy_init();
		if (!curl) {
			lastCurlError.assign("curl_easy_init failed");
			curl_easy_cleanup(curl);
			return false;
		}

		curl_easy_setopt(curl, CURLOPT_USERNAME, this->username.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, this->password.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, this->smtpUrl.c_str());
		res = curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
		if (res != CURLE_OK) {
			lastCurlError.assign("Send: SSL not available");
			curl_easy_cleanup(curl);
			return false;
		}

		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, message.Sender().GetMail().c_str());				

		std::vector<Address> recps = message.Receivers();
		for (unsigned int i = 0; i < recps.size(); i++) {
			recipients = curl_slist_append(recipients, recps[i].GetMail().c_str());
		}

		//TODO: Deal with CC recievers
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, PayloadSource);
		curl_easy_setopt(curl, CURLOPT_READDATA, &message);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		if (curlDebugOutput) {
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}
		else {
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		}

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			if (curlDebugOutput) {
				std::cout << "\nCURL perform error: " << curl_easy_strerror(res) << std::endl;
			}
			lastCurlError.assign(curl_easy_strerror(res));

			curl_slist_free_all(recipients);
			curl_easy_cleanup(curl);
			return false;
		}

		curl_slist_free_all(recipients);
		curl_easy_cleanup(curl);
		return true;
	}


	bool Client::Fetch(void) {

		CURL* curl;
		CURLcode res = CURLE_OK;
		std::vector<unsigned int> uids;
		std::string url;

		if (GetInboxUids(uids) != true) {
			if (lastCurlError.length() == 0) {
				lastCurlError.assign("GetInboxUids failed");
			}				
			return false;
		}

		curl = curl_easy_init();
		if (!curl) {
			lastCurlError.assign("curl_easy_init failed");
			curl_easy_cleanup(curl);
			return false;
		}
		curl_easy_setopt(curl, CURLOPT_USERNAME, this->username.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, this->password.c_str());
		res = curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
		if (res != CURLE_OK) {
			lastCurlError.assign("Fetch: curl_easy_setopt failed: SSL not available");
			curl_easy_cleanup(curl);
			return false;
		}
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PayloadTarget);

		if (curlDebugOutput) {
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}
		else {
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		}

		//max elements		
		unsigned int elements;
		if (uids.size() > inboxMaxElements) {
			elements = inboxMaxElements;
		}
		else {
			elements = uids.size();
		}

		for (size_t i = 0; i < elements; i++) {

			if (Inbox.count(uids[i]) != 0) {
				continue;
			}
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &Inbox[uids[i]]);

			url.assign(imapUrl + "/INBOX" + "/;UID=" + std::to_string(uids[i]));
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				if (curlDebugOutput) {
					std::cout << "\nCURL perform error: " << curl_easy_strerror(res) << std::endl;
				}
				lastCurlError.assign(curl_easy_strerror(res));
				curl_easy_cleanup(curl);
				return false;
			}

		}
		curl_easy_cleanup(curl);
		return true;
	}


	const std::string& Client::GetLastCurlError(void) {
		return lastCurlError;
	}

	void Client::SetCurlDebugOutput(bool mode) {
		curlDebugOutput = mode;
	}


	size_t Client::PayloadSource(void* ptr, size_t size, size_t nmemb, void* userp) {
		
		Message* msg = reinterpret_cast<Message*>(userp);

		if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1) || (msg->bytesLeft == 0)) {
			return 0;
		}

		//TODO: Handle if size*nmemb < bytes but not 0 etc which is handled above, (via bytesLeft & bytes difference)

		if ((nmemb * size) >= msg->bytes) {
			msg->bytesLeft = 0;
			return msg->Payload().copy(reinterpret_cast<char*>(ptr), msg->bytes, 0U);
		}

		return 0;
	}


	bool Client::GetInboxUids(std::vector<unsigned int>& uids) {

		CURL* curl;
		CURLcode res = CURLE_OK;
		std::string uidResponse;

		curl = curl_easy_init();
		if (!curl) {
			lastCurlError.assign("curl_easy_init failed");
			curl_easy_cleanup(curl);
			return false;
		}
		curl_easy_setopt(curl, CURLOPT_USERNAME, this->username.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, this->password.c_str());
		res = curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
		if (res != CURLE_OK) {
			lastCurlError.assign("curl_easy_setopt failed: SSL not available");
			curl_easy_cleanup(curl);
			return false;
		}
		curl_easy_setopt(curl, CURLOPT_URL, std::string(imapUrl + "/INBOX").c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, UidResponseTarget);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &uidResponse);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "UID SEARCH ALL");

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {			
			lastCurlError.assign(curl_easy_strerror(res));
			curl_easy_cleanup(curl);
			return false;
		}

		// parsing of UID response begins here
		size_t currentPos = std::string::npos;
		size_t numBegin;
		size_t numEnd;
		unsigned int num;

		unsigned int i = 0U;
		while (i <= inboxMaxElements) {
			numEnd = uidResponse.find_last_of("0123456789", currentPos);	
			if (numEnd == std::string::npos) {
				break;
			}
			numBegin = uidResponse.find_last_not_of("0123456789", numEnd - 1U) + 1U;						

			num = std::stoul(uidResponse.substr(numBegin, (numEnd - numBegin + 1U)));
			uids.push_back(static_cast<unsigned int>(num));

			currentPos = numBegin - 1U;
		}
		curl_easy_cleanup(curl);
		return true;
	}

	
	size_t Client::UidResponseTarget(void* ptr, size_t size, size_t nmemb, void* userp) {
		
		std::string* uidStr = static_cast<std::string*>(userp);		
		uidStr->assign((char*)ptr, size * nmemb);

		return nmemb * size;
	}


	size_t Client::PayloadTarget(void* ptr, size_t size, size_t nmemb, void* userp) {

		if (size == 0 || nmemb == 0) {
			return 0;
		}

		Message* msg = reinterpret_cast<Message*>(userp);
		std::string payload;
		payload.assign((char*)ptr, size * nmemb);
		msg->NewFromPayload(payload);
		
		//return payload.size(); // should be == size*nmemb, returns "bytes actually taken care of"
		return size * nmemb;
	}

}