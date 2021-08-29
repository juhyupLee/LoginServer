#pragma once
#include <iostream>
#include <cpp_redis/cpp_redis>

//-----------------------------------------------------------
// ������Ʈ -> �Ӽ� -> VC++ ���丮 -> ���Ե��丮 ->  C:............\RedisLib ��� �߰�
//-----------------------------------------------------------
#pragma comment (lib, "RedisLib/cpp_redis.lib")
#pragma comment (lib, "RedisLib/tacopie.lib")
#pragma comment (lib, "ws2_32.lib")

class RedisConnector
{
public:
	RedisConnector();
	~RedisConnector();

	void Connect(const std::string& ip, size_t port);
	void Disconnect();
	cpp_redis::reply Get(const std::string& key);
	void Set(const std::string& key, const std::string& value);
	void SetEx(const std::string& key, int64_t second, const std::string& value);
private:
	cpp_redis::client* m_Client;
};