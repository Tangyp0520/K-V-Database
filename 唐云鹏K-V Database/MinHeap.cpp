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
	/*file.seekg(0, ios::end);
	int length = file.tellg();
	file.seekg(0, ios::beg);
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
		
		if (s.time != KEY_NOT_EXIST_IN_MINHEAP)
			minHeap.push(s);

		length -= sizeof(int) * 2 + keyLen;
		delete[]key_buf;
	}
	file.close();*/
}
MinHeap::~MinHeap()
{
	MinHeap_filename.clear();
	while (!minHeap.empty())
	{
		minHeap.pop();
	}
}
void MinHeap::reset()
{
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

		if (s.time != KEY_NOT_EXIST_IN_MINHEAP)
			minHeap.push(s);

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
}

int MinHeap::get(string key)
{
	int time = KEY_NOT_EXIST_IN_MINHEAP;
	priority_queue<TimeNode, vector<TimeNode>, cmp> mid;
	while (!minHeap.empty())
	{
		if (minHeap.top().key == key)
		{
			time = minHeap.top().time;
			break;
		}
		mid.push(minHeap.top());
		minHeap.pop();
	}
	while (!mid.empty())
	{
		minHeap.push(mid.top());
		mid.pop();
	}
	return time;
}

void MinHeap::del(string key)
{
	priority_queue<TimeNode, vector<TimeNode>, cmp> mid;
	while (!minHeap.empty())
	{
		if (minHeap.top().key != key)
			mid.push(minHeap.top());

		minHeap.pop();
	}

	while (!mid.empty())
	{
		minHeap.push(mid.top());
		mid.pop();
	}
	set(key, OVERDUE_KEY);
}
