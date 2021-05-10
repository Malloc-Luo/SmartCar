#pragma once

#include <QWidget>
#include "socket.h"
#include "controldata.h"
#include "QString"
#include <QMap>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainClient; }
QT_END_NAMESPACE

class MainClient : public QWidget
{
    Q_OBJECT

public:
    MainClient(QWidget *parent = nullptr);
    ~MainClient();

    SocketThread *sockthread;

private:
    Ui::MainClient *ui;
    QMap<QString, QString> m_cars;
    QTimer timer0;
    StatusCode::StatusCode_t m_clientStatus;

    struct {
        int vx; int vy; int vr;
    } m_speed;

    int m_encodeSpeed[4];

    bool m_inctrl;
    void init_connect();

signals:
    void sgl_select_car(const QString& );
    /* true为遥控，false为识别 */
    void sgl_select_mode(bool);
    /* 发送控制信号 */
    void sgl_send_ctrl_data(const QString& );
    /* 断开连接 */
    void sgl_disconnect();

public slots:
    // void slt_get_client_status(StatusCode::StatusCode_t );
    void slt_get_car_list(const QString& );
    void slt_connect_car();
    void slt_select_control_mode();
    void slt_timeout();
    void slt_get_car_speed(const QString& );
    void slt_ctrl_state_changed(bool);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
};
