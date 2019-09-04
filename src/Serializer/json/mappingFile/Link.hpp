#pragma once

#include <vector>
#include <WString.h>

namespace ser{

class Link{
	std::vector<String> tokenizedHref;

	/*std::vector<String> ssplit2(const char *str, char c = '/')
	{
		std::vector<String> result;
		String string(str);
		const char* start = str;
		do
		{
			const char *begin = str;
			while(*str != c && *str)
				str++;
			result.push_back(string.substring(begin - start, str - start));
		} while (0 != *str++);

		return result;
	}*/
	std::vector<String> ssplit2(const char *str, char c = '/')
	{
		std::vector<String> result;
		do
		{
			const char *begin = str;
			while(*str != c && *str)
				str++;
			result.push_back(String(begin, str));
		} while (0 != *str++);

		return result;
	}

public:
	String rel;
	String href;

	Link(const String& rel, const String& href): rel(rel), href(href){}
	Link() {}

	const std::vector<String>& getHrefTokens(){//TODO: 111 After finishing the rest of the service: this code is suppose to do the same as the uri parser in the HttpServletRequest, extract that code to some lib
		if(tokenizedHref.size() > 0)//already tokenized
			return tokenizedHref;
		tokenizedHref = ssplit2(href.c_str(), '/');
		return tokenizedHref;
	}

	template <class Archive>
	void save(Archive& archive) const{
		archive.addProperties(MACRO_NVP(rel), MACRO_NVP(href));
	}

	template <class Archive>
	void load(Archive& archive) {
		archive.loadProperties(MACRO_NVP(rel), MACRO_NVP(href));
	}
};
}