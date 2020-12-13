#include "KVDBHandler.h"

KVDBHandler::KVDBHandler(const string& db_file)
{
	offset = 0;
	KVDB = db_file;
	const string filename = KVDB;
	fstream myfile;
	myfile.open(filename, ios::binary | ios::out | ios::app);
	if (!myfile)
	{
		ofstream newfile(filename);
		if (newfile)
			newfile.close();
	}
	myfile.close();

	resetIndex();
}

KVDBHandler::~KVDBHandler()
{
	KVDB.clear();
	index.clear();
	offset = 0;
}

long long KVDBHandler::getLength()
{
	const string filename = KVDB;
	fstream file;
	file.open(filename, ios::in | ios::binary);
	file.seekg(0, ios::end);
	long long length = file.tellg();
	file.seekg(0, ios::beg);
	return length;
}

void KVDBHandler::readKVDBData(fstream& file, KVDBData& s)
{
	int keyLen, valueLen, del, time;
	file.read((char*)&keyLen, sizeof(int));
	file.read((char*)&valueLen, sizeof(int));
	file.read((char*)&del, sizeof(int));
	file.read((char*)&time, sizeof(int));
	s.keyLen = keyLen; s.valueLen = valueLen;
	s.del = del; s.time = time;
	
	string key, value;
	char* key_buf = new char[s.keyLen];
	char* value_buf = new char[s.valueLen];
	file.read(key_buf, s.keyLen);
	file.read(value_buf, s.valueLen);
	for (int i = 0; i <= s.keyLen - 1; i++)
		key.push_back(key_buf[i]);
	for (int i = 0; i <= s.valueLen - 1; i++)
		value.push_back(value_buf[i]);

	s.key = key; s.value = value;
	delete[]key_buf; 
	delete[]value_buf;
}

void KVDBHandler::writeKVDBData(fstream& file, KVDBData& s)
{
	int keyLen = s.keyLen, valueLen = s.valueLen;
	string key = s.key, value = s.value;
	int del = s.del, time = s.time;

	file.write((char*)&keyLen, sizeof(int));
	file.write((char*)&valueLen, sizeof(int));
	file.write((char*)&del, sizeof(int));
	file.write((char*)&time, sizeof(int));

	file.write(key.c_str(), keyLen);
	file.write(value.c_str(), valueLen);
}

void KVDBHandler::resetIndex()
{
	index.clear();
	offset = 0;

	const string filename = KVDB;
	fstream file;
	file.open(filename, ios::in | ios::binary);
	file.seekg(0, ios::end);
	long long x = file.tellg();
	file.seekg(0, ios::beg);

	int length = x;
	while (length != 0)
	{
		KVDBData s;
		readKVDBData(file, s);

		length -= sizeof(int) * 4 + s.keyLen + s.valueLen;

		index.set(s.key, offset);
		offset += sizeof(int) * 4 + s.keyLen + s.valueLen;
	}

	file.close();
}

int set(KVDBHandler* handler, const string& key, const string& value)
{
	const string filename = handler->KVDB;
	fstream file;
	file.open(filename, ios::binary | ios::app);
	if (!file)
		return FILE_OPENIBG_FAILED;

	KVDBData s;
	s.keyLen = key.length(); s.valueLen = value.length();
	s.key = key; s.value = value;
	s.time = 0; s.del = 0;
	handler->writeKVDBData(file, s);
	handler->index.set(key, handler->offset);
	handler->offset += sizeof(int) * 4 + s.keyLen + s.valueLen;
	file.close();
	return SUCCESS;
}

int get(KVDBHandler* handler, const string& key, string& value)
{
	const string filename = handler->KVDB;
	fstream file;
	file.open(filename, ios::binary | ios::in);
	if (!file)
		return FILE_OPENIBG_FAILED;

	KVDBData s;
	s.keyLen = key.length(); s.valueLen = value.length();
	s.key = key; s.value = value;
	s.time = 0; s.del = 0;

	int offset = handler->index.get(key);
	if (offset == -1)
	{
		file.close();
		return KEY_NOT_EXIST;
	}

	file.seekg(offset, ios::beg);
	handler->readKVDBData(file, s);
	value = s.value;
	
	file.close();

	int flag = SUCCESS;
	if (s.del == 1)
		return KEY_HAS_BEEN_DELETED;
	if (s.time > 0)
	{
		time_t cur = time(NULL);
		if (cur < s.time)
			return SUCCESS;
		else return OVERDUE_KEY;
	}
	return SUCCESS;
}
int del(KVDBHandler* handler, const std::string& key)
{
	string value = "KEY_HAS_BEEN_DELETED";

	int flag = get(handler, key, value);
	if (flag != SUCCESS)
		return flag;

	const string filename = handler->KVDB;
	fstream file;
	file.open(filename, ios::binary | ios::app);
	if (!file)
		return FILE_OPENIBG_FAILED;

	KVDBData s;
	s.keyLen = key.length(); s.valueLen = value.length();
	s.key = key; s.value = value;
	s.time = 0; s.del = 1;
	handler->writeKVDBData(file, s);
	handler->index.set(key, handler->offset);
	handler->offset += sizeof(int) * 4 + s.keyLen + s.valueLen;
	file.close();
	return SUCCESS;
}
int purge(KVDBHandler* handler)
{
	KVDBHandler* save_handler;
	save_handler = new KVDBHandler("save_KVDB.txt");
	int flag;

	flag = purgeSubfunction(handler, save_handler);
	if (flag == MEOMORY_ALLOCATION_FAILUER)
	{
		cout << "MEOMORY_ALLOCATION_FAILUER" << endl;
		flag = FAILED;
	}
	if (flag == ERROR_EXECUTING_SUBFUNCTION)
	{
		cout << "ERROR_EXECUTING_SUBFUNCTION" << endl;
		flag = FAILED;
	}

	flag = purgeSubfunction(save_handler, handler);
	if (flag == MEOMORY_ALLOCATION_FAILUER)
	{
		cout << "MEOMORY_ALLOCATION_FAILUER" << endl;
		flag = FAILED;
	}
	if (flag == ERROR_EXECUTING_SUBFUNCTION)
	{
		cout << "ERROR_EXECUTING_SUBFUNCTION" << endl;
		flag = FAILED;
	}

	handler->resetIndex();

	return flag;
}

int purgeSubfunction(KVDBHandler* handler, KVDBHandler* save_handler)
{
	save_handler->index.clear();
	string value, save_value, key;
	Hash s = handler->index;
	for (int i = 0; i <= HASHMAX - 1; i++)
	{
		list<HashDataNode> root = s.root[i];
		while (!root.empty())
		{
			HashDataNode data = root.back();
			root.pop_back();
			key = data.key;
			if (get(handler, key, value) == SUCCESS && get(save_handler, key, save_value) != SUCCESS)
			{
				set(save_handler, key, value);
			}
		}
	}
	handler->index.clear();

	ofstream delete_file(handler->KVDB.c_str());
	if (delete_file.good())
		remove(handler->KVDB.c_str());
	else return ERROR_EXECUTING_SUBFUNCTION;
	return SUCCESS;
}
int expires(KVDBHandler* handler, const string key, int n)
{
	string value = "KEY_HAS_BEEN_DELETED";

	int flag = get(handler, key, value);
	if (flag != SUCCESS)
		return flag;

	const string filename = handler->KVDB;
	fstream file;
	file.open(filename, ios::binary | ios::app);
	if (!file)
		return FILE_OPENIBG_FAILED;

	time_t cur = time(NULL);
	KVDBData s;
	s.keyLen = key.length(); s.valueLen = value.length();
	s.key = key; s.value = value;
	s.time = cur + n; s.del = 0;
	handler->writeKVDBData(file, s);
	handler->index.set(key, handler->offset);
	handler->offset += sizeof(int) * 4 + s.keyLen + s.valueLen;
	file.close();
	return SUCCESS;
}