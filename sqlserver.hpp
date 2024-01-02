#ifndef SQLSERVER_H
#define SQLSERVER_H
#undef UNICODE
#undef _UNICODE
#include<windows.h>
#include<iostream>
#include<sql.h>
#include<sqlext.h>
#include<sqltypes.h>
#include<string>
#include <iomanip>
#pragma comment(lib,"odbc32.lib")

class SQLserver
{
public:
    //定义全局变量
    SQLRETURN ret = SQL_SUCCESS;//返回信息
    SQLHENV henv = NULL;//环境句柄
    SQLHDBC hdbc = NULL;//连接句柄
    SQLHSTMT hstmt = NULL;//语句句柄
public:
    SQLserver();
    ~SQLserver();
    template<typename T> void execute_query(SQLTCHAR *query, T param);
    void execute_add(SQLTCHAR *add);
    void show_error(SQLHANDLE handle, SQLSMALLINT type);
private:
    void connect();
};

// 通用的查询接口，能绑定一个参数
template<typename T>
void SQLserver::execute_query(SQLTCHAR *query, T param)
{
    SQLLEN paramLen = sizeof(param);
    // 分配语句句柄
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    // 准备查询，非多次执行则不必要
    // SQLPrepare(hstmt, query, SQL_NTS);
    // 绑定参数
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &param, 0, &paramLen);
    // 直接执行查询
    ret = SQLExecDirect(hstmt,(SQLTCHAR *)query, SQL_NTS);
}

#endif // SQLSERVER_H
