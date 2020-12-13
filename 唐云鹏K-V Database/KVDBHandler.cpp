#include"KVDBHandler.h"

string char_to_string(char* str, int n)
{
	string ans;
	for (int i = 0; i <= n - 1; i++)
	{
		ans.push_back(str[i]);
	}
	return ans;
}
char* int_to_char(int n)
{
	char* ans = new char[32];
	int i;
	for (i = 0; i <= 31; i++)
		ans[i] = '0';
	for (i = 31; n != 0; i--)
	{
		int x = n % 10;
		n /= 10;
		if (x < 10)
			ans[i] = x + '0';
	}
	return std::move(ans);
}
//int char_to_int(char* ch)
//{
//	int flag = 1;
//	int ans = 0;
//	for (int i = 31; i > 0; i--)
//	{
//		ans += (ch[i] - '0') * flag;
//		flag *= 10;
//	}
//	return ans;
//}
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
void KVDBHandler::resetIndex()
{
	index.clear();
	offset = 0;

	const string filename = KVDB;
	fstream file;
	file.open(filename, ios::in | ios::binary);
	file.seekg(0, ios::end);
	long long length = file.tellg();
	file.seekg(0, ios::beg);

	while (length != 0)
	{
		char* keyLen_buf = NULL;
		keyLen_buf = new char[32];
		file.read(keyLen_buf, 32);
		length -= 32;
		//int keyLen = char_to_int(keyLen_buf);
		int keyLen = atoi(keyLen_buf);
		delete[]keyLen_buf;

		char* valueLen_buf = NULL;
		valueLen_buf = new char[32];
		file.read(valueLen_buf, 32);
		length -= 32;
		//int valueLen = char_to_int(valueLen_buf);
		int valueLen = atoi(valueLen_buf);
		delete[]valueLen_buf;

		char* key_buf = NULL;
		key_buf = new char[keyLen];
		file.read(key_buf, keyLen);
		length -= keyLen;
		string key_string = char_to_string(key_buf, keyLen);
		delete[]key_buf;

		file.seekg(valueLen, ios::cur);
		length -= valueLen;

		index.set(key_string, offset);
		offset += 32 + 32 + keyLen + valueLen;

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

	int keyLen = key.length(), valueLen = value.length();
	int time = 0;

	char* keyLen1 = int_to_char(keyLen);
	char* valueLen1 = int_to_char(valueLen);
	char* time_char = int_to_char(time);

	file.write(keyLen1, 32);
	file.write(valueLen1, 32);
	file.write(key.c_str(), keyLen);
	file.write(value.c_str(), valueLen);
	file.write(time_char, 32);

	handler->index.set(key, handler->offset);
	handler->offset += 32 + 32 + keyLen + valueLen + 32;

	delete[]keyLen1;
	delete[]valueLen1;
	delete[]time_char;
	file.close();
	return SUCCESS;
}
int get(KVDBHandler* handler, const string& key, string& value)
{
	const string filename = handler->KVDB;
	ifstream file;
	file.open(filename, ios::in | ios::binary);
	if (!file)
		return FILE_OPENIBG_FAILED;

	int offset = handler->index.get(key);
	if (offset == -1)
		return KEY_NOT_EXIST;

	int flag = KEY_NOT_EXIST;


	file.seekg(offset, ios::beg);

	char* keyLen_buf = NULL;
	keyLen_buf = new char[32];
	if (keyLen_buf == NULL)
		return MEOMORY_ALLOCATION_FAILUER;
	file.read(keyLen_buf, 32);
	//int keyLen = char_to_int(keyLen_buf);
	int keyLen = atoi(keyLen_buf);
	delete[]keyLen_buf;

	char* valueLen_buf = NULL;
	valueLen_buf = new char[32];
	if (valueLen_buf == NULL)
		return MEOMORY_ALLOCATION_FAILUER;
	file.read(valueLen_buf, 32);
	//int valueLen = char_to_int(valueLen_buf);
	int valueLen = atoi(valueLen_buf);
	delete[]valueLen_buf;

	file.seekg(keyLen, ios::cur);

	if (valueLen > 0)
	{
		char* value_buf = NULL;
		value_buf = new char[valueLen];
		if (value_buf == NULL)
			return MEOMORY_ALLOCATION_FAILUER;
		file.read(value_buf, valueLen);
		value = char_to_string(value_buf, valueLen);
		delete[]value_buf;
		flag = SUCCESS;
	}
	if (valueLen == 0)
		flag = KEY_DELETED;

	char* time_buf = NULL;
	time_buf = new char[32];
	if (time_buf == NULL)
		return MEOMORY_ALLOCATION_FAILUER;
	file.read(time_buf, 32);
	int overdue = atoi(time_buf);
	time_t cur = time(NULL);
	if (overdue == -1 || overdue <= cur && overdue != 0)
		return OVERDUE_KEY;
	
	file.close();
	return flag;
}
int del(KVDBHandler* handler, const std::string& key)
{
	string value;
	int flag = get(handler, key, value);
	if (flag != SUCCESS)
		return flag;
	flag = set(handler, key, "");
	return flag;
}
int purgeSubfunction(KVDBHandler* handler, KVDBHandler* save_handler)
{
	save_handler->index.clear();
	string value, save_value, key;
	Hash s = handler->index;
	for (int i = 0; i <= HASHMAX - 1; i++)
	{
		list<DataNode> root = s.root[i];
		while (!root.empty())
		{
			DataNode data = root.back();
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
int purge(KVDBHandler* handler)
{
	KVDBHandler* save_handler;
	save_handler = new KVDBHandler("save_test.txt");
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
int expires(KVDBHandler* handler, const string key, int n)
{
	string value;
	int flag = get(handler, key, value);
	if (flag != SUCCESS)
		return flag;
	const string filename = handler->KVDB;
	fstream file;
	file.open(filename, ios::binary | ios::app);
	if (!file)
		return FILE_OPENIBG_FAILED;

	int keyLen = key.length(), valueLen = value.length();
	time_t cur = time(NULL);

	char* keyLen1 = int_to_char(keyLen);
	char* valueLen1 = int_to_char(valueLen);
	char* time_char = int_to_char(cur);

	file.write(keyLen1, 32);
	file.write(valueLen1, 32);
	file.write(key.c_str(), keyLen);
	file.write(value.c_str(), valueLen);
	file.write(time_char, 32);

	handler->index.set(key, handler->offset);
	handler->offset += 32 + 32 + keyLen + valueLen + 32;

	delete[]keyLen1;
	delete[]valueLen1;
	delete[]time_char;
	file.close();
	return SUCCESS;
}