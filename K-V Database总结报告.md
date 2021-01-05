# File based K-V Database

## 一. 需求

### 1. K-V数据库

基于文件的K-V数据库，以KeyLen、ValueLen、key、value作为一个单位元在文件中储存；key和value为string类型；用户可根据自身需求对key所对应单位元进行设置(set)、查找(get)，删除(del)等基本操作；为更有效实现上述功能引入基于哈希表的内存索引；除基础操作外添加基于最小堆的过期操作(expires)，用户可根据需要对key设置过期时间，key将在过期时删除；为方便调试查错，数据库同时设计有日志打印机制；

### 2. Append-Only File特点

Append-Only File在文件写入内容时为默认为尾加形式；在K-V数据库当中，利用其尾加的性质可认为遍历文件过程中最后得到的key所对应单位元为最新设置情况；K-V数据库各种功能将基于此特点设计；而在此特性下在反复操作set/get/sel/expires功能后，会导致导致文件膨胀而有效数据寥寥无几；为解决此问题设计purge功能可人为控制将文件中已失效单位元删除同时重置索引；



## 二. 设计

### 1. File Base

设计的K-V数据库将以文件为基础；在磁盘中存储并读写；不依靠与程序分配内存能够保证在程序退出后在数据库写入内容不会随着内存被释放而消失；程序分配内存有限，所能存储的数据库内容有限，而在磁盘中进行存储，理论上能够存储的内容无限，符合数据库要求；

### 2. 基础操作(set/get/del)

K-V数据库所实现基本操作有set/get/del；

引入结构体KVDBData；数据成员为(int)keyLen、(int)valueLen、(string)key、(string)value；

set操作将用户输入的key和value转化为结构体KVDBData，以keyLen、valueLen、key、value顺序写入文件并且在索引中写入或更新此key对应单位元在文件中存储的位置，具体表现为单位元距离文件头的偏移量

get操作根据用户输入的key值在索引中查找所对应在文件中位置，定位文件读指针后在数据库文件中查找其所对应单位元并返回value值；若此key所对应单位元已被设置删除或者过期则返回相应结果；

del操作根据用户输入的key值，首先利用get操作在文件中查找，确认key所对应单位元状态是否正常；若正常则在文件中尾加新的key所对应单位元，但将valueLen设置为-1，标志为此单位元已被删除

### 3. Index索引

索引的设计将基于哈希表设计；

引入结构体HashDataNode；数据成员为(string)key、(int)offset；

在数据库中作为索引使用；offset存储单位元位置偏移量；将查找key所对应最新单位元在文件中位置的时间复杂度从o(n)降低到o(1)；

在最小堆中作为历史记录使用；offset存储key进入最小堆的状态；存储历史进入最小堆所有key并更新其状态

### 4. 基于索引的Purge操作

purge操作目的在于删除文件中无效数据，解决文件膨胀问题；

在未引入索引时，设计方法为遍历源文件，将存在状态正常的K-V单位元存入副本文件，再将副本文件存回源文件；

引入索引后，设计方法为遍历索引，根据索引中存在状态正常的K-V单位元存储位置读出，再从文件中读取，存入副本文件，再将副本文件存回源文件

### 5. 基于最小堆的Expires操作

expires操作可对存在状态正常的K-V单位元操作设置其过期时间；在其生存周期达到其过期时间时过期；

引入结构体TimeNode；数据成员为(string)key、(int)time；以此创建STL中最小堆；

使用索引在此时作为历史记录，存储历史进入最小堆所有key并更新其状态

设计方法为最小堆推出其顶部小于当前时间的key设为过期，在查找中若存在于历史记录但不存在最小堆则说明其过期



## 三. 代码设计

### 主函数

定义KVDBHandler类对象作为数据库，定义LOGGER类对象用以打印日志

While(1)循环直到输入命令为EXIT；

循环内输出交互内容，交互内容包括但不限于数据库支持输入类型以及数据库支持的命令列表，在一次循环结束清空控制台内容；

输入命令；若为不支持的命令则输出error command并进入下一循环，若为支持的命令则利用if…else…跳转到相应的代码块，按照相适应要求请求输入并调用相应函数，根据调用函数的返回值输出相应交互语句

调用日志存储相应信息，日志等级为DEBUG

### KVDBHandler类

数据库类对象

数据成员：(string)KVDB：存储K-V数据库文件名； (Hash)index：作为索引快速查找K-V数据库； (MinHeap)minHeap：以最小堆为存储方式，存储设置过期时间的key；(int)offset：当前存储文件末与文件头偏移值

使用结构体KVDBData；同时设置set函数可对所有数据成员设值

```c++
class KVDBHandler
{
public:
	string KVDB;
	Hash index;
	MinHeap minHeap;
	int offset;

	KVDBHandler(const string& db_file);
	~KVDBHandler();
	void resetIndex();
	void readKVDBData(fstream& file, KVDBData& s);
	void writeKVDBData(fstream& file, KVDBData& s);
	friend int set(KVDBHandler* handler, const string& key, const string& value);
	friend int get(KVDBHandler* handler, const string& key, string& value);
	friend int del(KVDBHandler* handler, const std::string& key);
	friend int purge(KVDBHandler* handler);
	friend int purgeSubfunction(KVDBHandler* handler, KVDBHandler* save_handler);
	friend int expires(KVDBHandler* handler, const string key, int n);
};
```

构造函数：offset存储当前文件尾距文件头偏移量，此时offset归零；KVDB中存储数据库文件名，之后对数据库文件操作时可根据此打开数据库文件；打开数据库文件，若不存在则新建，之后关闭文件；最后调用resetIndex函数，根据当前数据库文件内容重建索引

析构函数：将KVDB，索引清空；将offset归零；

resetIndex：首先将索引清空，offset偏移值归零；将文件长度存为length用以遍历文件，在while循环中调用readKVDBData函数从文件中读取每一组K-V单位元，更新当前offset值，存入索引；

```c++
void KVDBHandler::resetIndex()
{
	index.clear();
	offset = 0;

	const string filename = KVDB;
	fstream file;
	file.open(filename, ios::in | ios::binary);
	file.seekg(0, ios::end);
	int length = file.tellg();
	file.seekg(0, ios::beg);

	while (length != 0)
	{
		KVDBData s;
		readKVDBData(file, s);
		if (s.valueLen == -1)
			s.valueLen = 0;
		length -= sizeof(int) * 2 + s.keyLen + s.valueLen;

		index.set(s.key, offset);
		offset += sizeof(int) * 2 + s.keyLen + s.valueLen;
	}

	file.close();
}
```

 readKVDBData：执行程序前，文件读指针已定位；利用read函数按照keyLen，valueLen，key，value顺序从文件中读取，若valueLen为-1，则说明该key已被删除，不需读取value，之间将value设为空，之后将所有读取到的结果放入参数s中传回去；设置此函数目的为将从文件中读的过程封装起来，提高复用性

```c++
void KVDBHandler::readKVDBData(fstream& file, KVDBData& s)
{
	int keyLen, valueLen;
	file.read((char*)&keyLen, sizeof(int));
	file.read((char*)&valueLen, sizeof(int));
	
	string key, value;
	char* key_buf = new char[keyLen];
	file.read(key_buf, keyLen);
	for (int i = 0; i <= keyLen - 1; i++)
		key.push_back(key_buf[i]);
	delete[]key_buf;

	if (valueLen != -1)
	{
		char* value_buf = new char[valueLen];
		file.read(value_buf, valueLen);
		for (int i = 0; i <= valueLen - 1; i++)
			value.push_back(value_buf[i]);
		delete[]value_buf;
	}
	else value = "";

	s.set(keyLen, valueLen, key, value);
}
```

writeKVDBData：执行程序前，文件写指针已定位；利用write函数按照keyLen、valueLen、key，value顺序将单位元写入文件；设置此函数目的为将向文件中写的过程封装起来，提高复用性

```c++
void KVDBHandler::writeKVDBData(fstream& file, KVDBData& s)
{
	int keyLen = s.keyLen, valueLen = s.valueLen;
	string key = s.key, value = s.value;
	s.valueLen = s.value.length();

	file.write((char*)&keyLen, sizeof(int));
	file.write((char*)&valueLen, sizeof(int));

	file.write(key.c_str(), keyLen);
	file.write(value.c_str(), valueLen);
}
```

#### KVDBHandler友元函数

友元函数为操作数据库文件的真正接口

set：调用writeKVDBData函数；在索引中新增key和当前偏移值；偏移值加上新增长度；在最小堆中新增key，存为KEY_NOT_EXIST_IN_MINHEAP

```c++
int set(KVDBHandler* handler, const string& key, const string& value)
{
	const string filename = handler->KVDB;
	fstream file;
	file.open(filename, ios::binary | ios::app);

	KVDBData s;
	s.set(key.length(), value.length(), key, value);

	handler->writeKVDBData(file, s);
	handler->index.set(key, handler->offset);
	handler->offset += sizeof(int) * 2 + s.keyLen + s.valueLen;
	handler->minHeap.set(key, KEY_NOT_EXIST_IN_MINHEAP);
	file.close();
	return SUCCESS;
}
```

get：调用最小堆的del函数，将当前时间已过期的key从最小堆中删除；从索引中得到key所对应的偏移值；如果偏移值为KEY_NOT_EXIST，说明key不存在；调用最小堆的get函数，若为false则此key已过期；借助偏移值定位文件读指针，从文件中读出key对应的K-V组；若valueLen为-1则说明此key已被删除

```c++
int get(KVDBHandler* handler, const string& key, string& value)
{
	handler->minHeap.del();

	const string filename = handler->KVDB;
	fstream file;
	file.open(filename, ios::binary | ios::in);

	KVDBData s;
	s.set(key.length(), value.length(), key, value);

	int offset = handler->index.get(key);
	if (offset == KEY_NOT_EXIST)
	{
		file.close();
		return KEY_NOT_EXIST;
	}

	bool keyOverdue = handler->minHeap.get(key);
	if (keyOverdue == false)
	{
		file.close();
		return OVERDUE_KEY;
	}

	file.seekg(offset, ios::beg);
	handler->readKVDBData(file, s);
	value = s.value;
	
	file.close();

	int flag = SUCCESS;
	if (s.valueLen == -1)
		return KEY_HAS_BEEN_DELETED;
	return SUCCESS;
}
```

del：调用get函数，若得到结果不为SUCCESS，则说明此key因为各种各样的原因已失效，删除失败；文件尾加key，同时valueLen设置为-1；索引中更新key的偏移值；offset更新

```c++
int del(KVDBHandler* handler, const std::string& key)
{
	string value;

	int flag = get(handler, key, value);
	if (flag != SUCCESS)
		return flag;

	const string filename = handler->KVDB;
	fstream file;
	file.open(filename, ios::binary | ios::app);

	KVDBData s;
	s.set(key.length(), -1, key, value);

	handler->writeKVDBData(file, s);
	handler->index.set(key, handler->offset);
	handler->offset += sizeof(int) * 2 + s.keyLen;
	handler->minHeap.set(key, KEY_NOT_EXIST_IN_MINHEAP);
	file.close();
	return SUCCESS;
}
```

purge：定义副本文件，调用两次purge子函数

```c++
int purge(KVDBHandler* handler)
{
	KVDBHandler* save_handler;
	save_handler = new KVDBHandler("save_KVDB.txt");
	int flag;
	flag = purgeSubfunction(handler, save_handler);
	flag = purgeSubfunction(save_handler, handler);
	handler->resetIndex();
	return flag;
}
```

purgeSubfunction：主要功能为将源文件中内容去除已失效的值写入副本文件；主要方式为遍历源文件索引，若源文件中此key可以得到但是副本文件中无法得到，则写入副本文件当中；源文件遍历完毕之后将源文件中内容删除

```c++
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
```

expires：调用get函数，若得到结果不为SUCCESS，则说明此key因为各种各样的原因已失效，设置过期失败；若成功，则调用最小堆set函数设置过期时间

```c++
int expires(KVDBHandler* handler, const string key, int n)
{
	string value;
	int flag = get(handler, key, value);
	if (flag == SUCCESS)
	{
		time_t cur = time(NULL);
		handler->minHeap.set(key, cur + n);
	}
	return flag;
}
```



### MinHeap类

用以K-V数据库中存储过期时间

数据成员：(string)MinHeap_filename存储最小堆文件名；(Hash)index作为历史记录，存储历史进入最小堆的key；(priority_queue<TimeNode, vector<TimeNode>, cmp>)minHeap最小堆存储设置过期时间的key

使用结构体TimeNode；

```c++
class MinHeap
{
private:
	priority_queue<TimeNode, vector<TimeNode>, cmp> minHeap;
	Hash index;
	string MinHeap_filename;
public:
	MinHeap();
	~MinHeap();
	void reset();
	void set(string key, int time);
	bool get(string key);
	void del();
};
```

void reset()：清空历史记录和最小堆；遍历最小堆文件，若从文件中读到的key的过期时间大于等于当前时间则推入最小堆，并在历史记录中存为KEY_EXIST_IN_MINHEAP；

void set(string key, int time)：将key和key的过期时间写入文件；推入最小堆；推入历史记录；

bool get(string key)：从历史记录中得到key，若返回值为EY_NOT_EXIST或KEY_NOT_EXIST_IN_MINHEAP，说明此key未过期，返回true；遍历最小堆，从最小堆中得到key；若能得到则说明未过期；若存在于历史记录但是不存在于最小堆说明此key已过期，返回false

void del()：遍历最小堆，将最小堆中过期时间小于当前时间的key全部从最小堆中删除

### Hash类

作为KVDBHandler中的索引以及存储MinHeap中的历史记录

数据成员：(list<HashDataNode>)root[HASHMAX]以链表形式存储哈希表

使用结构体HashDataNode；

```c++
class Hash
{
public:
	list<HashDataNode> root[HASHMAX];
	~Hash();
	int GetHashCode(string str);
	void set(string key, int offset);
	int get(string key);
	void clear();
};
```

GetHashCode：哈希函数，传入的字符串按照一定规律设置为hash值

set：得到此key对应的hash值；遍历对应的链表，若找到key则更新offset；若无法找到，则新增；

get：得到此key对应的hash值；遍历对应的链表，若找到key则返回对应的offset，若找不到则返回KEY_NOT_EXIST

clear：将哈希表清空

### LOGGER类

在程序运行过程中，打印日志到日志文件；日志分为两级，Debug和Error；

数据成员：(ofstream)outfile作为日志文件指针、(string)path存储日志文件路径

宏定义Debug和Error作为日志等级标志

```c++
#define Debug 1
#define Error 0
class LOGGER
{
private:
    ofstream outfile;
    string path;
    void output(string text, int act_level);
public:
    LOGGER(string path);
    void DEBUG(string text);
    void ERROR(string text);
    string getAns(int flag);
    string currTime();
};
```

构造函数：打开日志文件，向日志文件中写入开始记录日志提示及开始记录日志时间

output：根据参数中传入日志等级，将输入日志文件中话语添加上传入text内容并且格式化，写入日志文件

DEBUG：调用output函数，设置日志等级为Debug

ERRDR：调用output函数，设置日志等级为Error

getAns：根据传入flag值返回相应答复结果

currTime：得到当前时间并将其格式化为string类型

### DataStruct.h文件和ErrorReturnValue.h文件

DataStruct.h文件封装各个类所使用的结构体，包括KVDBData(内有可重置各数据成员的函数set)、TimeNode、TimeNode配套的比较结构体cmp、HashDataNode

ErrorReturnValue.h文件中对各种返回值进行宏定义；

```c++
#pragma once

#define SUCCESS 1
#define FAIL 0
#define KEY_NOT_EXIST -1
#define KEY_HAS_BEEN_DELETED -2
#define ERROR_EXECUTING_SUBFUNCTION -3
#define OVERDUE_KEY -4
#define KEY_EXIST_IN_MINHEAP -5
#define KEY_NOT_EXIST_IN_MINHEAP -6
/*
KEY_NOT_EXIST_IN_MINHEAP必须比KEY_EXIST_IN_MINHEAP小
保证存储在最小堆中时KEY_NOT_EXIST_IN_MINHEAP在KEY_EXIST_IN_MINHEAP前
*/
```



## 四. 测试

程序交互页面如下(以退出程序为例)：

![](https://github.com/2019151091/K-V-Database/blob/main/%E7%A8%8B%E5%BA%8F%E4%BA%A4%E4%BA%92%E9%A1%B5%E9%9D%A2.png)

K-V单位元在数据库中可能存在状态以及相应状态下set/get/del/expires函数操作后结果如下：

![](https://github.com/2019151091/K-V-Database/blob/main/K-V%E5%8D%95%E4%BD%8D%E5%85%83%E5%8F%AF%E8%83%BD%E5%AD%98%E5%9C%A8%E7%8A%B6%E6%80%81%E4%BB%A5%E5%8F%8A%E5%87%BD%E6%95%B0%E6%93%8D%E4%BD%9C%E7%BB%93%E6%9E%9C.png)

针对其K-V单位元存在状态设计的测试方案以及预期结果如下：

| 测试类型 | 命令    | Key  | Value | 结果              | 测试类型 | 命令    | Key  | Value | 结果              |
| -------- | ------- | ---- | ----- | ----------------- | -------- | ------- | ---- | ----- | ----------------- |
| 从未存在 | set     | 1    | 123   | SUCCESS           | 未过期   | set     | 5    | 567   | SUCCESS           |
|          | get     | 2    |       | KEY NOT EXIST     |          | get     | 5    |       | SUCCESS,Value=567 |
|          | del     | 3    |       | KEY NOT EXIST     | 0s       | expires | 5    | 20s   | SUCCESS           |
|          | expires | 5    | 10s   | KEY NOT EXIST     |          | get     | 5    |       | SUCCESS,Value=567 |
|          |         |      |       |                   | 10s      | expires | 5    | 10s   | SUCCESS           |
| 正常存在 | set     | 2    | 234   | SUCCESS           |          | set     | 5    | 56    | SUCCESS           |
|          | get     | 2    |       | SUCCESS,Value=234 | 30s      | get     | 5    |       | SUCCESS,Value=56  |
|          | set     | 2    | 23    | SUCCESS           |          | expires | 5    | 10    | SUCCESS           |
|          | get     | 2    |       | SUCCESS，Value=23 | 5s       | del     | 5    |       | SUCCESS           |
|          | del     | 2    |       | SUCCESS           | 12s      | get     | 5    |       | DELETED           |
|          | get     | 2    |       | DELETED           |          |         |      |       |                   |
|          | set     | 2    | 32    | SUCCESS           | 已过期   | set     | 6    | 678   | SUCCESS           |
|          | expires | 2    | 10s   | SUCCESS           |          | get     | 6    |       | SUCCESS,Value=678 |
|          |         |      |       |                   |          | expires | 6    | 1s    | SUCCESS           |
| 已被删除 | set     | 3    | 345   | SUCCESS           |          | get     | 6    |       | OVERDUE           |
|          | del     | 3    |       | SUCCESS           |          | del     | 6    |       | OVERDUE           |
|          | get     | 3    |       | DELETED           |          | expires | 6    |       | OVERDUE           |
|          | del     | 3    |       | DELETED           |          | set     | 6    | 67    | SUCCESS           |
|          | expires | 3    | 30s   | DELETED           |          | get     | 6    |       | SUCCESS,Value=67  |
|          | set     | 3    | 34    | SUCCESS           | 退出程序 |         |      |       |                   |
|          | get     | 3    |       | SUCCESS,Value=34  |          |         |      |       |                   |
|          |         |      |       |                   |          |         |      |       |                   |



在严格执行测试流程之后，程序相应反应如预期所料，日志文件中结果如下：

![](https://github.com/2019151091/K-V-Database/blob/main/%E8%BF%9B%E8%A1%8C%E6%B5%8B%E8%AF%95%E4%B9%8B%E5%90%8E%E6%97%A5%E5%BF%97%E6%96%87%E4%BB%B6%E5%86%85%E5%AE%B9.png)