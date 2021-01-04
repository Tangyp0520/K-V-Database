#include "MinHeap.h"
MinHeap::MinHeap()
{
	MinHeap_filename = "MinHeap_file.txt";
	const string filename = MinHeap_filename;
	fstream file;
	file.open(filename, ios::binary | ios::in);
	if (!file)
	{
		ofstream newfile(filename);
		if (newfile)
			newfile.close();
	}
	file.close();
	reset();
}
MinHeap::~MinHeap()
{
	MinHeap_filename.clear();
	index.clear();
	while (!minHeap.empty())
	{
		minHeap.pop();
	}
}
void MinHeap::reset()
{
	index.clear();
	while (!minHeap.empty())
	{
		minHeap.pop();
	}

	const string filename = MinHeap_filename;
	fstream file;
	file.open(filename, ios::binary | ios::in);
	file.seekg(0, ios::end);
	int length = file.tellg();
	file.seekg(0, ios::beg);

	time_t cur = time(NULL);
	while (length != 0)
	{
		TimeNode s;
		int keyLen;
		file.read((char*)&keyLen, sizeof(int));
		file.read((char*)&s.time, sizeof(int));
		char* key_buf = new char[keyLen];
		file.read(key_buf, keyLen);
		for (int i = 0; i < keyLen; i++)
			s.key.push_back(key_buf[i]);

		if (s.time >= cur)
		{
			minHeap.push(s);
			index.set(s.key, KEY_EXIST_IN_MINHEAP);
		}

		length -= sizeof(int) * 2 + keyLen;
		delete[]key_buf;
	}
	file.close();
}
void MinHeap::set(string key, int time)
{
	const string filename = MinHeap_filename;
	fstream file;
	file.open(filename, ios::binary | ios::app);
	int keyLen = key.length();
	file.write((char*)&keyLen, sizeof(int));
	file.write((char*)&time, sizeof(int));
	file.write(key.c_str(), key.length());
	file.close();

	TimeNode s;
	s.key = key; s.time = time;
	minHeap.push(s);

	int offset = KEY_EXIST_IN_MINHEAP;
	if (time == KEY_NOT_EXIST_IN_MINHEAP)
		offset = KEY_NOT_EXIST_IN_MINHEAP;
	index.set(key, offset);
}

bool MinHeap::get(string key)
{
	int re = index.get(key);
	if (re == KEY_NOT_EXIST || re == KEY_NOT_EXIST_IN_MINHEAP)
		return true;
	bool flag = false;
	priority_queue<TimeNode, vector<TimeNode>, cmp> mid;
	while (!minHeap.empty())
	{
		if (minHeap.top().key == key)
		{
			flag = true;
		}
		mid.push(minHeap.top());
		minHeap.pop();
	}
	while (!mid.empty())
	{
		minHeap.push(mid.top());
		mid.pop();
	}
	return flag;
}

void MinHeap::del()
{
	time_t cur = time(NULL);
	while (1)
	{
		if (minHeap.empty())
			break;
		if (minHeap.top().time < cur)
			minHeap.pop();
		else break;
	}
}