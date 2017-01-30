#pragma once
template<class T>
inline void escape(T& str, char separator)
{
	int i = 0;
	auto v = str.begin();
	while (i!=str.size())
	{
		if (*v == '\"')
		{
			str.insert(i, 1, '\\');
			++i;
		}
		else if (*v == '\\')
		{
			str.insert(i, 1, '\\');
			++i;
		}
		else if (*v == separator)
		{
			str.insert(i, 1, '\\');
			++i;
		}
		++i;
		v = str.begin() + i;
	}
}
inline std::string replace(std::string subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

inline void tolower(std::string& chars)
{
	for (auto& thechar : chars)
	{
		thechar = static_cast<char>(tolower(thechar));
	}
}