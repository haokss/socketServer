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
    void execute_query(SQLTCHAR *query);
    void show_error(SQLHANDLE handle, SQLSMALLINT type);
private:
    void connect();
};

#endif // SQLSERVER_H
