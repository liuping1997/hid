#pragma once 

#include "Logger.hpp"

using namespace std;

class Proto
{
public:
	struct Item
	{
		int id = 0;
		int offset = 0;
		char type[8] = "byte";
		char name[8] = "null";
	};

public:
	static inline unordered_map<string, Item> values;

public:
	Proto()
	{
	}

	static Proto& instance()
	{
		static Proto m_instance;
		return m_instance;
	}

	inline Item&& value(const string&& key)
	{
		Item item{};
		if (values.find(key) != values.end())
			item = values[key];
		return std::move(item);
	}

	inline Item&& value(const string& key)
	{
		Item item{};
		if (values.find(key) != values.end())
			item = values[key];
		return std::move(item);
	}

	static void load(const vector<Item>& in)
	{
		values.clear();
		for (auto item : in)
		{
			values[string(item.name)] = item;
			values[to_string(item.id)] = item;
		}
	}
};

