#include"MeowMessage.hpp"

MeowMessage::~MeowMessage(){
    
}

std::string MeowMessage::Serialize() const{
    std::string data = std::to_string(send_time)+"#"+std::to_string((int)type)+"#"+std::to_string(send_id)+"#"+std::to_string(receive_id)+"#"+content;
    // char *SerializeData = new char[data.size()+1];
    // strcpy(SerializeData, data.c_str());
    return data;
    // 使用 strdup 复制字符串，并返回新分配的字符串指针
    // return strdup(data.c_str());
}

void MeowMessage::DeSerialize(char* Buffer){
    char * token = strtok(Buffer,"#");
    // 默认除Content以外，其他类型都不可能出现\n
    MeowData md = MeowData::TIME;
    while(token!=NULL){
        switch (md)
        {
        case MeowData::TIME:{
            this->send_time = std::stol(token);
            md = MeowData::TYPE;
        }break;
        case MeowData::TYPE:{
            this->type = static_cast<MeowDataType>(std::stoi(token));
            md = MeowData::SENDER;
        }break;
        case MeowData::SENDER:{
            send_id = std::stoi(token);
            md = MeowData::RECEIVE;
        }break;
        case MeowData::RECEIVE:{
            receive_id = std::stoi(token);
            md = MeowData::CONTENT;
        }break;
        case MeowData::CONTENT:{
            content = token;
        }break;
        default:
            std::cerr<<"receive unknow datatype";
            break;
        }
        token = strtok(NULL, "#");
    }

}

