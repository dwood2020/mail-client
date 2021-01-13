#pragma once
#include <string>
#include <cstring>	//for size_t


namespace email {
	
	class Address {
	protected:
		std::string mail;
		std::string name;
		size_t bytes;

	public:
		Address();
		Address(const std::string& mail);
		Address(const std::string& mail, const std::string& name);		

		void Set(const std::string& mail);
		void Set(const std::string& mail, const std::string& name);

		const std::string& GetMail(void) const;
		const std::string& GetName(void) const;
		const std::string GetFull(void) const;
		size_t GetBytes(void) const;
		bool Empty(void) const;

		void Build(const std::string& s);

	protected:
		void Reparse(void);		

	};

}


