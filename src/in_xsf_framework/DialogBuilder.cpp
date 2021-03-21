/*
 * Windows Dynamic Dialog Builder framework
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 */

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <cstdint>
#include "windowsh_wrapper.h"
#include "DialogBuilder.h"
#include "XSFCommon.h"

// Code modified from the following answer on Stack Overflow:
// http://stackoverflow.com/a/3407254
static inline std::uint32_t getNextMultipleOf4(std::uint32_t origNum)
{
	std::uint32_t remainder = origNum % 4;
	if (!remainder)
		return origNum;
	return origNum + 4 - remainder;
}

Point<short> RelativePosition::CalculatePosition(const Rect<short> &child, const Rect<short> &other)
{
	Point<short> newPosition = child.position;
	if (this->relativePosition.y != -1)
	{
		if (this->positionType == PositionType::FromTop || this->positionType == PositionType::FromTopLeft || this->positionType == PositionType::FromTopRight)
			newPosition.y = other.position.y + this->relativePosition.y;
		if (this->positionType == PositionType::FromBottom || this->positionType == PositionType::FromBottomLeft || this->positionType == PositionType::FromBottomRight)
			newPosition.y = other.position.y + other.size.height + this->relativePosition.y;
	}
	if (this->relativePosition.x != -1)
	{
		if (this->positionType == PositionType::FromLeft || this->positionType == PositionType::FromTopLeft || this->positionType == PositionType::FromBottomLeft)
			newPosition.x = other.position.x + this->relativePosition.x;
		if (this->positionType == PositionType::FromRight || this->positionType == PositionType::FromTopRight || this->positionType == PositionType::FromBottomRight)
			newPosition.x = other.position.x + other.size.width + this->relativePosition.x;
	}
	return newPosition;
}

void DialogTemplate::DialogGroup::CalculatePositions(bool doRightAndBottom)
{
	short x = 0, num = this->controls.empty() ? 0 : static_cast<short>(this->controls.size());
	for (; x < num; ++x)
	{
		auto &control = this->controls[x];
		if (control->relativePosition)
		{
			bool valid = true;
			if (!doRightAndBottom && control->relativePosition->type == RelativePosition::BaseType::ToParent)
			{
				switch (control->relativePosition->positionType)
				{
					case RelativePosition::PositionType::FromBottom:
					case RelativePosition::PositionType::FromBottomLeft:
					case RelativePosition::PositionType::FromBottomRight:
					case RelativePosition::PositionType::FromRight:
					case RelativePosition::PositionType::FromTopRight:
						valid = false;
						break;
					default:
						break;
				}
			}
			if (valid)
			{
				Rect<short> other = this->rect;
				if (control->relativePosition->type == RelativePosition::BaseType::ToSibling)
				{
					short siblingsBack = dynamic_cast<const RelativePositionToSibling *>(control->relativePosition.get())->siblingsBack;
					if (x - siblingsBack >= 0)
						other = this->controls[x - siblingsBack]->rect;
				}
				control->rect.position = control->relativePosition->CalculatePosition(control->rect, other);
			}
		}
	}
}

void DialogTemplate::DialogGroup::CalculateSize()
{
	short maxX = 0, maxY = 0, maxOtherWidth = 0, maxOtherHeight = 0;
	std::for_each(this->controls.begin(), this->controls.end(), [&](const std::unique_ptr<DialogControl> &control)
	{
		bool usePosition = true;
		if (control->relativePosition)
		{
			if (control->relativePosition->type == RelativePosition::BaseType::ToParent)
			{
				switch (control->relativePosition->positionType)
				{
					case RelativePosition::PositionType::FromBottom:
					case RelativePosition::PositionType::FromBottomLeft:
					case RelativePosition::PositionType::FromBottomRight:
					case RelativePosition::PositionType::FromRight:
					case RelativePosition::PositionType::FromTopRight:
						usePosition = false;
						/*if (control->relativePosition->relativePosition.x != -1 && control->rect.size.width + control->relativePosition->relativePosition.x > maxOtherWidth)
							maxOtherWidth = control->rect.size.width + control->relativePosition->relativePosition.x;
						if (control->relativePosition->relativePosition.y != -1 && control->GetControlHeight() + control->relativePosition->relativePosition.y > maxOtherHeight)
							maxOtherHeight = control->GetControlHeight() + control->relativePosition->relativePosition.y;*/
						break;
					default:
						break;
				}
			}
		}
		if (usePosition)
		{
			if (control->rect.position.x + control->rect.size.width > maxX)
				maxX = control->rect.position.x + control->rect.size.width;
			if (control->rect.position.y + control->GetControlHeight() > maxY)
				maxY = control->rect.position.y + control->GetControlHeight();
		}
	});
	this->rect.size.width = (maxX - this->rect.position.x) + maxOtherWidth + 4;
	this->rect.size.height = (maxY - this->rect.position.y) + maxOtherHeight + 7;
}

std::uint16_t DialogTemplate::DialogGroup::GetControlCount() const
{
	std::uint16_t count = 1;
	std::for_each(this->controls.begin(), this->controls.end(), [&](const std::unique_ptr<DialogControl> &control) { count += control->GetControlCount(); });
	return count;
}

std::vector<std::uint8_t> DialogTemplate::DialogGroup::GenerateControlTemplate() const
{
	auto data = std::vector<std::uint8_t>(getNextMultipleOf4(24 + sizeof(wchar_t) * (this->groupName.length() + 1)), 0);

	*reinterpret_cast<std::uint32_t *>(&data[0]) = WS_CHILD | WS_VISIBLE | BS_GROUPBOX | this->style;
	*reinterpret_cast<std::uint32_t *>(&data[4]) = this->exstyle;
	*reinterpret_cast<std::uint16_t *>(&data[8]) = this->rect.position.x;
	*reinterpret_cast<std::uint16_t *>(&data[10]) = this->rect.position.y;
	*reinterpret_cast<std::uint16_t *>(&data[12]) = this->rect.size.width;
	*reinterpret_cast<std::uint16_t *>(&data[14]) = this->rect.size.height;
	*reinterpret_cast<std::uint16_t *>(&data[16]) = static_cast<std::uint16_t>(this->id);
	*reinterpret_cast<std::uint16_t *>(&data[18]) = 0xFFFF;
	*reinterpret_cast<std::uint16_t *>(&data[20]) = 0x0080;
	CopyToString(this->groupName, reinterpret_cast<wchar_t *>(&data[22]));

	std::for_each(this->controls.begin(), this->controls.end(), [&](const std::unique_ptr<DialogControl> &control)
	{
		auto controlData = control->GenerateControlTemplate();
		data.insert(data.end(), controlData.begin(), controlData.end());
	});

	return data;
}

std::vector<std::uint8_t> DialogTemplate::DialogControlWithoutLabel::GenerateControlTemplate() const
{
	auto data = std::vector<std::uint8_t>(getNextMultipleOf4(24 + sizeof(wchar_t)), 0);

	*reinterpret_cast<std::uint32_t *>(&data[0]) = WS_CHILD | WS_VISIBLE | this->style;
	*reinterpret_cast<std::uint32_t *>(&data[4]) = this->exstyle;
	*reinterpret_cast<std::uint16_t *>(&data[8]) = this->rect.position.x;
	*reinterpret_cast<std::uint16_t *>(&data[10]) = this->rect.position.y;
	*reinterpret_cast<std::uint16_t *>(&data[12]) = this->rect.size.width;
	*reinterpret_cast<std::uint16_t *>(&data[14]) = this->rect.size.height;
	*reinterpret_cast<std::uint16_t *>(&data[16]) = static_cast<uint16_t>(this->id);
	*reinterpret_cast<std::uint16_t *>(&data[18]) = 0xFFFF;
	*reinterpret_cast<std::uint16_t *>(&data[20]) = this->type;

	return data;
}

std::vector<std::uint8_t> DialogTemplate::DialogControlWithLabel::GenerateControlTemplate() const
{
	auto data = std::vector<std::uint8_t>(getNextMultipleOf4(24 + sizeof(wchar_t) * (this->label.length() + 1)), 0);

	*reinterpret_cast<std::uint32_t *>(&data[0]) = WS_CHILD | WS_VISIBLE | this->style;
	*reinterpret_cast<std::uint32_t *>(&data[4]) = this->exstyle;
	*reinterpret_cast<std::uint16_t *>(&data[8]) = this->rect.position.x;
	*reinterpret_cast<std::uint16_t *>(&data[10]) = this->rect.position.y;
	*reinterpret_cast<std::uint16_t *>(&data[12]) = this->rect.size.width;
	*reinterpret_cast<std::uint16_t *>(&data[14]) = this->rect.size.height;
	*reinterpret_cast<std::uint16_t *>(&data[16]) = static_cast<std::uint16_t>(this->id);
	*reinterpret_cast<std::uint16_t *>(&data[18]) = 0xFFFF;
	*reinterpret_cast<std::uint16_t *>(&data[20]) = this->type;
	CopyToString(this->label, reinterpret_cast<wchar_t *>(&data[22]));

	return data;
}

std::uint16_t DialogTemplate::GetTotalControlCount() const
{
	std::uint16_t count = 0;
	std::for_each(this->controls.begin(), this->controls.end(), [&](const std::unique_ptr<DialogControl> &control) { count += control->GetControlCount(); });
	return count;
}

void DialogTemplate::AddGroupControl(const DialogControlBuilder<DialogGroupBuilder> &builder)
{
	auto newGroup = DialogGroup::CreateControl(builder);
	if (builder.index == -1)
		this->controls.push_back(std::move(newGroup));
	else
		this->controls.insert(this->controls.begin() + builder.index, std::move(newGroup));
}

void DialogTemplate::AddEditBoxControl(const DialogControlBuilder<DialogEditBoxBuilder> &builder)
{
	this->AddControlToGroup(DialogEditBox::CreateControl(builder), builder);
}

void DialogTemplate::AddLabelControl(const DialogControlBuilder<DialogLabelBuilder> &builder)
{
	this->AddControlToGroup(DialogLabel::CreateControl(builder), builder);
}

void DialogTemplate::AddCheckBoxControl(const DialogControlBuilder<DialogCheckBoxBuilder> &builder)
{
	this->AddControlToGroup(DialogButton::CreateControl(builder), builder);
}

void DialogTemplate::AddButtonControl(const DialogControlBuilder<DialogButtonBuilder> &builder)
{
	this->AddControlToGroup(DialogButton::CreateControl(builder), builder);
}

void DialogTemplate::AddListBoxControl(const DialogControlBuilder<DialogListBoxBuilder> &builder)
{
	this->AddControlToGroup(DialogListBox::CreateControl(builder), builder);
}

void DialogTemplate::AddComboBoxControl(const DialogControlBuilder<DialogComboBoxBuilder> &builder)
{
	this->AddControlToGroup(DialogComboBox::CreateControl(builder), builder);
}

bool DialogTemplate::CalculateControlPosition(short index, bool doRightAndBottom)
{
	auto &control = this->controls[index];
	bool valid = true;
	if (control->relativePosition)
	{
		if (!doRightAndBottom && control->relativePosition->type == RelativePosition::BaseType::ToParent)
		{
			switch (control->relativePosition->positionType)
			{
				case RelativePosition::PositionType::FromBottom:
				case RelativePosition::PositionType::FromBottomLeft:
				case RelativePosition::PositionType::FromBottomRight:
				case RelativePosition::PositionType::FromRight:
				case RelativePosition::PositionType::FromTopRight:
					valid = false;
					break;
				default:
					break;
			}
		}
		if (valid)
		{
			Rect<short> other = Rect<short>(Point<short>(), this->size);
			if (control->relativePosition->type == RelativePosition::BaseType::ToSibling)
			{
				short siblingsBack = dynamic_cast<const RelativePositionToSibling *>(control->relativePosition.get())->siblingsBack;
				if (index - siblingsBack >= 0)
					other = this->controls[index - siblingsBack]->rect;
			}
			control->rect.position = control->relativePosition->CalculatePosition(control->rect, other);
		}
	}
	return valid;
}

void DialogTemplate::CalculateSize()
{
	short maxX = 0, maxY = 0, maxOtherWidth = 0, maxOtherHeight = 0;
	std::for_each(this->controls.begin(), this->controls.end(), [&](const std::unique_ptr<DialogControl> &control)
	{
		bool usePosition = true;
		if (control->relativePosition)
		{
			if (control->relativePosition->type == RelativePosition::BaseType::ToParent)
			{
				switch (control->relativePosition->positionType)
				{
					case RelativePosition::PositionType::FromBottom:
					case RelativePosition::PositionType::FromBottomLeft:
					case RelativePosition::PositionType::FromBottomRight:
					case RelativePosition::PositionType::FromRight:
					case RelativePosition::PositionType::FromTopRight:
						usePosition = false;
						/*if (control->relativePosition->relativePosition.x != -1 && control->rect.size.width + control->relativePosition->relativePosition.x > maxOtherWidth)
							maxOtherWidth = control->rect.size.width + control->relativePosition->relativePosition.x;
						if (control->relativePosition->relativePosition.y != -1 && control->GetControlHeight() + control->relativePosition->relativePosition.y > maxOtherHeight)
							maxOtherHeight = control->GetControlHeight() + control->relativePosition->relativePosition.y;*/
						break;
					default:
						break;
				}
			}
		}
		if (usePosition)
		{
			if (control->rect.position.x + control->rect.size.width > maxX)
				maxX = control->rect.position.x + control->rect.size.width;
			if (control->rect.position.y + control->GetControlHeight() > maxY)
				maxY = control->rect.position.y + control->GetControlHeight();
		}
	});
	this->size.width = maxX + maxOtherWidth + 7;
	this->size.height = maxY + maxOtherHeight + 7;
}

void DialogTemplate::AutoSize()
{
	// Step 1: Calculate positions of dialog controls (only if the controls's position is relative to a sibling or if it's relativeness to the parent is from the top, left, or top left)
	short x = 0, num = this->controls.empty() ? 0 : static_cast<short>(this->controls.size()), maxGroupWidth = 0;
	for (; x < num; ++x)
	{
		auto &control = this->controls[x];
		bool valid = this->CalculateControlPosition(x, false);
		if (valid && control->controlType == DialogControlType::Group)
		{
			dynamic_cast<DialogGroup *>(control.get())->CalculatePositions(false);
			// Technically step 2, but calculate the size of the group
			dynamic_cast<DialogGroup *>(control.get())->CalculateSize();
			if (control->rect.size.width > maxGroupWidth)
				maxGroupWidth = control->rect.size.width;
		}
	}
	// Step 2: Resize all group controls to be the same width, and then within the group, recalculate positions for all controls
	for (x = 0; x < num; ++x)
	{
		auto &control = this->controls[x];
		if (control->controlType != DialogControlType::Group)
			continue;
		control->rect.size.width = maxGroupWidth;
		dynamic_cast<DialogGroup *>(control.get())->CalculatePositions(true);
	}
	// Step 3: Resize the dialog box itself
	this->CalculateSize();
	// Step 4: Calculate position of all dialog controls
	for (x = 0; x < num; ++x)
		this->CalculateControlPosition(x, true);
}

const DLGTEMPLATE *DialogTemplate::GenerateTemplate()
{
	this->templateData.clear();
	std::uint16_t controlCount = this->GetTotalControlCount();
	this->templateData.resize(getNextMultipleOf4(24 + sizeof(wchar_t) * (this->title.length() + 1 + (this->fontName.empty() ? 0 : this->fontName.length() + 1))), 0);

	*reinterpret_cast<std::uint32_t *>(&this->templateData[0]) = this->style | (this->fontName.empty() ? 0 : DS_SETFONT);
	*reinterpret_cast<std::uint32_t *>(&this->templateData[4]) = this->exstyle;
	*reinterpret_cast<std::uint16_t *>(&this->templateData[8]) = controlCount;
	*reinterpret_cast<std::uint16_t *>(&this->templateData[14]) = this->size.width;
	*reinterpret_cast<std::uint16_t *>(&this->templateData[16]) = this->size.height;
	CopyToString(this->title, reinterpret_cast<wchar_t *>(&this->templateData[22]));
	if (!this->fontName.empty())
	{
		*reinterpret_cast<std::uint16_t *>(&this->templateData[22 + sizeof(wchar_t) * (this->title.length() + 1)]) = this->fontSizeInPts;
		CopyToString(this->fontName, reinterpret_cast<wchar_t *>(&this->templateData[24 + sizeof(wchar_t) * (this->title.length() + 1)]));
	}

	std::for_each(this->controls.begin(), this->controls.end(), [&](const std::unique_ptr<DialogControl> &control)
	{
		auto controlData = control->GenerateControlTemplate();
		this->templateData.insert(this->templateData.end(), controlData.begin(), controlData.end());
	});

	return reinterpret_cast<const DLGTEMPLATE *>(&this->templateData[0]);
}
