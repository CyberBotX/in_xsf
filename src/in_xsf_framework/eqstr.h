/*
 * Case-insensitive string equality
 *
 * Based on "How to do case-insensitive string comparison"
 * By Matt Austern
 * http://lafstern.org/matt/col2_new.pdf
 */

#pragma once

#include <algorithm>
#include <limits>
#include <locale>
#include <string>

struct eq_str
{
	struct eq_char
	{
		const char *tab;
		eq_char(const char *t) : tab(t) { }
		bool operator()(char x, char y) const { return this->tab[x - std::numeric_limits<char>::min()] == this->tab[y - std::numeric_limits<char>::min()]; }
	};

	char tab[std::numeric_limits<char>::max() - std::numeric_limits<char>::min() + 1];

	eq_str(const std::locale &L = std::locale::classic())
	{
		for (int i = std::numeric_limits<char>::min(); i <= std::numeric_limits<char>::max(); ++i)
			this->tab[i - std::numeric_limits<char>::min()] = static_cast<char>(i);
		std::use_facet<std::ctype<char>>(L).toupper(this->tab, this->tab + (std::numeric_limits<char>::max() - std::numeric_limits<char>::min() + 1));
	}

	bool operator()(const std::string &x, const std::string &y) const
	{
		return x.length() == y.length() && std::equal(x.begin(), x.end(), y.begin(), eq_char(this->tab));
	}
};
