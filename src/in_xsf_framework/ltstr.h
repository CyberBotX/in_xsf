/*
 * Case-insensitive string comparison
 * Last modification on 2012-03-30
 *
 * Based on "How to do case-insensitive string comparison"
 * By Matt Austern
 * http://lafstern.org/matt/col2_new.pdf
 */

#ifndef LTSTR_H
#define LTSTR_H

#include <functional>
#include <locale>
#include <string>
#include <algorithm>
#include <climits>

struct lt_str : std::binary_function<std::string, std::string, bool>
{
	struct lt_char : std::binary_function<char, char, bool>
	{
		const char *tab;
		lt_char(const char *t) : tab(t) { }
		bool operator()(char x, char y) const { return this->tab[x - CHAR_MIN] < this->tab[y - CHAR_MIN]; }
	};

	char tab[CHAR_MAX - CHAR_MIN + 1];

	lt_str(const std::locale &L = std::locale::classic())
	{
		for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
			this->tab[i - CHAR_MIN] = static_cast<char>(i);
		std::use_facet<std::ctype<char>>(L).toupper(this->tab, this->tab + (CHAR_MAX - CHAR_MIN + 1));
	}

	bool operator()(const std::string &x, const std::string &y) const
	{
		return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end(), lt_char(this->tab));
	}
};

#endif
