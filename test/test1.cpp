#include <iostream>
#include <fstream>
#include "../email/email.hpp"


void TestSend(email::Client* client) {

	using namespace email;

	Address from("mr-test-max@web.de", "Judas Made");
	Address empfaenger1("my-test-friend@gmx.de");
	Address self("mr-test-max@web.de", "Madenjudas");

	std::vector<Address> to;
	to.push_back(empfaenger1);
	to.push_back(self);

	std::string subject = "New Test Mail";
	std::string body = "Hello!\n\nThis is a test message from test1.cpp: TestSend() on 01.12.2020.\n\nGreetings!";

	Message message(from, to, subject, body);

	bool sendRes = client->Send(message);
	if (!sendRes) {
		std::cout << "Send lastCurlError: " << client->GetLastCurlError() << std::endl;
	}
	else {
		std::cout << "Sent successfully." << std::endl;
	}

}


void TestReceive(email::Client* client) {

	if (client->Fetch() != true) {
		std::cout << client->GetLastCurlError() << std::endl;
		return;
	}
	
	std::cout << "Fetched successfully.\n";
	std::cout << "Inbox size: " << client->Inbox.size() << std::endl;

	// filedump:	
	std::map<unsigned int, email::Message>::iterator it;
	std::ofstream fout;
	fout.open("inboxDump.txt", std::ios::out | std::ios::trunc);
	if (fout.is_open()) {
		for (it = client->Inbox.begin(); it != client->Inbox.end(); it++) {
			fout << it->first << ":\n=============================\n " << std::endl;
			fout << "From: " << it->second.Sender().GetFull();
			fout << "\nTo: ";
			for (email::Address to : it->second.Receivers()) {
				fout << to.GetFull() << "; ";
			}
			fout << "\nDate: " << it->second.dateTime.ToString()
				<< "\nSubject: " << it->second.Subject() << "\n--------------------------\n" << it->second.Body() << std::endl;
		}
	}
	fout.close();

	// print uuids
	for (it = client->Inbox.begin(); it != client->Inbox.end(); it++) {
		std::cout << "UID " << it->first << ": " << it->second.bytes << " bytes" << std::endl;
	}

}


int main(void) {	

	email::Client client("mr-test-max@web.de", "testpassword123", "smtp://smtp.web.de:587", "imap://imap.web.de", 10U);
	client.SetCurlDebugOutput(false);
	
	//TestSend(&client);
	TestReceive(&client);

	std::cin.get();
	return 0;
}