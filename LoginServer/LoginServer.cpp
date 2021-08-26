#include "LoginServer.h"
#include <process.h>
#include "Log.h"
#include "CommonProtocol.h"
#include "CPUUsage.h"
#include "MonitorPDH.h"
#include "DBConnector.h"
#include "CharacterEncoding.h"
#include "RedisConnector.h"



CPUUsage g_CPUUsage;
MonitorPDH g_MonitorPDH;

MyLoginServer::MyLoginServer()
    :NetServer(this),
    m_JobPool(500),
    m_ClientPool(500,true),
    m_DBPool(500)
{
    wcscpy_s(m_BlackIPList[0], L"185.216.140.27"); // 네덜란드 블랙 IP

    wcscpy_s(m_WhiteIPList[0], L"127.0.0.1"); // 루프팩
    wcscpy_s(m_WhiteIPList[1], L"10.0.1.2"); // Unit 1
    wcscpy_s(m_WhiteIPList[2], L"10.0.2.2"); // unit 1

   
    m_UpdateTPS = 0;

   m_MaxTCPRetrans =0;
   m_Min_MaxTCPRetrans=INT64_MAX;

   m_WakeTime = 0;

   m_LoginTPS = 0;


}

MyLoginServer::~MyLoginServer()
{
}

void MyLoginServer::ServerMonitorPrint()
{
    g_CPUUsage.UpdateCPUTime();
    g_MonitorPDH.ReNewPDH();

    wprintf(L"==========OS Resource=========\n");
    wprintf(L"UpdateThread SleepTime :%d\n", m_WakeTime);
    wprintf(L"HardWare U:%.1f  K:%.1f  Total:%.1f\n", g_CPUUsage.ProcessorUser(), g_CPUUsage.ProcessorKernel(), g_CPUUsage.ProcessorTotal());
    wprintf(L"Process U:%.1f  K:%.1f  Total:%.1f\n", g_CPUUsage.ProcessUser(), g_CPUUsage.ProcessKernel(), g_CPUUsage.ProcessTotal());
    wprintf(L"Private Mem :%lld[K]\n", g_MonitorPDH.GetPrivateMemory() / 1024);
    wprintf(L"NP Mem :%lld[K]\n", g_MonitorPDH.GetNPMemory() / 1024);
    wprintf(L"Available Mem :%.1lf[G]\n", g_MonitorPDH.GetAvailableMemory() / 1024);

    int64_t retransValue = g_MonitorPDH.GetTCPRetrans();
    if (m_MaxTCPRetrans < retransValue)
    {
        m_MaxTCPRetrans = retransValue;
    }
    if (m_Min_MaxTCPRetrans > retransValue)
    {
        m_Min_MaxTCPRetrans = retransValue;
    }
    wprintf(L"TCP Retransmit/sec  :%lld\n", retransValue);
    wprintf(L"Max TCP Retransmit/sec  :%lld\n", m_MaxTCPRetrans);
    wprintf(L"Min TCP Retransmit/sec  :%lld\n", m_Min_MaxTCPRetrans);


    wprintf(L"==========TPS=========\n");
    wprintf(L"Accept TPS : %d\nLogin TPS:%d\nSend TPS:%d\nRecv TPS:%d\n"
        , GetAcceptTPS()
        , m_LoginTPS
        , GetSendTPS()
        , GetRecvTPS());

    wprintf(L"==========Memory=========\n");
    wprintf(L"Packet PoolAlloc:%d \nPacket Pool Count :%d \nPacket Pool Use Count :%d \nClient Pool Alloc:%d \nClient Pool Count:%d \nClient Use Count:%d \nAlloc Memory Count:%d\nFree Memory Count:%d \nDB Alloc Count:%d \nDB Pool Count:%d \nDB Use Count:%d \n"
        , NetPacket::GetMemoryPoolAllocCount()
        , NetPacket::GetPoolCount()
        , NetPacket::GetUseCount()
        , m_ClientPool.GetChunkCount()
        , m_ClientPool.GetPoolCount()
        , m_ClientPool.GetUseCount()
        , g_AllocMemoryCount
        , g_FreeMemoryCount
        , m_DBPool.GetChunkCount()
        , m_DBPool.GetPoolCount()
        , m_DBPool.GetUseCount());

    wprintf(L"==========Network =========\n");
    wprintf(L"NetworkTraffic(Send) :%d \nTCP TimeOut ReleaseCount :% d \n"
        , GetNetworkTraffic()
        , g_TCPTimeoutReleaseCnt);


    wprintf(L"==========Count ===========\n");
    wprintf(L"Accept Count:%lld\tDisconnect Count :%d\nSession Count:%d\tClient Count:%llu\n"
        , GetAcceptCount()
        , g_DisconnectCount
        , m_SessionCount
        , m_ClientMap.size());
    wprintf(L"==============================\n");

    if (GetAsyncKeyState(VK_F2))
    {
        for (DWORD i = 0; i < m_MaxUserCount; ++i)
        {
            wprintf(L"Session[%d] SendQ UsedSize:%d\n", i, m_SessionArray[i]._SendQ.GetQCount());
        }
    }
    m_UpdateTPS = 0;
    m_LoginTPS = 0;
    //g_FreeMemoryCount = 0;
    //g_AllocMemoryCount = 0;

  /*  InterlockedExchange(&g_FreeMemoryCount, 0);
    InterlockedExchange(&g_AllocMemoryCount, 0);*/
}

bool MyLoginServer::OnConnectionRequest(WCHAR* ip, uint16_t port)
{
    for (int i = 0; i < MAX_WHITE_IP_COUNT; ++i)
    {
        // White IP와 같으면 바로 Return true
        if (!wcscmp(m_WhiteIPList[i], ip))
        {
            return true;
        }
    }
    _LOG->WriteLog(L"ChattingServer", SysLog::eLogLevel::LOG_LEVEL_SYSTEM, L"외부 IP:%s", ip);

    return false;
}

void MyLoginServer::OnClientJoin(uint64_t sessionID, WCHAR* ip, uint16_t port)
{
#if MEMORYLOG_USE ==1 
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::ON_CLIENT_JOIN, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
#endif

#if MEMORYLOG_USE  ==2
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::ON_CLIENT_JOIN, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
    curSession->_MemoryLog_IOCP.MemoryLogging(log);
#endif

    CreateNewUser(sessionID);
}

void MyLoginServer::OnClientLeave(uint64_t sessionID)
{
#if MEMORYLOG_USE ==1 
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::ON_CLIENT_LEAVE, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
#endif

#if MEMORYLOG_USE  ==2
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::ON_CLIENT_LEAVE, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
    curSession->_MemoryLog_IOCP.MemoryLogging(log);
#endif

    DeleteUser(sessionID);

}

void MyLoginServer::OnRecv(uint64_t sessionID, NetPacket* packet)
{
 
#if MEMORYLOG_USE ==1 
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::ON_RECV, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
#endif

#if MEMORYLOG_USE  ==2
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::ON_RECV, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
    curSession->_MemoryLog_IOCP.MemoryLogging(log);
#endif

    //--------------------------------------------------
    // 바로 OnRecv에서 메시지 마샬링
    //--------------------------------------------------
    MessageMarshalling(sessionID, packet);
}

void MyLoginServer::OnError(int errorcode, WCHAR* errorMessage)
{


}
void MyLoginServer::OnTimeOut(uint64_t sessionID)
{


}
MyLoginServer::Client* MyLoginServer::FindClient(uint64_t sessionID)
{
    Client* findClient = nullptr;

    auto iter = m_ClientMap.find(sessionID);
    if (iter == m_ClientMap.end())
    {
        return nullptr;
    }
    findClient = (*iter).second;

    return findClient;
}



bool MyLoginServer::LoginServerStart(WCHAR* ip, uint16_t port, DWORD runningThread, SocketOption& option, DWORD workerThreadCount, DWORD maxUserCount, TimeOutOption& timeOutOption)
{

    //-------------------------------------------------------------------
    // 워커스레드, Accept Thread  , Monitoring 스레드 가동
    //-------------------------------------------------------------------
    ServerStart(ip,port, runningThread,option,workerThreadCount,maxUserCount, timeOutOption);

    return true;
}

bool MyLoginServer::LoginServerStop()
{
    //-------------------------------------------------------------------
    // Update Thread 중지
    //-------------------------------------------------------------------
    ServerStop();

 

    return true;
}

void MyLoginServer::CreateNewUser(uint64_t sessionID)
{
#if MEMORYLOG_USE ==1 
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::CREATE_NEW_CLIENT, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
#endif

#if MEMORYLOG_USE  ==2
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::CREATE_NEW_CLIENT, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
    curSession->_MemoryLog_IOCP.MemoryLogging(log);
#endif

    Client* newClient = m_ClientPool.Alloc();

    newClient->_SessionID = sessionID;
    m_ClientMap.insert(std::make_pair(sessionID, newClient));

}

void MyLoginServer::DeleteUser(uint64_t sessionID)
{
    InterlockedIncrement(&g_OnClientLeaveCall);

#if MEMORYLOG_USE ==1 
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::DELETE_CLIENT, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
#endif

#if MEMORYLOG_USE  ==2
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::DELETE_CLIENT, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag);
    g_MemoryLog_IOCP.MemoryLogging(log);
    curSession->_MemoryLog_IOCP.MemoryLogging(log);
#endif

    auto iter = m_ClientMap.find(sessionID);
    if (iter == m_ClientMap.end())
    {
        Crash();
    }

    Client* delClient = (*iter).second;
    m_ClientMap.erase(iter);


   

    m_ClientPool.Free(delClient);
}

void MyLoginServer::MessageMarshalling(uint64_t sessionID, NetPacket* netPacket)
{
    WORD type;
    if ((*netPacket).GetPayloadSize() < sizeof(WORD))
    {
        Crash();
    }

    (*netPacket) >> type;
    

#if MEMORYLOG_USE ==1 
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::MESSAGE_MARSHAL, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag, (int32_t)2, -1, (eRecvMessageType)type, -1);
    g_MemoryLog_IOCP.MemoryLogging(log);
#endif

#if MEMORYLOG_USE  ==2
    Session* curSession = FindSession(sessionID);
    IOCP_Log log;
    log.DataSettiong(InterlockedIncrement64(&g_IOCPMemoryNo), eIOCP_LINE::MESSAGE_MARSHAL, GetCurrentThreadId(), curSession->_Socket, curSession->_IOCount, (int64_t)curSession, sessionID, (int64_t)&curSession->_RecvOL, (int64_t)&curSession->_SendOL, curSession->_SendFlag, (int32_t)2, -1, (eRecvMessageType)type, -1);
    g_MemoryLog_IOCP.MemoryLogging(log);
    curSession->_MemoryLog_IOCP.MemoryLogging(log);
#endif

    switch (type)
    {
    case en_PACKET_CS_LOGIN_REQ_LOGIN:
        PacketProcess_en_PACKET_CS_LOGIN_REQ_LOGIN(sessionID, netPacket);
        InterlockedIncrement(&m_LoginTPS);
        break;
    default:
#if DISCONLOG_USE ==1
        _LOG->WriteLog(L"ChattingServer", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"메시지마샬링: 존재하지않는 메시지타입 [Session ID:%llu] [Type:%d]", curSession->_ID, type);
#endif
        InterlockedIncrement(&g_FreeMemoryCount);
        netPacket->Free(netPacket);
        Disconnect(sessionID);
        break;
    }
}


void MyLoginServer::PacketProcess_en_PACKET_CS_LOGIN_REQ_LOGIN(uint64_t sessionID, NetPacket* netPacket)
{
    Client* client = FindClient(sessionID);
    if (client == nullptr)
    {
        Crash();
    }
    //---------------------------------------------
    // INT64(Account No)
    // Char64(SessionKey)
    //---------------------------------------------
    if (netPacket->GetPayloadSize() != sizeof(int64_t) + sizeof(char) * 64)
    {
#if DISCONLOG_USE ==1
        _LOG->WriteLog(L"ChattingServer", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"LoginPacket: 직렬화패킷 규격에 안맞는 메시지 [Session ID:%llu] [PayloadSize:%d]", sessionID, netPacket->GetPayloadSize());
#endif
        netPacket->Free(netPacket);
        InterlockedIncrement(&g_FreeMemoryCount);
        Disconnect(sessionID);
        return; 
    }
    (*netPacket) >> client->_AccoutNo;
    (*netPacket).GetData(client->_SessionKey, 64);

    netPacket->Clear();


    DBConnector* tempDB = m_DBPool.Alloc();
    tempDB->DBConInit(L"127.0.0.1", L"3306", L"root", L"tpwhd963", L"accountdb");
    if (!tempDB->Connect())
    {
        Crash();
    }
    //-------------------------------------------------------------------------
    // sessionKey table 접근 여기서 SessionKey를읽어와 클라이언트의 SessionKey와 비교한후
    // 일치하면 레디스에 넣는다.
    //-------------------------------------------------------------------------
    if (!tempDB->Query_Result(L"Select * from sessionkey where `accountno` = %lld", client->_AccoutNo))
    {
        Crash();
    }

    sql::ResultSet* resultDB = tempDB->FetchResult();
    if (resultDB == nullptr)
    {
        Crash();
    }

    std::string tempSessionKey;
    std::string tempAccountNo;

    while (resultDB->next())
    {
        tempSessionKey = resultDB->getString("sessionkey");
        tempAccountNo = resultDB->getString("accountno");
    }
    delete resultDB;


    RedisConnector tempRedis;

    tempRedis.Connect("127.0.0.1", 6379);
    
    
    if (!tempRedis.Get(tempAccountNo).is_null())
    {
        //---------------------------------
        // 중복접속
        //---------------------------------
        int a = 10;

    }

    
    tempRedis.Set(tempAccountNo, client->_SessionKey);
    tempRedis.Disconnect();

    //-------------------------------------------------------------------------
    // Account table 접근
    //-------------------------------------------------------------------------

    if (!tempDB->Query_Result(L"Select *from account where `accountno` = %lld", client->_AccoutNo))
    {
        Crash();
    }

    resultDB = tempDB->FetchResult();

    if (resultDB == nullptr)
    {
        Crash();
    }

    //--------------------------------------
    // 만약, 연결을 커넥션풀처럼 재활용한다고하면, Discoonect를하지않고
    // 그냥 메모리풀에 반납을 한다.(성능비교 필요)
    //--------------------------------------
    tempDB->Disconnect();

    int64_t tempAccountAno;
    std::string tempUserID;
    std::string tempUserPass;
    std::string tempUserNick;

    std::wstring wtempUserID;
    std::wstring wtempUserPass;
    std::wstring wtempUserNick;

    while (resultDB->next())
    {
        tempAccountAno = resultDB->getInt64("accountno");
        tempUserID = resultDB->getString("userid");
        tempUserPass = resultDB->getString("userpass");
        tempUserNick = resultDB->getString("usernick");
    }
    delete resultDB;

    tempDB->DBConRelease();
    m_DBPool.Free(tempDB);

    //if (tempAccountAno != client->_AccoutNo)
    //{
    //    Crash();
    //}
    UTF8ToUTF16(tempUserID.c_str(), wtempUserID);
    UTF8ToUTF16(tempUserPass.c_str(), wtempUserPass);
    UTF8ToUTF16(tempUserNick.c_str(), wtempUserNick);

    MakePacket_en_PACKET_CS_LOGIN_RES_LOGIN(netPacket, en_PACKET_CS_LOGIN_RES_LOGIN,tempAccountAno, dfLOGIN_STATUS_OK,  wtempUserID.c_str(), wtempUserNick.c_str(), L"127.0.0.1", 1000, L"127.0.0.1", 12001);

    SendUnicast(sessionID, netPacket);
}




void MyLoginServer::SendUnicast(uint64_t sessionID, NetPacket* packet)
{
    packet->IncrementRefCount();

    if (!SendPacket(sessionID, packet))
    {
        if (packet->DecrementRefCount() == 0)
        {
            InterlockedIncrement(&g_FreeMemoryCount);
            packet->Free(packet);
        }

        return;
    }

    PostQueuedCompletionStatus(m_IOCP, -1, NULL, (LPOVERLAPPED)sessionID);

    if (packet->DecrementRefCount() == 0)
    {
        InterlockedIncrement(&g_FreeMemoryCount);
        packet->Free(packet);
    }
}

void MyLoginServer::MakePacket_en_PACKET_CS_LOGIN_RES_LOGIN(NetPacket* packet, uint16_t type, int64_t accountNo, BYTE status,  const WCHAR* id, const WCHAR* nickName , const WCHAR* gameServerIP, uint16_t gameServerPort, const WCHAR* chattingServerIP, uint16_t chattingServerPort)
{
    *(packet) << type << accountNo << status;

    packet->PutData((char*)id, sizeof(WCHAR) * 20);
    packet->PutData((char*)nickName, sizeof(WCHAR) * 20);

    packet->PutData((char*)gameServerIP, sizeof(WCHAR) * 16);
    *(packet) << gameServerPort;

    packet->PutData((char*)chattingServerIP, sizeof(WCHAR) * 16);
    *(packet) << chattingServerPort;
}

