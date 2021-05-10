#pragma once
#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QRandomGenerator>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QNetworkConfigurationManager>
#include <iostream>
#include "controldata.h"

namespace SendStatus {
typedef enum {
    InitRequest, VerRequest, Work, Disconnect
} Status_t;
}


class Socket : public QObject {
    Q_OBJECT
public:
    explicit Socket(QObject *parent = nullptr);
    ~Socket();

public:
    QUdpSocket *m_socket;
    uint16_t m_id;
    /* static members */
    static QHostAddress host;
    static uint16_t port;
    static QHostAddress bindhost;
    static uint16_t bindport;

private:
    inline uint16_t generate_unique_id();

public:
    void send(const QString& );

public slots:

signals:
    void sig_online_car(const QMap<QString, QString>& );
};



class SocketThread : public QThread {
    Q_OBJECT
public:
    explicit SocketThread(QObject *parent = nullptr);
    Socket *m_sock;
    ~SocketThread();
    virtual void run() override;

private:
    SendStatus::Status_t m_sendStatus;
    StatusCode::StatusCode_t m_clientStatus;
    Mode::ControlMode_t m_controlMode;
    QString m_carId;
    QString m_sendMsg;
    QString m_lastMsg;
    void send_data();
    /* 获取收到的数据 */
    QByteArray get_recv_data();

    /* 服务器在线状态检测，超过5s服务器没有发送消息就是离线 */
    uint32_t m_ServerOfflineCnt;
    void server_offline_check();
    /* 解析收到的数据 */
    void parse_recv_data(const QString& );

private:
    inline bool check_network() {
        QNetworkConfigurationManager mgr;
        return mgr.isOnline();
    }

public slots:
    void recv_data_from_server();
    void slt_get_mode(bool);
    void slt_get_connect_car(const QString&);
    void slt_force_reflash();
    void slt_get_data(const QString& );
    void slt_disconnect();

signals:
    void sgl_client_status(StatusCode::StatusCode_t );
    void sgl_car_list(const QString& );
    void sgl_car_speed(const QString& );
};

