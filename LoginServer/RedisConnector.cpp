#include "RedisConnector.h"
#include "Global.h"
#include "Log.h"
#include "CharacterEncoding.h"

RedisConnector::RedisConnector()
{
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(version, &data);
	m_Client = new cpp_redis::client;
	//wprintf(L"Redis m_Client:%p\n", m_Client);
}

RedisConnector::~RedisConnector()
{
	delete m_Client;
}

void RedisConnector::Connect(const std::string& ip, size_t port)
{
	if (m_Client->is_connected())
	{
		Crash();
	}
	try
	{
		m_Client->connect(ip, port);
	}
	catch (std::exception e)
	{
		std::wstring errorMessage;
		UTF8ToUTF16(e.what(), errorMessage);
		_LOG->WriteLog(L"Redis", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"Connect() Error :%s", errorMessage);
	}
}

void RedisConnector::Disconnect()
{
	try
	{
		if (m_Client->is_connected())
		{
			m_Client->disconnect();
		}
	}
	catch(std::exception e)
	{
		std::wstring errorMessage;
		UTF8ToUTF16(e.what(), errorMessage);
		_LOG->WriteLog(L"Redis", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"Disconnect() Error :%s", errorMessage);
	}
	
}

cpp_redis::reply RedisConnector::Get(const std::string& key)
{
	std::future<cpp_redis::reply> future;
	try
	{
		future = m_Client->get(key);
		m_Client->sync_commit();
		return future.get();
	}
	catch (std::exception e)
	{
		std::wstring errorMessage;
		UTF8ToUTF16(e.what(), errorMessage);
		_LOG->WriteLog(L"Redis", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"Get() Error :%s", errorMessage);
		return future.get();
	}
}

void RedisConnector::Set(const std::string& key, const std::string& value)
{
	try
	{
		m_Client->set(key, value);
	}
	catch (std::exception e)
	{
		std::wstring errorMessage;
		UTF8ToUTF16(e.what(), errorMessage);
		_LOG->WriteLog(L"Redis", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"Set() Error :%s", errorMessage);
	}
	//m_Client->sync_commit();
}
void RedisConnector::SetEx(const std::string& key,int64_t second, const std::string& value)
{
	try
	{
		m_Client->setex(key, second, value);
	}
	catch (std::exception e)
	{
		std::wstring errorMessage;
		UTF8ToUTF16(e.what(), errorMessage);
		_LOG->WriteLog(L"Redis", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"SetEx() Error :%s", errorMessage);
	}
}