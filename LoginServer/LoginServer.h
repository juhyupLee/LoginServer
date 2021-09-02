#pragma once
#include "NetServerLib.h"
#include "DBConnector.h"
#include "RedisConnector.h"
#include <list>

#define SQL_IP L"10.0.2.2"
#define REDIS_IP "10.0.1.2"
#define CHATSERVER_IP L"10.0.1.1"

//#define SQL_IP L"127.0.0.1"
//#define REDIS_IP "127.0.0.1"
//#define CHATSERVER_IP L"127.0.0.1"



class MyLoginServer : public NetServer
{
public:
	enum class eJobTYPE
	{
		ON_CLIENT_JOIN,
		ON_CLIENT_LEAVE,
		ON_MESSAGE,
		ON_TIMEOUT
	};
	struct Job
	{
		eJobTYPE _TYPE;
		int64_t _SessionID;
		NetPacket* _NetPacketPtr;
	};
	enum
	{
		MAX_IP_COUNT = 100,
		MAX_WHITE_IP_COUNT = 3

	};
	struct Client
	{
		Client()
			: _SessionKey{0,}
		{
			_SessionID = 0;
			_AccoutNo = 0;

		}
		uint64_t _SessionID;

		int64_t _AccoutNo;
		char _SessionKey[64];
		//WCHAR _IP[17];
		//uint16_t _Port;
	};

	struct Black_IP
	{
		WCHAR _IP[17];
		uint16_t _Port;
	};
	struct White_IP
	{
		WCHAR _IP[17];
		uint16_t _Port;
	};

public:
	MyLoginServer();
	~MyLoginServer();
	void ServerMonitorPrint();

public:
	bool LoginServerStart(WCHAR* ip, uint16_t port, DWORD runningThread, SocketOption& option, DWORD workerThreadCount, DWORD maxUserCount, TimeOutOption& timeOutOption);
	bool LoginServerStop();
public:

	//------------------------------------------
	// Contentes
	//------------------------------------------
	virtual bool OnConnectionRequest(WCHAR* ip, uint16_t port);
	virtual void OnClientJoin(uint64_t sessionID, WCHAR* ip, uint16_t port);
	virtual void OnClientLeave(uint64_t sessionID);
	virtual void OnRecv(uint64_t sessionID, NetPacket* packet);
	virtual void OnError(int errorcode, WCHAR* errorMessage);
	virtual void OnTimeOut(uint64_t sessionID);

private:
	Client* FindClient(uint64_t sessionID);

	void CreateNewUser(uint64_t sessionID);
	void DeleteUser(uint64_t sessionID);

	void MessageMarshalling(uint64_t sessionID, NetPacket* netPacket);

	void MakePacket_en_PACKET_CS_LOGIN_RES_LOGIN(NetPacket* packet, uint16_t type, int64_t accountNo, BYTE status, const WCHAR* id, const WCHAR* nickName, const WCHAR* gameServerIP, uint16_t gameServerPort, const WCHAR* chattingServerIP, uint16_t chattingServerPort);

public:
	void PacketProcess_en_PACKET_CS_LOGIN_REQ_LOGIN(uint64_t sessionID, NetPacket* netPacket);
	void SendUnicast(uint64_t sessionID, NetPacket* packet);

public:

private:
	MyLock m_Lock;
	TLSDBConnector* m_TLSDBCon;


	DWORD m_RedisTlsIndex;
	LONG m_RedisConIndex;
	RedisConnector** m_RedisManager;
	

	MemoryPool_TLS<Client> m_ClientPool;

	std::unordered_map<uint64_t, Client*> m_ClientMap;

	WCHAR m_BlackIPList[MAX_IP_COUNT][17];
	WCHAR m_WhiteIPList[MAX_WHITE_IP_COUNT][17];

	//---------------------------------------------------------
	// 	   For Debug
	//---------------------------------------------------------
	LONG m_UpdateTPS;
	LONG m_LoginTPS;


	int64_t m_MaxTCPRetrans ;
	int64_t m_Min_MaxTCPRetrans;
};
