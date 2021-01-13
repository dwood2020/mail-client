#pragma once
#include <string>


namespace email {

	class DateTime {
	public:
		static const std::string months[12];
		static const std::string days[7];

	public:
		int year;
		int month;
		int day;
		int weekday;
		int hour;
		int min;
		int sec;
		int tmZoneOffset;

	public:
		DateTime();
		virtual ~DateTime();

		DateTime& Now(void);		
		void FromString(const std::string str);
		const std::string ToString(void);

	};

}
