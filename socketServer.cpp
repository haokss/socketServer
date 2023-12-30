#define _CRT_SECURE_NO_WARNINS
#include<iostream>
#include<WinSock2.h>
#include<thread>
#include<map>
#include<ctime>
#include<stdlib.h>
#include"MeowMessage.hpp"
#include"sqlserver.hpp"

#pragma comment(lib,"ws2_32.lib")

using namespace std;

// 返回当前时间
void Meow_now_time(){
    std::time_t now_time = std::time(nullptr);
    struct tm* now = localtime(&now_time);
    char str[50];
    strftime(str, sizeof(str),"%Y/%m/%d %H:%M:%S",now);
    std::cout<<str;
}

// 存储服务器连接soket列表
struct socketNode{
    int port;
    SOCKET clientSock;
};

/// 初始化全局变量
// socket连接
std::vector<socketNode> MeowSocket;
// 用户在线映射
std::map<int,socketNode> MeowOnlineMap;
// 用户暂存好友消息
std::vector<MeowMessage> MeowAddFriend;
// 数据库初始化
SQLserver sql;


void MeowPrintOnline(){
    for(const auto it: MeowOnlineMap){
        std::cout<<"the userID is:"<<it.first<<", port:"<<it.second.port<<std::endl;
    }
}

void HandleClient(SOCKET clientSocket, sockaddr_in clientAddr) {
    // 处理客户端的数据交互
    char buffer[BUFSIZ];
    int retVal;

    while (true) {
        ZeroMemory(buffer, BUFSIZ);
        retVal = recv(clientSocket, buffer, BUFSIZ, 0);
        if (retVal == SOCKET_ERROR) {
            cerr << "Receive error" << endl;
            closesocket(clientSocket);
            return;
        }

        if (retVal == 0) {
            // 将客户端从在线列表中移除
            socketNode delSocket;
            delSocket.port = clientAddr.sin_port;
            delSocket.clientSock = clientSocket;
            for(auto it = MeowOnlineMap.begin();it!=MeowOnlineMap.end();){
                if(it->second.port == delSocket.port){
                    std::cout<<it->first<<"client disconnected!"<<std::endl;
                    it = MeowOnlineMap.erase(it);
                }else{
                    ++it;
                }
            }
            // 清除socket连接
            // std::remove(MeowSocket.begin(),MeowSocket.end(),clientSocket);
            // 客户端断开连接
            closesocket(clientSocket);
            return;
        }
        Meow_now_time();
        std::cout <<"[Received form]: "<<clientAddr.sin_port<<"[byteBuffer]:"<<buffer;
        size_t buffer_size = strlen(buffer);

        // 解析客户端请求
        MeowMessage mmsg;
        mmsg.DeSerialize(buffer);
        cout<<"  MeowMessage->[sendTime]:"<<mmsg.send_time<<",[type]:"<<static_cast<typename std::underlying_type<MeowDataType>::type>(mmsg.type) <<",[id]:"<<mmsg.send_id<<",[receive]:"<<mmsg.receive_id<<",[content]:"<<mmsg.content<<std::endl;
        // 根据类型对消息执行对应的操作
        switch (mmsg.type)
        {
        case MeowDataType::LOGIN:
        {
            // 获取用户登录信息
            char *loginInfo = mmsg.content;
            char *token = strtok(loginInfo,"$");
            int _id = std::stoi(token);
            token= strtok(NULL,"$");
            char *_pwd = token;
            // 判断用户登录数据是否在数据库中,靠端口区分用户
            SQLCHAR query[] = "SELECT id, password FROM user_id WHERE id = ?";
            SQLINTEGER id = _id;
            SQLCHAR password[50];
            SQLLEN len_id = sizeof(id);
            SQLLEN len_password = sizeof(password);
            sql.execute_query(query,SQLINTEGER(id));
            // 绑定结果集
            SQLBindCol(sql.hstmt, 1, SQL_C_LONG, &id, 0, &len_id);
            SQLBindCol(sql.hstmt, 2, SQL_C_CHAR, password, sizeof(password), &len_password);
            // 获取查询结果
            SQLFetch(sql.hstmt);

            // 在这里处理查询结果并与 _pwd 进行比较
            if (len_id > 0 && len_password > 0) {
                // 进行密码比较等其他逻辑
                if(strcmp(reinterpret_cast<char*>(password),_pwd)==0){
                    // 从Socket列表中取出，加入在线队列
                    socketNode add_Node;
                    for(auto it:MeowSocket){
                        if(it.port==clientAddr.sin_port){
                            add_Node.port = it.port;
                            add_Node.clientSock = it.clientSock;
                        }
                    }
                    MeowOnlineMap[mmsg.send_id]= add_Node;
                    MeowPrintOnline();
                    // 回传登陆成功信息
                    char *message = "0#0#0#0#1";
                    retVal = send(clientSocket, message, strlen(message), 0);
                    if (retVal == SOCKET_ERROR) {
                        cerr << "Send error" << endl;
                        closesocket(clientSocket);
                        return;
                    }
                    // 从数据库加载加载用户数据并回传
                    SQLCHAR queryUser[] = "SELECT user_info.id,user_info.name,user_info.label FROM user_info WHERE id = ?";
                    sql.execute_query(queryUser,(SQLINTEGER)id);
                    // 解析查询结果
                    std::string msg = "0#0#0#0#";
                    if (SQL_SUCCEEDED(sql.ret)|| sql.ret == SQL_SUCCESS_WITH_INFO){
                        SQLCHAR str1[30],str2[10],str3[20];
                        SQLLEN len_str1, len_str2, len_str3;//字符串对应长度，你有几列就定义几个变量
                        while (SQLFetch(sql.hstmt) != SQL_NO_DATA)
                        {
                            SQLGetData(sql.hstmt, 1, SQL_C_CHAR, str1, 50, &len_str1);   //获取第一列数据
                            SQLGetData(sql.hstmt, 2, SQL_C_CHAR, str2, 50, &len_str2);   //获取第二列数据
                            SQLGetData(sql.hstmt, 3, SQL_C_CHAR, str3, 50, &len_str3);   //获取第三列数据
                            msg = msg+reinterpret_cast<const char*>(str1)+"$"+reinterpret_cast<const char*>(str2)+"$" +reinterpret_cast<const char*>(str3)+"|";
                        }

                    }
                    else{
                        cerr << "Error executing query." << endl;
                        sql.show_error(sql.hstmt, SQL_HANDLE_STMT);
                    }
                    SQLCHAR queryUser1[] = "SELECT user_info.id,user_info.name,user_info.label FROM user_info WHERE id in(SELECT friend_id FROM user_info,user_friend WHERE user_info.id = user_friend.id AND user_info.id = ?);";
                    sql.execute_query(queryUser1,(SQLINTEGER)id);
                    // 解析查询结果
                    if (SQL_SUCCEEDED(sql.ret)|| sql.ret == SQL_SUCCESS_WITH_INFO){
                        SQLCHAR str1[30],str2[10],str3[20];
                        SQLLEN len_str1, len_str2, len_str3;//字符串对应长度，你有几列就定义几个变量
                        while (SQLFetch(sql.hstmt) != SQL_NO_DATA)
                        {
                            SQLGetData(sql.hstmt, 1, SQL_C_CHAR, str1, 50, &len_str1);   //获取第一列数据
                            SQLGetData(sql.hstmt, 2, SQL_C_CHAR, str2, 50, &len_str2);   //获取第二列数据
                            SQLGetData(sql.hstmt, 3, SQL_C_CHAR, str3, 50, &len_str3);   //获取第三列数据
                            msg = msg+reinterpret_cast<const char*>(str1)+"$"+reinterpret_cast<const char*>(str2)+"$" +reinterpret_cast<const char*>(str3)+"|";
                        }

                    }
                    else{
                        cerr << "Error executing query." << endl;
                        sql.show_error(sql.hstmt, SQL_HANDLE_STMT);
                    }
                    SQLFreeStmt(sql.hstmt, SQL_CLOSE);
                    msg = msg+'\0';
                    Sleep(100);
                    const char *message1 = msg.c_str();
                    std::cout<<message1<<std::endl;
                    retVal = send(clientSocket, message1, strlen(message1), 0);
                    if (retVal == SOCKET_ERROR) {
                        cerr << "Send error" << endl;
                        closesocket(clientSocket);
                        return;
                    }
                    // 清除一次消息队列和待发送的好友请求队列, 查询自己是否有需要接受的消息
                    for(const auto it:MeowAddFriend){
                        if(it.receive_id ==mmsg.send_id){
                            // 找到对应id，发送好友请求
                            char * send1 = it.Serialize();
                            Sleep(100);
                            retVal = send(clientSocket, send1, strlen(send1), 0);
                            if (retVal == SOCKET_ERROR) {
                                cerr << "Send error" << endl;
                                closesocket(clientSocket);
                                return;
                            }
                            delete []send1;
                        }
                    }                     
                    // 消息发送结束标志
                    Sleep(100);
                    const char *end = "0#0#0#0#0";
                    retVal = send(clientSocket, end, strlen(end), 0);
                    if (retVal == SOCKET_ERROR) {
                        cerr << "Send error" << endl;
                        closesocket(clientSocket);
                        return;
                    }
                }else{
                    std::cout<<"incorrect password"<<std::endl;
                    // 回传登录失败消息
                    char *message = "0#0#0#0#0";
                    retVal = send(clientSocket, message, strlen(message), 0);
                    if (retVal == SOCKET_ERROR) {
                        cerr << "Send error" << endl;
                        closesocket(clientSocket);
                        return;
                    }
                }
            } else {
                std::cout << "User not found or incorrect password." << std::endl;
            }
        }break;
        case MeowDataType::MESSAGE:
        {
            char *message = nullptr;
            message = mmsg.Serialize();
            // 先回传相同的数据
            retVal = send(clientSocket, message, strlen(message), 0);
            std::cout<<"[send message]:"<<message<<std::endl;
            if (retVal == SOCKET_ERROR) {
                cerr << "Send error" << endl;
                closesocket(clientSocket);
                return;
            }
            /* 消息类型，如果消息接收人在线将消息转发给接收人，否则存入消息队列等待上线
            暂时操作：判断消息接收人是否在线，如果在线就发送消息，否则什么也不做*/
            // 判断在线
            auto it = MeowOnlineMap.find(mmsg.receive_id);
            if(it!=MeowOnlineMap.end()){
                SOCKET receiveSocket = it->second.clientSock;
                retVal = send(receiveSocket, message, strlen(message), 0);
                if (retVal == SOCKET_ERROR) {
                    cerr << "Send error" << endl;
                    closesocket(clientSocket);
                    return;
                }
            }else{
                // 什么都不做
                std::cout<<"receive is not online! "<<std::endl;
                // 存入暂时发送消息队列，等待用户上线
                
            }
            delete []message;
        }break;
        case MeowDataType::ADD:
        {
            // 将好友请求消息转发给接收方
            auto it = MeowOnlineMap.find(mmsg.receive_id);
            // 判断消息是发送请求还是接受请求
            if(strcmp(mmsg.content,"request")==0){
                // 存入暂时发送消息队列，等待用户再次上线 
                MeowAddFriend.push_back(mmsg);
                std::cout<<"receive is not online! Add to vector "<<std::endl;
            }else if(strcmp(mmsg.content,"accept")==0){
                // 双方成为好友,更新数据库信息

                // 返回成功消息
            }          
        }break;
        default:
            break;
        }
    }
}

int main()
{
    // 初始化Winsock库
    int RetVal;
    WORD SocketVersion=MAKEWORD(2, 2);
    WSADATA wsd;
    if (WSAStartup(SocketVersion, &wsd) != 0)
    {
        std::cout << "bind socket failed";
    }
    // 创建socket
    SOCKET ServerSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (ServerSocket == INVALID_SOCKET)
    {
        std::cout << "创建服务器套接字失败" << std::endl;
        WSACleanup();
        return -1;
    }
    // 绑定socket
    SOCKADDR_IN ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(2345);
    ServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    RetVal = bind(ServerSocket, (SOCKADDR *)&ServerAddr, sizeof(SOCKADDR_IN));
    if (RetVal == SOCKET_ERROR)
    {
        cout << "bind error" << endl;
        closesocket(ServerSocket);
        WSACleanup();
       return -1;
    }
    // 监听连接
    RetVal = listen(ServerSocket,2);
    if (RetVal == SOCKET_ERROR)
    {
        cout << "listen error" << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return -1;
    }
    cout << "Server listening on port 2345..." << endl;
    // 处理连接请求
    while(true){
        SOCKET ClientSocket;
        SOCKADDR_IN clientAddr;
        int ClientAddrLen = sizeof(clientAddr);
        ClientSocket = accept(ServerSocket, (SOCKADDR*)&clientAddr, &ClientAddrLen);
        if (ClientSocket == INVALID_SOCKET)
        {
            cout << "receive error" << endl;
            closesocket(ServerSocket);
            WSACleanup();
            return -1;
        }
        // 如果连接成功建立，加入到Socket连接列表，等待登录
        socketNode add_Node;
        add_Node.port = clientAddr.sin_port;
        add_Node.clientSock = ClientSocket;
        MeowSocket.push_back(add_Node);
        cout << "Client connected: " << inet_ntoa(clientAddr.sin_addr)<<":"<<clientAddr.sin_port<< endl;


        // 创建一个新线程来处理客户端连接
        thread(HandleClient, ClientSocket, clientAddr).detach();
    }
    // 关闭套接字和清理Winsock
    closesocket(ServerSocket);
    WSACleanup();
    return 0;

    }