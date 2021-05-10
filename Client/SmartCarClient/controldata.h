#pragma once
/*
 * 通信用的数据结构体
 */
namespace Mode {
const unsigned char Remote = 0x01;
const unsigned char Track = 0x02;
const unsigned char Identify = 0x04;
typedef unsigned char ControlMode_t;
}

#pragma pack(push)
#pragma pack(1)

namespace Udp {
typedef struct {
    Mode::ControlMode_t mode;
    short vx;
    short vy;
    short vr;
} Send2SmartCarStruct_t;
}

namespace StatusCode {
typedef enum {
    RequestConnect, /* 正在请求连接 */
    ConnectSuccess, /* 连接到服务器*/
    ServerOffline,  /* 无法连接到服务器 */
    NetworkError,   /* 网络错误 */
    SendingData,    /* 正在控制 */
    AccessDeny,     /* 访问被拒绝 */
} StatusCode_t;
}

#pragma pack(pop)
