#include "sqlserver.h"

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

// 等待封装的查询接口
// 通用的查询接口
void SQLserver::execute_query(SQLTCHAR *query)
{
    // 判断用户登录数据是否在数据库中,靠端口区分用户
    SQLINTEGER id;
    SQLCHAR password[50];
    SQLLEN len_id = sizeof(id);
    SQLLEN len_password = sizeof(password);
    // 分配语句句柄
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    // 准备查询
    SQLPrepare(hstmt, query, SQL_NTS);
    // 绑定参数
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, &len_id);
    // 执行查询
    SQLExecute(hstmt);

    // 绑定结果集
    SQLBindCol(hstmt, 1, SQL_C_LONG, &id, 0, &len_id);
    SQLBindCol(hstmt, 2, SQL_C_CHAR, password, sizeof(password), &len_password);

    // 获取查询结果
    SQLFetch(hstmt);
    // 执行查询
    ret = SQLExecDirect(hstmt, (SQLTCHAR *)query, SQL_NTS);
    if (SQL_SUCCEEDED(ret)|| ret == SQL_SUCCESS_WITH_INFO){
        // 查询成功
    }
    else
    {
        std::cerr << "Error executing query." << std::endl;
        show_error(hstmt, SQL_HANDLE_STMT);
    }
    SQLFreeStmt(hstmt, SQL_CLOSE); // Free statement handle resources
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
