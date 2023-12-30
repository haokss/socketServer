#include "sqlserver.hpp"

SQLserver::SQLserver()
{
    connect();
}

// 关闭数据库连接
SQLserver::~SQLserver()
{
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

// 数据库连接函数
void SQLserver::connect()
{
    SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);//申请环境
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);//设置环境
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);//申请数据库连接
    SQLCHAR* connStr = (SQLCHAR*)"DRIVER={SQL Server};SERVER=43.143.200.197,1433;"
                                 "DATABASE=MeowTiger;UID=sa;PWD=123456;";
    ret = SQLDriverConnect(hdbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    // 判断数据库是否连接成功
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr<<"database connect failed! "<<std::endl;
    } else {
        std::cout<<"database connect success! "<<std::endl;
    }
}

// 处理数据库错误
void SQLserver::show_error(SQLHANDLE handle, SQLSMALLINT type)
{
    SQLINTEGER i = 0;
    SQLINTEGER native;
    SQLCHAR state[7];
    SQLCHAR text[256];
    SQLSMALLINT len;
    SQLRETURN ret;

    do
    {
        ret = SQLGetDiagRec(type, handle, ++i, state, &native, text,
                            sizeof(text), &len);
        if (SQL_SUCCEEDED(ret))
            std::cerr << "SQL Error: " << state << "(" << native << "): " << text << std::endl;
    } while (ret == SQL_SUCCESS);
}

// 标准查询
// // 分配语句句柄
// sql.ret = SQLAllocHandle(SQL_HANDLE_STMT, sql.hdbc, &sql.hstmt);
// // // 准备查询
// SQLPrepare(sql.hstmt, queryUser, SQL_NTS);
// // // 绑定参数
// SQLBindParameter(sql.hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, &len_id);
// // 执行查询
// sql.ret = SQLExecDirect(sql.hstmt,(SQLTCHAR *)queryUser1, SQL_NTS);