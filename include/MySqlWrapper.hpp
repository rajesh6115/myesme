#ifndef __CMySQL_H__
#define __CMySQL_H__

#include<mysql.h>
#include<string>
#include<cstring>
#include <mutex> 
class CMySQL
{

        public:
                CMySQL();
                ~CMySQL();
                int mcfn_Open(const char *cpDBSource, const char *cpDBName,const char *cpDBUserName,const char *cpDBPassword);
                int mcfn_Execute(const char *cpExecuteQuery, int &iErrorCode, char *cpErrorInfo);
                int mcfn_GetResultSet(const char *cpSQLQuery, int &iErrorCode, char *cpErrorInfo);
                int mcfn_Close();
		int mcfn_reconnect();
		bool mcfb_isConnectionAlive();
		long mcfl_getNumRows();
		long mcfl_getNumCols();
		int mcfS_getFieldType(int siL_OffSet);
		char *mcfS_getFieldName(int siL_OffSet);
        private:
                int m_iReturnValue;
		std::string strDbSource;
		std::string strDbName;
		std::string strDbUserName;
		std::string strDbPassword;
                std::mutex meC_CriticalSection;
        public:
                MYSQL* m_pConnectionPtr;
                MYSQL_RES *m_pRecordsetPtr;
};

#endif

