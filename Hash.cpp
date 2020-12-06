#include "Hash.h"
Hash::~Hash()
{
	for (int i = 0; i <= HASHMAX - 1; i++)
		while (!root[i].empty())
			root[i].pop_back();
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
	DataNode s;
	s.key = key; s.offset = offset;
	root[hashcode].push_back(s);
}
int Hash::get(string key)
{
	int hashcode = GetHashCode(key);
	list<DataNode> s = root[hashcode];
	list<DataNode>::iterator it = s.begin();
	int offset = -1;

	for (; it != s.end(); it++)
	{
		DataNode data = *it;
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