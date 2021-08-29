#pragma once
/////////////////////////////////////////////////////////
// MySQL DB 연결 클래스
//프로젝트-> 속성 -> 디버깅 -> 환경
// PATH=~~\MYSQLLib;%PATH%  DLL 검색할수있도록 Path설정
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
	// 	   Memory Pool을 안쓴다면, 그냥 생성자에 매개변수 던져서 초기화
	//----------------------------------------------------
	DBConnector(const WCHAR* dbIP, const WCHAR* dbPort, const WCHAR* user, const WCHAR* password, const WCHAR* dbName);
	virtual		~DBConnector();

	//----------------------------------------------------
	// 	   MemoryPool을쓴다면 , 직접초기화 및 연결유지.
	//----------------------------------------------------
	bool DBConInit(const WCHAR* dbIP, const WCHAR* dbPort, const WCHAR* user, const WCHAR* password, const WCHAR* dbName);
	DBConnector();
	bool DBConRelease();


	//////////////////////////////////////////////////////////////////////
	// MySQL DB 연결
	//////////////////////////////////////////////////////////////////////
	bool		Connect(void);

	//////////////////////////////////////////////////////////////////////
	// MySQL DB 끊기
	//////////////////////////////////////////////////////////////////////
	bool		Disconnect(void);


	//////////////////////////////////////////////////////////////////////
	// 쿼리 날리고 결과셋 임시 보관
	//
	//////////////////////////////////////////////////////////////////////
	bool		Query(WCHAR* szStringFormat, ...);
	bool		Query_Result(const WCHAR* stringFormat, ...);	// DBWriter 스레드의 Save 쿼리 전용
															// 결과셋을 저장하지 않음.

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
