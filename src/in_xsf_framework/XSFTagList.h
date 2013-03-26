/*
 * xSF Tag List
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-21
 */

#ifndef XSFTAGLIST_H
#define XSFTAGLIST_H

#include <utility>
#include <vector>
#include "eqstr.h"

struct XSFTagNameCompare
{
	XSFTagNameCompare(const std::string &Name) : name(Name) { }
	bool operator()(const std::pair<std::string, std::string> &v) const;
private:
	static eq_str eqstr;
	std::string name;
};

class XSFTagList
{
	typedef std::vector<std::pair<std::string, std::string> > Tags;

	Tags tags;
public:
	XSFTagList() : tags() { }
	std::vector<std::string> GetAllKeys() const;
	bool Exists(const std::string &name) const;
	std::string operator[](const std::string &name) const;
	std::string &operator[](const std::string &name);
	void Remove(const std::string &name);
	void Clear();
};

#endif
