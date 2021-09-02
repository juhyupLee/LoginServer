
#include "LoginServer.h"
#include <conio.h>
#include "MemoryDump.h"
#include "Global.h"
#include "Log.h"

#include "RedisConnector.h"
CrashDump dump;
int main()
{
	RedisConnector tempRedis;
	tempRedis.Connect("127.0.0.1", 6379);
	tempRedis.Set("hello", "42");
	std::cout << tempRedis.Get("hello");


	while (true)
	{

		tempRedis.Set("hello", "42");
		//tempRedis.Get("hello");

		if (GetAsyncKeyState(VK_F2))
		{
			wprintf(L"Stop\n");
			Sleep(INFINITE);

		}
	}
	//MyLoginServer* loginServer = new  MyLoginServer();
	//TimeOutOption timeOutOption;
	//
	//SocketOption sockOption;
	//timeOutOption._LoginTimeOut = 20000;
	//timeOutOption._HeartBeatTimeOut = 40000;
	//timeOutOption._OptionOn = false;

	//sockOption._KeepAliveOption.onoff = 0;
	//sockOption._Linger = true;
	//sockOption._TCPNoDelay = false;
	//sockOption._SendBufferZero = false;
	//bool bServerStartFlag = false;

	//int threadNum = 0;
	//int maxUser = 0;
	//int runningThread = 0;
	//
	//wprintf(L"워커스레드 갯수:");
	//wscanf_s(L"%d", &threadNum);

	//wprintf(L"러닝 스레드 갯수:");
	//wscanf_s(L"%d", &runningThread);

	//wprintf(L"최대 유저수:");
	//wscanf_s(L"%d", &maxUser);


	//loginServer->LoginServerStart(nullptr, 30000, runningThread, sockOption, threadNum, maxUser, timeOutOption);
	//bServerStartFlag = true;


	//while (true)
	//{
	//	if (GetAsyncKeyState(VK_F4))
	//	{
	//		break;
	//	}
	//	if (_kbhit())
	//	{
	//		WCHAR temp = _getwch();

	//		if (temp == L'Q' || temp == L'q')
	//		{
	//			if (bServerStartFlag)
	//			{
	//				bServerStartFlag = false;
	//				loginServer->LoginServerStop();
	//			}
	//			else
	//			{
	//				wprintf(L"이미 서버가 종료되었습니다\n");
	//			}
	//		}
	//		if (temp == L's' || temp == L'S')
	//		{
	//			if (!bServerStartFlag)
	//			{
	//				bServerStartFlag = true;
	//				loginServer->LoginServerStart(nullptr, 30000, runningThread, sockOption, threadNum, maxUser, timeOutOption);
	//			}
	//			else
	//			{
	//				wprintf(L"서버가 이미 가동중입니다\n");
	//			}
	//			
	//			
	//		}
	//	}
	//	loginServer->ServerMonitorPrint();
	//	Sleep(999);
	//}

	//if (bServerStartFlag)
	//{
	//	loginServer->ServerStop();
	//}
	//delete loginServer;

	//return 0;

}