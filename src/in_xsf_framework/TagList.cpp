/*
 * xSF Tag List
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Storage of tags from PSF-style files, specifications found at
 * http://wiki.neillcorlett.com/PSFTagFormat
 */

#include <algorithm>
#include "TagList.h"

using namespace std::placeholders;

eq_str TagList::eqstr;

auto TagList::GetKeys() const -> const TagsList &
{
	return this->tagsOrder;
}

auto TagList::GetTags() const -> TagsList
{
	TagsList allTags;
	for (auto curr = this->tagsOrder.begin(), end = this->tagsOrder.end(); curr != end; ++curr)
		allTags.push_back(*curr + "=" + this->tags.find(*curr)->second);
	return allTags;
}

bool TagList::Exists(const std::string &name) const
{
	return std::find_if(this->tagsOrder.begin(), this->tagsOrder.end(), std::bind(TagList::eqstr, _1, name)) != this->tagsOrder.end();
}

std::string TagList::operator[](const std::string &name) const
{
	auto tag = this->tags.find(name);
	if (tag == this->tags.end())
		return "";
	return tag->second;
}

std::string &TagList::operator[](const std::string &name)
{
	auto tag = std::find_if(this->tagsOrder.begin(), this->tagsOrder.end(), std::bind(TagList::eqstr, _1, name));
	if (tag == this->tagsOrder.end())
	{
		this->tagsOrder.push_back(name);
		this->tags[name] = "";
	}
	return this->tags[name];
}

void TagList::Remove(const std::string &name)
{
	auto tagOrder = std::find_if(this->tagsOrder.begin(), this->tagsOrder.end(), std::bind(TagList::eqstr, _1, name));
	if (tagOrder != this->tagsOrder.end())
		this->tagsOrder.erase(tagOrder);
	if (this->tags.count(name))
		this->tags.erase(name);
}

void TagList::Clear()
{
	this->tagsOrder.clear();
	this->tags.clear();
}
