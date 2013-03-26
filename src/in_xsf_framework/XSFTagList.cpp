/*
 * xSF Tag List
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-21
 */

#include <algorithm>
#include "XSFTagList.h"

eq_str XSFTagNameCompare::eqstr;

bool XSFTagNameCompare::operator()(const std::pair<std::string, std::string> &v) const
{
	return XSFTagNameCompare::eqstr(v.first, this->name);
}

std::vector<std::string> XSFTagList::GetAllKeys() const
{
	std::vector<std::string> keys;
	for (XSFTagList::Tags::const_iterator curr = this->tags.begin(), end = this->tags.end(); curr != end; ++curr)
		keys.push_back(curr->first);
	return keys;
}

bool XSFTagList::Exists(const std::string &name) const
{
	return std::find_if(this->tags.begin(), this->tags.end(), XSFTagNameCompare(name)) != this->tags.end();
}

std::string XSFTagList::operator[](const std::string &name) const
{
	XSFTagList::Tags::const_iterator tag = std::find_if(this->tags.begin(), this->tags.end(), XSFTagNameCompare(name));
	if (tag == this->tags.end())
		return "";
	return tag->second;
}

std::string &XSFTagList::operator[](const std::string &name)
{
	XSFTagList::Tags::iterator tag = std::find_if(this->tags.begin(), this->tags.end(), XSFTagNameCompare(name));
	if (tag == this->tags.end())
	{
		this->tags.push_back(std::make_pair(name, ""));
		return this->tags[this->tags.size() - 1].second;
	}
	return tag->second;
}

void XSFTagList::Remove(const std::string &name)
{
	XSFTagList::Tags::iterator tag = std::find_if(this->tags.begin(), this->tags.end(), XSFTagNameCompare(name));
	if (tag != this->tags.end())
		this->tags.erase(tag);
}

void XSFTagList::Clear()
{
	this->tags.clear();
}
