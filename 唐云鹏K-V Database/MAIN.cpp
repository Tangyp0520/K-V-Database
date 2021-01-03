#include<iostream>
#include<string>
#include"ErrorReturnValue.h"
#include"KVDBHandler.h"
#include"LOGGER.h"
using namespace std;



int main()
{
	KVDBHandler* handler;
	handler = new KVDBHandler("KVDB.txt");
	string command;
	LOGGER log("LOGGER_file.txt");

	while (1)
	{
		cout << "******************************" << endl;
		cout << "This K-V Database named 'test' does not support Chinese,does not support empty value,does not support space input" << endl;
		cout << "Command list:" << endl;
		cout << "1. SET" << endl;
		cout << "2. GET" << endl;
		cout << "3. DEL" << endl;
		cout << "4. PURGE" << endl;
		cout << "5. EXPIRES" << endl;
		cout << "6. EXIT" << endl;
		cout << "******************************" << endl;
		cout << "Please enter your command serial number according to the command list:" << endl;
		cout << "command serial number:";
		cin >> command;
		if (command == "1")
		{
			string key, value;
			cout << "key=";
			cin >> key;
			cout << "value=";
			cin >> value;
			int flag = set(handler, key, value);
			string ans = log.getAns(flag);

			ans = "Set "+ ans + " key=" + key + " value=" + value;
			log.DEBUG(ans);
			cout << ans << endl;
		}
		else if (command == "2")
		{
			string value, key;
			cout << "key=";
			cin >> key;
			int flag = get(handler, key, value);
			string ans = log.getAns(flag);
			
			ans = "Get " + ans;
			if (flag == SUCCESS)
				ans = ans + " " + key + "'s value=" + value;
			log.DEBUG(ans);
			cout << ans << endl;

		}
		else if (command == "3")
		{
			string key;
			cout << "key=";
			cin >> key;
			int flag = del(handler, key);
			string ans = log.getAns(flag);
			ans = "Delete " + ans;
			log.DEBUG(ans);
			cout << ans << endl;
		}
		else if (command == "4")
		{
			int flag = purge(handler);
			string ans = log.getAns(flag);
			ans = "Purge " + ans;
			log.DEBUG(ans);
			cout << ans << endl;
		}
		else if (command == "5")
		{
			string key;
			int n;
			cout << "key=";
			cin >> key;
			cout << key << "'s life cycle=";
			cin >> n;
			int flag = expires(handler, key, n);
			string ans = log.getAns(flag);
			ans = "Expires " + ans;
			log.DEBUG(ans);
			cout << ans << endl;
		}
		else if (command == "6")
		{
			cout << "Thank you for your use" << endl;
			break;
		}
		else cout << "error command" << endl;
		system("pause");
		system("cls");
	}
	return 1;
}