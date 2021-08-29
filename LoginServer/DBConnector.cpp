#include "DBConnector.h"
#include "Log.h"
#include "CharacterEncoding.h"
#include <strsafe.h>

sql::Driver* DBConnector::m_Driver = get_driver_instance(); 

DBConnector::DBConnector()
{
	
}

DBConnector::DBConnector(const WCHAR* dbIP,const WCHAR* dbPort, const WCHAR* user, const WCHAR* password, const WCHAR* dbName)
{
	QueryPerformanceFrequency(&m_QPF);
	m_DBIP = dbIP;
	m_DBPORT = dbPort;
	m_UserID = user;
	m_DBPassword = password;
	m_DBName = dbName;
	m_bInit = false;


}
bool DBConnector::DBConInit(const WCHAR* dbIP, const WCHAR* dbPort, const WCHAR* user, const WCHAR* password, const WCHAR* dbName)
{
	m_DBIP = dbIP;
	m_DBPORT = dbPort;
	m_UserID = user;
	m_DBPassword = password;
	m_DBName = dbName;

	//---------------------------------------------------
	// 	   생성자에선 Driver에대한 초기화만한다.
	//---------------------------------------------------
	m_Query = nullptr;
	m_Result = nullptr;
	m_Connect = nullptr;

	QueryPerformanceFrequency(&m_QPF);
	m_bInit = true;

	return m_bInit;
}
bool DBConnector::DBConRelease()
{
	if (m_Connect != nullptr)
	{
		if (!m_Connect->isClosed())
		{
			m_Connect->close();
		}
	}

	delete m_Connect;
	delete m_Query;

	return true;
}
DBConnector::~DBConnector()
{
	if (m_Connect != nullptr)
	{
		if (!m_Connect->isClosed())
		{
			m_Connect->close();
		}
	}
	//---------------------------------------
	// m_Result는 사용자가 메모리관리하는것으로 이관.
	//---------------------------------------
	delete m_Connect;
	delete m_Query;
}

bool DBConnector::Connect(void)
{
	//-----------------------------------------------------
	//연결중이라면 그대로 이 커넥션을 활용한다
	//-----------------------------------------------------
	if (m_Connect !=nullptr)
	{
		if (!m_Connect->isClosed())
		{
			return true;
		}
	}
	std::string ip;
	std::string dbPort;
	std::string user;
	std::string password;
	std::string dbName;

	UTF16ToUTF8(m_DBIP.c_str(), ip);
	UTF16ToUTF8(m_DBPORT.c_str(), dbPort);
	UTF16ToUTF8(m_UserID.c_str(), user);
	UTF16ToUTF8(m_DBPassword.c_str(), password);
	UTF16ToUTF8(m_DBName.c_str(), dbName);

	try
	{
		char hostAddr[100] = { 0, };
		sprintf_s(hostAddr, "tcp://%s:%s", ip.c_str(), dbPort.c_str());
		m_Connect = m_Driver->connect(hostAddr, user, password);

		//wprintf(L"m_Connect:%p\n", m_Connect);

		m_Connect->setSchema(dbName);

		//---------------------------------------------------
		// 	  Connect시, CreateStatement를 진행하고, Release에서 delete한다
		//---------------------------------------------------
		m_Query = m_Connect->createStatement();
	}
	catch (sql::SQLException& e)
	{
		std::wstring errorMessage;
		UTF8ToUTF16(e.what(), errorMessage);
		_LOG->WriteLog(L"SQL", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"SQL Connect() [ErroMessage:%s][ErrorCode:%d]", errorMessage.c_str(), e.getErrorCode());

		return false;
	}

	return true;
}

bool DBConnector::Disconnect(void)
{
	if (m_Connect != nullptr)
	{
		if (!m_Connect->isClosed())
		{
			m_Connect->close();
			return true;
		}
	}

	return false;
}

bool DBConnector::Query(WCHAR* stringFormat, ...)
{
	va_list va;
	sql::ResultSet* result;

	va_start(va, stringFormat);

	WCHAR message[QUERY_MAX_LEN];
	StringCchVPrintf(message, QUERY_MAX_LEN, stringFormat, va);

	std::string outString;
	UTF16ToUTF8(message, outString);

	try
	{
		//ping 확인
		if (m_Connect->isClosed() == true)
		{
			_LOG->WriteLog(L"SQL", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"Query_Result() Ping Error :응답없음");
			return false;
		}

		// 결과를 받지 않을때

		QueryPerformanceCounter(&m_QueryStartTime);
		m_Query->executeUpdate(outString.c_str());
		QueryPerformanceCounter(&m_QueryEndTime);

		int64_t queryTime = (m_QueryEndTime.QuadPart - m_QueryStartTime.QuadPart) / 10000;

		
		//--------------------------------------------------------------
		// 쿼리를 처리하는시간이 내가 설정한 시간보다 크다면, 에러로그를남긴다.(추후 디버깅을위해)
		// 쿼리 자체를 로그로 남긴다.
		//--------------------------------------------------------------
		if (LOG_TIME < queryTime)
		{
			_LOG->WriteLog(L"SQL", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"SQL Query() [걸린시간:%lld] [Query:%s]", queryTime, message);
		}


	}

	catch (sql::SQLException& e)
	{
		std::wstring errorMessage;
		UTF8ToUTF16(e.what(), errorMessage);

		_LOG->WriteLog(L"SQL", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"SQL Query():[Query:%s] [ErrorMessage:%s] [ErrorCode:%d]", message, errorMessage.c_str(), e.getErrorCode());
		return false;
	}


	return true;
}

bool DBConnector::Query_Result(const WCHAR* stringFormat, ...)
{
	va_list va;
	va_start(va, stringFormat);

	WCHAR message[QUERY_MAX_LEN];
	HRESULT result = StringCchVPrintf(message, QUERY_MAX_LEN, stringFormat, va);

	std::string outString;
	UTF16ToUTF8(message, outString);

	try
	{
		//ping 확인
		if (m_Connect->isClosed() == true)
		{
			_LOG->WriteLog(L"SQL", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"Query_Result() Ping Error :응답없음");
			return false;
		}
		// 결과를 받을때

		QueryPerformanceCounter(&m_QueryStartTime);
		m_Result = m_Query->executeQuery(outString.c_str());
		QueryPerformanceCounter(&m_QueryEndTime);

		int64_t queryTime = (m_QueryEndTime.QuadPart - m_QueryStartTime.QuadPart) / 10000;
		
		//--------------------------------------------------------------
		// 쿼리를 처리하는시간이 내가 설정한 시간보다 크다면, 에러로그를남긴다.(추후 디버깅을위해)
		// 쿼리 자체를 로그로 남긴다.
		//--------------------------------------------------------------
		if (LOG_TIME < queryTime)
		{
			_LOG->WriteLog(L"SQL", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"SQL QueryResult() [걸린시간:%lld] [Query:%s]", queryTime, message);
		}
	}

	catch (sql::SQLException& e)
	{
		std::wstring errorMessage;
		UTF8ToUTF16(e.what(), errorMessage);
		_LOG->WriteLog(L"SQL", SysLog::eLogLevel::LOG_LEVEL_ERROR, L"SQL QueryResult()[Query:%s] [ErrorMessage:%s] [ErrorCode:%d]", message, errorMessage.c_str(), e.getErrorCode());
		return false;
	}

	return true;
}

sql::ResultSet* DBConnector::FetchResult()
{
	if (m_Result == nullptr)
	{
		return nullptr;
	}
	return m_Result;
}
