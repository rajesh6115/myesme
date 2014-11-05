
#include"MySqlWrapper.hpp"


#define DBSTATUS_ERROR		0
#define DBSTATUS_SUCCESS	1
#define DBSTATUS_NODATA		2


CMySQL::CMySQL()
{
}

CMySQL::~CMySQL()
{
	if(m_pConnectionPtr!=NULL)
		mcfn_Close();
} 

int CMySQL::mcfn_Open(const char *cpDBSource, const char *cpDBName, const char *cpDBUserName, const char *cpDBPassword)
{
	
	strDbSource = cpDBSource;
	strDbName = cpDBName;
	strDbUserName = cpDBUserName;
	strDbPassword = cpDBPassword;
        try
        {
                m_pConnectionPtr = mysql_init(NULL);

                if (!m_pConnectionPtr)
                        return DBSTATUS_ERROR;
                else
                {
                        if (mysql_real_connect(m_pConnectionPtr, cpDBSource, cpDBUserName, cpDBPassword, cpDBName, 0, NULL, 0))
                                return DBSTATUS_SUCCESS;
                        else
			{
                                return DBSTATUS_ERROR;
			}
                }
        }
        catch (...)
        {
        }
        return DBSTATUS_ERROR;
}

int CMySQL::mcfn_Close()
{
        mysql_close(m_pConnectionPtr);
        m_pRecordsetPtr = NULL;
        m_pConnectionPtr = NULL;
        return DBSTATUS_SUCCESS;
}

int CMySQL::mcfn_Execute(const char *cpExecuteQuery, int &iErrorCode, char *cpErrorInfo)
{
        try
        {
                meC_CriticalSection.lock();
                m_iReturnValue = mysql_real_query(m_pConnectionPtr, cpExecuteQuery,strlen(cpExecuteQuery));

                if (!m_iReturnValue)
                {
                        meC_CriticalSection.unlock();
                        return DBSTATUS_SUCCESS;
                } 
                else
                {
                        meC_CriticalSection.unlock();			
			iErrorCode = mysql_errno(m_pConnectionPtr);
                        return DBSTATUS_ERROR;
                }
        }
        catch (...)
        {
                iErrorCode = mysql_errno(m_pConnectionPtr);
                strcpy(cpErrorInfo, mysql_error(m_pConnectionPtr));
        }
        meC_CriticalSection.unlock();
        return DBSTATUS_ERROR;
}

int CMySQL::mcfn_GetResultSet(const char *cpSQLQuery, int &iErrorCode, char *cpErrorInfo)
{
        try
        {
                meC_CriticalSection.lock();
                int iReturn = -1;
                iReturn = mysql_real_query(m_pConnectionPtr, cpSQLQuery,strlen(cpSQLQuery));
                if (iReturn > 0)
                {
                        meC_CriticalSection.unlock();
			iErrorCode = mysql_errno(m_pConnectionPtr);
                        return DBSTATUS_ERROR;
                }
                m_pRecordsetPtr = mysql_store_result(m_pConnectionPtr);
        }
        catch (...)
        {
                iErrorCode = mysql_errno(m_pConnectionPtr);
                strcpy(cpErrorInfo, mysql_error(m_pConnectionPtr));

                meC_CriticalSection.unlock();
                return DBSTATUS_ERROR;
        }
        meC_CriticalSection.unlock();
        return DBSTATUS_SUCCESS;
}

int CMySQL::mcfn_reconnect()
{
	meC_CriticalSection.lock();
        mysql_close(m_pConnectionPtr);
        m_pConnectionPtr = NULL;
	try
        {
                m_pConnectionPtr = mysql_init(NULL);
                if (!m_pConnectionPtr)
		{
			meC_CriticalSection.unlock();
                        return DBSTATUS_ERROR;
		}
                else
                {
                        if (mysql_real_connect(m_pConnectionPtr, strDbSource.c_str(), strDbUserName.c_str(), strDbPassword.c_str(), strDbName.c_str(), 0, NULL, 0))
			{
				meC_CriticalSection.unlock();
                                return DBSTATUS_SUCCESS;
			}
                        else
                        {
				meC_CriticalSection.unlock();
                                return DBSTATUS_ERROR;
                        }
                }
        }
        catch (...)
        {
        }
	meC_CriticalSection.unlock();
        return DBSTATUS_ERROR;
}

bool CMySQL::mcfb_isConnectionAlive()
{
	meC_CriticalSection.lock();
        int siL_RetVal = mysql_ping(m_pConnectionPtr);
	meC_CriticalSection.unlock();
        if(siL_RetVal == 0x00)
                return true;
        else
        {
                return false;
        }
}

long CMySQL::mcfl_getNumRows()
{
	if(m_pRecordsetPtr != NULL)
		return (long)mysql_num_rows(m_pRecordsetPtr);
	else
		return 0;
}

long CMySQL::mcfl_getNumCols()
{
	if(m_pRecordsetPtr != NULL)
		return mysql_num_fields(m_pRecordsetPtr);
	else
		return 0;

}

int CMySQL::mcfS_getFieldType(int siL_OffSet)
{
	MYSQL_FIELD *tmp;
	tmp = mysql_fetch_field_direct(m_pRecordsetPtr,siL_OffSet);
	return tmp->type;
}
	
char *CMySQL::mcfS_getFieldName(int siL_OffSet)
{
	MYSQL_FIELD *tmp;
	tmp = mysql_fetch_field_direct(m_pRecordsetPtr,siL_OffSet);
	return tmp->name;
}
