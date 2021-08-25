#include "RedisConnector.h"
RedisConnector::RedisConnector()
{
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(version, &data);
	m_Client = new cpp_redis::client;

}

RedisConnector::~RedisConnector()
{
	delete m_Client;

}

void RedisConnector::Connect(const std::string& ip, size_t port)
{
	m_Client->connect(ip, port);
}

void RedisConnector::Disconnect()
{
	m_Client->disconnect();
}

cpp_redis::reply RedisConnector::Get(const std::string& key)
{
	std::future<cpp_redis::reply> future = m_Client->get(key);
	m_Client->sync_commit();

	return future.get();
}

void RedisConnector::Set(const std::string& key, const std::string& value)
{
	m_Client->set(key, value);
}
