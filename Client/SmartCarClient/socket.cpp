#include "socket.h"
#include <QByteArray>
#include <QRegExp>
#include <iostream>
#include <QMap>
#include <QStringList>

#define LOCAL_TEST 1

#if LOCAL_TEST
QHostAddress Socket::host = QHostAddress("39.106.216.248");
uint16_t Socket::port = 19500;
#else
QHostAddress Socket::host = QHostAddress("127.0.0.1");
uint16_t Socket::port = 10000;
#endif

QHostAddress Socket::bindhost = QHostAddress("0.0.0.0");
uint16_t Socket::bindport = 10086;


Socket::Socket(QObject *parent) : QObject(parent), m_socket(new QUdpSocket), m_id(generate_unique_id()) {
    m_socket->bind(Socket::bindhost, Socket::bindport);
}

Socket::~Socket() {
    delete m_socket;
}

inline uint16_t Socket::generate_unique_id() {
    return QRandomGenerator::global()->bounded(65535);
}

void Socket::send(const QString &data) {
    QByteArray byte = data.toUtf8();
    this->m_socket->writeDatagram(byte.data(), byte.size(), Socket::host, Socket::port);
}




/*
 *  Socket thread class
 */
SocketThread::SocketThread(QObject *parent) : QThread(parent) {
    this->m_sock = new Socket();
    this->m_sendStatus = SendStatus::InitRequest;
    this->m_clientStatus = StatusCode::RequestConnect;
    this->m_controlMode = Mode::Remote;
    this->m_ServerOfflineCnt = 0;
    this->m_lastMsg = "";
    connect(this->m_sock->m_socket, &QUdpSocket::readyRead, this, &SocketThread::recv_data_from_server);
}

SocketThread::~SocketThread() {
    delete this->m_sock;
}

void SocketThread::send_data() {
    if (this->check_network() == false) {
        this->m_clientStatus = StatusCode::NetworkError;
    } else {
        switch (this->m_sendStatus) {
        /* 初始化请求 */
        case SendStatus::InitRequest: {
            /* 发送请求命令，发送后等待下一步 */
            this->m_sock->send(tr("master+%1+SmartClient+requests").arg(this->m_sock->m_id));
            break;
        }
        case SendStatus::VerRequest: {
            this->m_sock->send(tr("master+%1+SmartClient+connect+%2").arg(this->m_sock->m_id).arg(this->m_carId));
            break;
        }
        case SendStatus::Work: {
            QString msg = tr("master+%1+SmartClient+control+").arg(this->m_sock->m_id) + this->m_sendMsg;
            this->m_sock->send(msg);
            break;
        }
        case SendStatus::Disconnect: {
            this->m_sock->send(tr("master+%1+SmartClient+disconnect").arg(this->m_sock->m_id));
        }
        default:
            break;
        }
    }
    emit this->sgl_client_status(this->m_clientStatus);
}


void SocketThread::server_offline_check() {
    if (this->m_ServerOfflineCnt++ >= 25) {
        this->m_clientStatus = StatusCode::ServerOffline;
    }
}

void SocketThread::run() {
    while (true) {
        this->send_data();
        QThread::msleep(150);
        /* 离线检测 */
        this->server_offline_check();
    }
}

QByteArray SocketThread::get_recv_data() {
    QByteArray datagram;
    while (this->m_sock->m_socket->hasPendingDatagrams()) {
        datagram.resize(this->m_sock->m_socket->pendingDatagramSize());
        this->m_sock->m_socket->readDatagram(datagram.data(), datagram.size());
    }
    std::cout << "[Recv]: " << datagram.data() << std::endl;
    return datagram;
}

/*
 *  slots
 */
void SocketThread::recv_data_from_server() {
    this->m_ServerOfflineCnt = 0;
    QByteArray datagram = this->get_recv_data();
    std::cout << datagram.data() << std::endl;
    this->parse_recv_data(datagram.data());
}

void SocketThread::parse_recv_data(const QString &data) {
    switch (this->m_sendStatus) {
    case SendStatus::InitRequest: {
        QRegExp reg = QRegExp("server\\+([\\da-zA-Z\\,\\;]+)");
        if (reg.indexIn(data) >= 0 && this->m_lastMsg != data) {
            QString carLits = reg.cap(1);
            emit this->sgl_car_list(carLits);
        }
        this->m_clientStatus = StatusCode::ConnectSuccess;
        this->m_lastMsg = data;
        break;
    }
    case SendStatus::VerRequest: {
        if (data.contains("Fail") == true || data.contains("fail") == true) {
            std::cout << "Connect failed" << std::endl;
            this->m_clientStatus = StatusCode::AccessDeny;
        } else {
            this->m_sendStatus = SendStatus::Work;
            this->m_clientStatus = StatusCode::SendingData;
        }
        break;
    }
    case SendStatus::Work: {
        QRegExp reg = QRegExp("server\\+([\\da-zA-Z\\,\\-]+)");
        if (reg.indexIn(data) >= 0) {
            QString speed = reg.cap(1);
            if (speed != "Fail") {
                emit this->sgl_car_speed(speed);
            } else {
                std::cout << "Smart Car Offline" << std::endl;
                this->m_sendStatus = SendStatus::InitRequest;
            }
        } else {
            std::cout << "Work Error" << std::endl;
        }
        break;
    }
    case SendStatus::Disconnect: {
        if (data.contains("OK") == true) {
            std::cout << "Disconnect Successfully" << std::endl;
            this->m_sendStatus = SendStatus::InitRequest;
        }
        break;
    }
    default:
        break;
    }
}

void SocketThread::slt_get_mode(bool flag) {
    this->m_controlMode = flag ? Mode::Remote : Mode::Identify;
}

void SocketThread::slt_force_reflash() {
    this->m_sendStatus = SendStatus::InitRequest;
}

void SocketThread::slt_get_connect_car(const QString& id) {
    this->m_carId = id;
    this->m_sendStatus = SendStatus::VerRequest;
}

void SocketThread::slt_disconnect() {
    this->m_sendStatus = SendStatus::Disconnect;
}

void SocketThread::slt_get_data(const QString& data) {
    this->m_sendMsg = tr("%1,").arg(this->m_controlMode == Mode::Remote ? 1 : 0) + data;
}

