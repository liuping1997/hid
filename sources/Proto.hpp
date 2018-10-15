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

private:
	static inline unordered_map<string, Item> mItems;

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
		if (mItems.find(key) != mItems.end())
			item = mItems[key];
		return std::move(item);
	}

	inline Item&& value(const string& key)
	{
		Item item{};
		if (mItems.find(key) != mItems.end())
			item = mItems[key];
		return std::move(item);
	}

	static void load(const vector<Item>& items)
	{
		mItems.clear();
		for (auto item : items)
		{
			mItems[string(item.name)] = item;
			mItems[to_string(item.id)] = item;
		}
	}
};

