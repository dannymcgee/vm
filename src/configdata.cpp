#include "configdata.h"
#include "value.h"
#include <sstream>

std::shared_ptr<sqf::value> sqf::configdata::inherited_parent_unsafe() const 
{
	std::weak_ptr<configdata> lparent = m_logical_parent;
	while (!lparent.expired())
	{
		// aquire parent
		auto lockparent = lparent.lock();
		// try to find parent
		auto res = lockparent->navigate_unsafe(m_inherited_parent_name);
		// check result
		if (res && res->data<configdata>().get() != this)
		{ // hit, return parent
			return res;
		}
		// set lparent for next round
		lparent = lockparent->m_logical_parent;
	}
	return std::shared_ptr<sqf::value>();
}

std::shared_ptr<sqf::value> sqf::configdata::navigate_unsafe(std::string_view nextnode) const
{
	for (auto it : innervector())
	{
		if (it->dtype() != type::CONFIG)
			continue;
		auto cd = it->data<sqf::configdata>();
		if (str_cmpi(cd->m_name.c_str(), -1, nextnode.data(), -1) == 0)
			return it;
	}
	return std::shared_ptr<sqf::value>();
}

std::shared_ptr<sqf::value> sqf::configdata::navigate_full_unsafe(std::string_view nextnode) const
{
	auto it = navigate_unsafe(nextnode);
	if (it)
	{
		return it;
	}
	else
	{
		std::shared_ptr<sqf::value> p;
		while ((p = inherited_parent_unsafe()).get())
		{
			it = p->data<configdata>()->navigate_full_unsafe(nextnode);
			if (it)
				return it;
		}
		return std::shared_ptr<sqf::value>();
	}
}

std::string sqf::configdata::tosqf() const
{
	std::stringstream sstream;
	for (size_t i = count_logical_parents(); i != (size_t)~0; i--)
	{
		auto node = std::shared_ptr<configdata>(const_cast<configdata*>(this), [](configdata*) {});
		for (size_t j = 0; j < i; j++)
		{
			node = node->m_logical_parent.lock();
		}
		sstream << node->m_name;
		if (i != 0)
		{
			sstream << '/';
		}
	}
	return sstream.str();
}

bool sqf::configdata::is_kind_of(std::string_view s) const
{
	if (str_cmpi(m_name.c_str(), -1, s.data(), -1) == 0)
	{
		return true;
	}
	auto node = this->inherited_parent()->data<configdata>();
	while (!node->is_null())
	{
		if (str_cmpi(node->name().c_str(), -1, s.data(), -1) == 0)
		{
			return true;
		}
		node = node->inherited_parent()->data<configdata>();
	}
	return false;
}

std::shared_ptr<sqf::value> sqf::configdata::logical_parent() const
{
	return m_logical_parent.expired() ? configNull() : std::make_shared<sqf::value>(m_logical_parent.lock(), type::CONFIG);
}

bool sqf::configdata::cfgvalue(std::string_view key, bool def) const
{
	auto node = navigate(key)->data<configdata>();
	if (node->is_null() || (node->cfgvalue()->dtype() != type::BOOL && node->cfgvalue()->dtype() != type::SCALAR))
	{
		return def;
	}
	return node->cfgvalue()->as_bool();
}
float sqf::configdata::cfgvalue(std::string_view key, float def) const
{
	auto node = navigate(key)->data<configdata>();
	if (node->is_null() || node->cfgvalue()->dtype() != type::SCALAR)
	{
		return def;
	}
	return node->cfgvalue()->as_float();
}
std::string sqf::configdata::cfgvalue(std::string_view key, std::string def) const
{
	auto node = navigate(key)->data<configdata>();
	if (node->is_null() || node->cfgvalue()->dtype() != type::SCALAR)
	{
		return def;
	}
	return node->cfgvalue()->as_string();
}

void sqf::configdata::mergeinto(std::shared_ptr<configdata> cd)
{
	for (auto& val : innervector())
	{
		if (val->dtype() != sqf::type::CONFIG)
			continue;
		auto subcd = val->data<configdata>();
		auto othercd = cd->navigate_unsafe(subcd->m_name);
		if (othercd)
		{
			subcd->mergeinto(othercd->data<configdata>());
		}
		else
		{
			cd->push_back(val);
		}
	}
}

std::shared_ptr<sqf::value> sqf::configdata::configFile()
{
	static std::shared_ptr<sqf::configdata> cdata = std::make_shared<sqf::configdata>();
	return std::make_shared<sqf::value>(cdata, sqf::type::CONFIG);
}

std::shared_ptr<sqf::value> sqf::configdata::configNull()
{
	static std::shared_ptr<sqf::configdata> cdata = std::make_shared<sqf::configdata>("");
	return std::make_shared<sqf::value>(cdata, sqf::type::CONFIG);
}
