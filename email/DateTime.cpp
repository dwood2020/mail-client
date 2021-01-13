#include "DateTime.h"
#include <sstream>
#include <ctime>
#include <cmath>
#include <vector>


namespace email {

	const std::string DateTime::months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	const std::string DateTime::days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	DateTime::DateTime(): year(0), month(1), day(0), weekday(0), hour(0), min(0), sec(0), tmZoneOffset(0) {}


	DateTime::~DateTime() {}

	DateTime& DateTime::Now(void) {
		time_t rawtm;
		time(&rawtm);

#ifdef _MSC_VER		
		struct tm gtmS;		
		gmtime_s(&gtmS, &rawtm);		
		struct tm* gtm = &gtmS;

		struct tm ltmS;
		localtime_s(&ltmS, &rawtm);
		struct tm* ltm = &ltmS;
#else
		struct tm* gtm = gmtime(&rawtm);
		struct tm* ltm = localtime(&rawtm);
#endif

		this->tmZoneOffset = ltm->tm_hour - gtm->tm_hour;

		this->year = gtm->tm_year + 1900;
		this->month = gtm->tm_mon + 1;
		this->day = gtm->tm_mday;
		this->weekday = gtm->tm_wday;
		this->hour = gtm->tm_hour + tmZoneOffset;
		this->min = gtm->tm_min;
		this->sec = gtm->tm_sec;

		return *this;
	}

	const std::string DateTime::ToString(void) {
		// format: "Fri, 29 Nov 2019 10:58:00 +0100"

		std::stringstream ss;
		ss << DateTime::days[this->weekday] << ", ";
		if (this->day < 10) {
			ss << "0";
		}
		ss << this->day << " ";
		ss << DateTime::months[this->month - 1] << " " << this->year << " ";

		if (hour < 10) {
			ss << "0";
		}
		ss << (hour) << ":";

		if (min < 10) {
			ss << "0";
		}
		ss << min << ":";

		if (sec < 10) {
			ss << "0";
		}
		ss << sec;

		ss << " ";
		ss << ((tmZoneOffset >= 0) ? "+" : "-");
		if (abs(tmZoneOffset) < 10) {
			ss << "0";
		}
		ss << tmZoneOffset << "00";

		return ss.str();
	}
	

	void DateTime::FromString(const std::string str) {
		// format: "Fri, 29 Nov 2019 10:58:00 +0100"

		size_t posStart = str.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
		size_t posEnd = str.find_first_of(",", posStart);
		std::string temp = str.substr(posStart, (posEnd - posStart));
		for (int i = 0; i < 7; i++) {
			if (temp.compare(DateTime::days[i]) == 0) {
				this->weekday = i;
				break;
			}
		}

		posStart = str.find_first_of("123456789");
		posEnd = str.find_first_of(" ", posStart);
		temp = str.substr(posStart, (posEnd - posStart));
		this->day = std::stoi(temp);

		posStart = str.find_first_not_of(" ", posEnd);
		posEnd = str.find_first_of(" ", posStart);
		temp = str.substr(posStart, (posEnd-posStart));
		for (int i = 0; i < 12; i++) {
			if (temp.find(DateTime::months[i]) != std::string::npos) {
				this->month = i + 1;
				break;
			}			
		}

		posStart = str.find_first_of("0123456789", posStart);
		posEnd = str.find_first_of(" ", posStart);
		temp = str.substr(posStart, 4);
		this->year = std::stoi(temp);

		posStart = str.find_first_of("0123456789", posEnd);
		posEnd = str.find_first_of(":", posStart);
		temp = str.substr(posStart, (posEnd - posStart));
		this->hour = std::stoi(temp);

		posStart = str.find_first_of("0123456789", posEnd);
		posEnd = str.find_first_of(":", posStart);
		temp = str.substr(posStart, (posEnd - posStart));
		this->min = std::stoi(temp);

		posStart = str.find_first_of("0123456789", posEnd);
		posEnd = str.find_first_not_of("0123456789", posStart);
		temp = str.substr(posStart, (posEnd - posStart));
		this->sec = std::stoi(temp);

		posStart = str.find_first_of("+-");
		posEnd = str.find_last_of("0123456789", posStart);
		if (posStart != std::string::npos) {
			int vz;
			vz = (str.at(posStart) == '+') ? 1 : -1;

			posStart = str.find_first_of("0123456789", posStart);
			std::string temp = str.substr(posStart, posEnd - posStart);
		
			std::vector<int> pos;
			pos.resize(temp.length());
			std::stringstream ss;
			for (unsigned int i = 0; i < temp.length(); i++) {
				ss << temp.at(i);
				ss >> pos[i];
				ss.clear();
			}

			this->tmZoneOffset = (pos[0] * 10 + pos[1] * 1) * vz;			
			//TODO: handle half hours
		}
		
	}

}