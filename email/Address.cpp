#include "Address.h"


namespace email {

	Address::Address(const std::string& mail): mail(mail) {
		bytes = 1 + mail.size() + 1;
	}

	Address::Address(const std::string& mail, const std::string& name): mail(mail), name(name) {
		bytes = 1 + mail.size() + 1 + 2 + name.size() + 1;
	}

	Address::Address(): bytes(0) {}


	void Address::Set(const std::string& mail) {
		this->mail = mail;
		bytes = 1 + mail.size() + 1;
	}

	void Address::Set(const std::string& mail, const std::string& name) {
		this->mail = mail;
		this->name = name;
		bytes = 1 + mail.size() + 1 + 2 + name.size() + 1;
	}


	const std::string& Address::GetMail(void) const {
		return mail;
	}

	const std::string& Address::GetName(void) const {
		return name;
	}

	const std::string Address::GetFull(void) const {
		std::string full;
		/*full = ("<" + mail + ">");
		if (!name.empty()) {
			full += (" (" + name + ")");
		}*/

		if (!name.empty()) {
			full = ("\"" + name + "\" ");
		}
		full += ("<" + mail + ">");

		return full;
	}


	size_t Address::GetBytes(void) const {
		return bytes;
	}


	bool Address::Empty(void) const {
		if (mail.empty() && name.empty()) {
			return true;
		}
		else {
			return false;
		}
	}


	void Address::Build(const std::string& s) {
		mail = s;
		//TODO pack this into a coroutine?
		Reparse();
	}


	void Address::Reparse(void) {		
		//postprocessing
		size_t bracket1 = mail.find_first_of('<');
		size_t bracket2 = mail.find_first_of('>', bracket1);
		if (bracket1 != std::string::npos && bracket2 != std::string::npos) {
			// it has brackets => fromName may be available
			// => check all Variants

			// Variant1: <madenjudas@gay.de> (Judas Made) or swapped
			size_t name1 = mail.find_first_of('(');
			size_t name2 = mail.find_first_of(')', name1);
			if (name2 != std::string::npos && (name1 > bracket2 || name2 < bracket1)) {
				name = mail.substr(name1 + 1U, name2 - (name1 + 1U));
			}
			else {
				// Variant2: "Judas Made" <madenjudas@gay.de> or swapped
				name1 = mail.find_first_of('"');
				name2 = mail.find_first_of('"', name1 + 1U);
				if (name1 != std::string::npos && name2 != std::string::npos && (name2 < bracket1 || name1 > bracket2)) {
					name = mail.substr(name1 + 1U, name2 - (name1 + 1U));
				}
				else {
					// Variant3: Judas Made <madenjudas@gay.de>
					name1 = mail.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
					name2 = mail.find_last_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", bracket1);
					if (name2 != std::string::npos && name1 < name2) {
						name = mail.substr(name1, name2 + 1U - name1);
					}
				}
			}
			
			mail = mail.substr(bracket1 + 1U, bracket2 - (bracket1 + 1U));			
		}

		// calc bytes: either accordingly for mail & name each
		// or if not even brackets found, take mail length
		bytes = 1 + mail.size() + 1;
		if (!name.empty()) {
			bytes += (2 + name.size() + 1);
		}

	}

	
}