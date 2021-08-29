
#include "LoginServer.h"
#include <conio.h>
#include "MemoryDump.h"
#include "Global.h"
#include "Log.h"
CrashDump dump;
int main()
{
	MyLoginServer* loginServer = new  MyLoginServer();
	TimeOutOption timeOutOption;
	
	SocketOption sockOption;
	timeOutOption._LoginTimeOut = 20000;
	timeOutOption._HeartBeatTimeOut = 40000;
	timeOutOption._OptionOn = false;

	sockOption._KeepAliveOption.onoff = 0;
	sockOption._Linger = true;
	sockOption._TCPNoDelay = false;
	sockOption._SendBufferZero = false;
	bool bServerStartFlag = false;

	int threadNum = 0;
	int maxUser = 0;
	int runningThread = 0;
	
	wprintf(L"��Ŀ������ ����:");
	wscanf_s(L"%d", &threadNum);

	wprintf(L"���� ������ ����:");
	wscanf_s(L"%d", &runningThread);

	wprintf(L"�ִ� ������:");
	wscanf_s(L"%d", &maxUser);


	loginServer->LoginServerStart(nullptr, 30000, runningThread, sockOption, threadNum, maxUser, timeOutOption);
	bServerStartFlag = true;


	while (true)
	{
		if (GetAsyncKeyState(VK_F4))
		{
			break;
		}
		if (_kbhit())
		{
			WCHAR temp = _getwch();

			if (temp == L'Q' || temp == L'q')
			{
				if (bServerStartFlag)
				{
					bServerStartFlag = false;
					loginServer->LoginServerStop();
				}
				else
				{
					wprintf(L"�̹� ������ ����Ǿ����ϴ�\n");
				}
			}
			if (temp == L's' || temp == L'S')
			{
				if (!bServerStartFlag)
				{
					bServerStartFlag = true;
					loginServer->LoginServerStart(nullptr, 30000, runningThread, sockOption, threadNum, maxUser, timeOutOption);
				}
				else
				{
					wprintf(L"������ �̹� �������Դϴ�\n");
				}
				
				
			}
		}
		loginServer->ServerMonitorPrint();
		Sleep(999);
	}

	if (bServerStartFlag)
	{
		loginServer->ServerStop();
	}
	delete loginServer;

	return 0;

}