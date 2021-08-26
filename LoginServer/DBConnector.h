#pragma once
/////////////////////////////////////////////////////////
// MySQL DB ���� Ŭ����
//������Ʈ-> �Ӽ� -> ����� -> ȯ��
// PATH=~~\MYSQLLib;%PATH%  DLL �˻��Ҽ��ֵ��� Path����
/////////////////////////////////////////////////////////

#pragma comment(lib,"MYSQLLib/mysqlcppconn.lib")
#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>


#include "MYSQLLib/Connector C++ 8.0/include/jdbc/cppconn/driver.h"
#include "MYSQLLib/Connector C++ 8.0/include/jdbc/cppconn/exception.h"
#include "MYSQLLib/Connector C++ 8.0/include/jdbc/cppconn/resultset.h"
#include "MYSQLLib/Connector C++ 8.0/include/jdbc/cppconn/statement.h"

class DBConnector
{
public:
	enum
	{
		QUERY_MAX_LEN = 2048,
		LOG_TIME = 500
	};

	//----------------------------------------------------
	// 	   Memory Pool�� �Ⱦ��ٸ�, �׳� �����ڿ� �Ű����� ������ �ʱ�ȭ
	//----------------------------------------------------
	DBConnector(const WCHAR* dbIP, const WCHAR* dbPort, const WCHAR* user, const WCHAR* password, const WCHAR* dbName);
	virtual		~DBConnector();

	//----------------------------------------------------
	// 	   MemoryPool�����ٸ� , �����ʱ�ȭ �� ��������.
	//----------------------------------------------------
	bool DBConInit(const WCHAR* dbIP, const WCHAR* dbPort, const WCHAR* user, const WCHAR* password, const WCHAR* dbName);
	DBConnector();
	bool DBConRelease();


	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	//////////////////////////////////////////////////////////////////////
	bool		Connect(void);

	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	//////////////////////////////////////////////////////////////////////
	bool		Disconnect(void);


	//////////////////////////////////////////////////////////////////////
	// ���� ������ ����� �ӽ� ����
	//
	//////////////////////////////////////////////////////////////////////
	bool		Query(WCHAR* szStringFormat, ...);
	bool		Query_Result(const WCHAR* stringFormat, ...);	// DBWriter �������� Save ���� ����
															// ������� �������� ����.

	sql::ResultSet* FetchResult();


private:
	std::wstring m_UserID;
	std::wstring m_DBIP;
	std::wstring m_DBPORT;
	std::wstring m_DBPassword;
	std::wstring m_DBName;

	sql::Statement* m_Query;
	sql::ResultSet* m_Result;
	LARGE_INTEGER m_QueryStartTime;
	LARGE_INTEGER m_QueryEndTime;
	LARGE_INTEGER m_QPF;
	bool m_bInit;


	static sql::Driver* m_Driver;
	sql::Connection* m_Connect;

};
