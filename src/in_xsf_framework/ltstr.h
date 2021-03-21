/*
 * Case-insensitive string comparison
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

struct lt_str
{
	struct lt_char
	{
		const char *tab;
		lt_char(const char *t) : tab(t) { }
		bool operator()(char x, char y) const { return this->tab[x - std::numeric_limits<char>::min()] < this->tab[y - std::numeric_limits<char>::min()]; }
	};

	char tab[std::numeric_limits<char>::max() - std::numeric_limits<char>::min() + 1];

	lt_str(const std::locale &L = std::locale::classic())
	{
		for (int i = std::numeric_limits<char>::min(); i <= std::numeric_limits<char>::max(); ++i)
			this->tab[i - std::numeric_limits<char>::min()] = static_cast<char>(i);
		std::use_facet<std::ctype<char>>(L).toupper(this->tab, this->tab + (std::numeric_limits<char>::max() - std::numeric_limits<char>::min() + 1));
	}

	bool operator()(const std::string &x, const std::string &y) const
	{
		return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end(), lt_char(this->tab));
	}
};
