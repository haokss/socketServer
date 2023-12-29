#define _CRT_SECURE_NO_WARNINGS
#ifndef MEOWMESSAGE_HPP
#define MEOWMESSAGE_HPP

#include<iostream>
#include<ctime>
#include<vector>
#include<string>
enum class MeowDataType{
    LOGIN = 1, REG = 2,MESSAGE = 3, ADD = 4, DEL = 5
};
// 定义数据的发送类型 必须严格按照格式
//1、 Time: xxx\n
//2、 Type: xxx\n
//3、 Sender: xxxx\n   可选
//4、 Receive: xxxx,xxxx\n  可选
//5、 Content: xxxx
enum class MeowData{
    TIME =0, TYPE = 1, SENDER = 2, RECEIVE = 3, CONTENT = 4
};

class MeowMessage{
public:
    MeowMessage()=default;
    ~MeowMessage();
    // 序列化字节流
    // 请注意使用此函数一定要清除开辟的内存，防止内存泄漏
    char* Serialize()const;
    // 反序列化
    void DeSerialize(char *Buffer);
public:
    std::time_t send_time;
    MeowDataType type;
    int send_id;
    int receive_id;
    char *content;
};


#endif