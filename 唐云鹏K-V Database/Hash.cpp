#include "Hash.h"

Hash::~Hash()
{
	clear();
}

int Hash::GetHashCode(string str)
{
	int hash = 0, n = str.length();
	for (int i = 0; i <= n - 1; i++)
		hash = str[i] + (HASHMAX * hash);
	return hash % HASHMAX;
}

void Hash::set(string key, int offset)
{
	int hashcode = GetHashCode(key);
	bool flag = false;
	list<HashDataNode>::iterator it = root[hashcode].begin();
	for (; it != root[hashcode].end(); it++)
	{
		if (it->key == key)
		{
			it->offset = offset;
			flag = true;
			break;
		}
	}
	if (flag == false)
	{
		HashDataNode s;
		s.key = key; s.offset = offset;
		root[hashcode].push_back(s);
	}
}

int Hash::get(string key)
{
	int hashcode = GetHashCode(key);
	list<HashDataNode> s = root[hashcode];
	list<HashDataNode>::iterator it = s.begin();
	int offset = KEY_NOT_EXIST;

	for (; it != s.end(); it++)
	{
		HashDataNode data = *it;
		if (data.key == key)
		{
			offset = data.offset;
		}
	}
	return offset;
}

void Hash::clear()
{
	for (int i = 0; i <= HASHMAX - 1; i++)
		while (!root[i].empty())
			root[i].pop_back();
}