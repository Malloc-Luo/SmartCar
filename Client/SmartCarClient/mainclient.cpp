#include "mainclient.h"
#include "ui_mainclient.h"
#include "socket.h"
#include <QMap>
#include <QStringList>
#include <QKeyEvent>

static inline int ZOOM(int val, int min, int max) {
    return val < min ? min : (val > max ? max : val);
}
const int MAX_SPEED = 100;
/* 客户端状态 */
static QMap<StatusCode::StatusCode_t, QString> Tips = {{StatusCode::AccessDeny, "无法获得控制权限"},
                                                       {StatusCode::ConnectSuccess, "连接到服务器"},
                                                       {StatusCode::SendingData, "正在控制"},
                                                       {StatusCode::NetworkError, "网络错误"},
                                                       {StatusCode::ServerOffline, "无法连接到服务器"},
                                                       {StatusCode::RequestConnect, "请求连接中..."}};

MainClient::MainClient(QWidget *parent): QWidget(parent) , ui(new Ui::MainClient) {
    ui->setupUi(this);
    this->sockthread = new SocketThread;
    this->m_inctrl = false;
    this->m_speed = {0, 0, 0};
    /* 设置主窗口 */
    this->setWindowTitle("Smart Car Client");
    this->ui->rb_remote->setChecked(true);
    this->ui->lb_status->setText(" ");
    /* 初始化信号槽的连接 */
    this->init_connect();
    this->sockthread->start();
    this->timer0.start(100);
}

MainClient::~MainClient() {
    delete ui;
}

void MainClient::init_connect() {
    connect(&this->timer0, &QTimer::timeout, this, &MainClient::slt_timeout);
    connect(this->sockthread, &SocketThread::sgl_client_status, [&, this](StatusCode::StatusCode_t sta) {
        this->ui->lb_status->setText(Tips[sta]);
        this->m_clientStatus = sta;
    });
    connect(this->sockthread, &SocketThread::sgl_car_list, this, &MainClient::slt_get_car_list);
    connect(this->ui->pbt_connect, &QPushButton::clicked, this, &MainClient::slt_connect_car);
    connect(this->ui->rb_identify, &QRadioButton::clicked, this, &MainClient::slt_select_control_mode);
    connect(this->ui->rb_remote, &QRadioButton::clicked, this, &MainClient::slt_select_control_mode);
    connect(this->ui->pbt_reflash, &QPushButton::clicked, this->sockthread, &SocketThread::slt_force_reflash);
    connect(this->ui->checkBox, &QCheckBox::stateChanged, [&, this](int state) {
        this->m_inctrl = (state == Qt::Checked ? true : false);
    });
    connect(this->ui->checkBox,&QCheckBox::stateChanged, this, &MainClient::slt_ctrl_state_changed);
    connect(this, &MainClient::sgl_select_car, this->sockthread, &SocketThread::slt_get_connect_car);
    connect(this, &MainClient::sgl_select_mode, this->sockthread, &SocketThread::slt_get_mode);
    connect(this, &MainClient::sgl_send_ctrl_data, this->sockthread, &SocketThread::slt_get_data);
    connect(this, &MainClient::sgl_disconnect, this->sockthread, &SocketThread::slt_disconnect);
    connect(this->sockthread, &SocketThread::sgl_car_speed, this, &MainClient::slt_get_car_speed);
}

/*
 *  往ComboBox中添加项目
 */
void MainClient::slt_get_car_list(const QString& carlist) {
    QStringList list = carlist.split(";");
    /* 清空map中的所有项目 */
    this->m_cars.clear();
    this->ui->cb_car->clear();

    for (auto& car: list) {
        QStringList info = car.split(",");
        this->m_cars.insert(info[1], info[0]);
    }

    for (const QString& name: this->m_cars.keys()) {
        this->ui->cb_car->addItem(name);
    }
}

void MainClient::slt_connect_car() {
    if (this->ui->cb_car->count() > 0 && this->ui->pbt_connect->text() == "连接") {
        QString name = this->ui->cb_car->currentText();
        QString id = this->m_cars[name];
        emit this->sgl_select_car(id);
        this->ui->pbt_connect->setText("断开连接");
    } else if (this->ui->pbt_connect->text() == "断开连接") {
        emit this->sgl_disconnect();
        this->ui->pbt_connect->setText("连接");
    }
}

void MainClient::slt_select_control_mode() {
    emit this->sgl_select_mode(this->ui->rb_remote->isChecked());
}

void MainClient::slt_timeout() {
    if (this->m_inctrl == true) {
        QString data = tr("%1,%2,%3").arg(this->m_speed.vx).arg(this->m_speed.vy).arg(this->m_speed.vr);
        std::cout << data.toStdString() << std::endl;
        emit this->sgl_send_ctrl_data(data);
    } else {
        this->m_speed.vy = 0;
        this->m_speed.vx = 0;
        this->m_speed.vr = 0;
        emit this->sgl_send_ctrl_data("0,0,0");
    }
    this->timer0.start(200);
}

void MainClient::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_W) {
        this->m_speed.vy += 60;
    } else if (event->key() == Qt::Key_S) {
        this->m_speed.vy -= 60;
    } else if (event->key() == Qt::Key_A) {
        this->m_speed.vx += 60;
    } else if (event->key() == Qt::Key_D) {
        this->m_speed.vx -= 60;
    } else if (event->key() == Qt::Key_Control) {
        this->m_speed.vr += 40;
    } else if (event->key() == Qt::Key_Shift) {
        this->m_speed.vr -= 40;
    }

    if (event->key() == Qt::Key_Space) {
        this->m_speed.vx = 0;
        this->m_speed.vy = 0;
        this->m_speed.vr = 0;
    }

    this->m_speed.vy = ZOOM(this->m_speed.vy, -MAX_SPEED, MAX_SPEED);
    this->m_speed.vx = ZOOM(this->m_speed.vx, -MAX_SPEED, MAX_SPEED);
    this->m_speed.vr = ZOOM(this->m_speed.vr, -MAX_SPEED, MAX_SPEED);
}

void MainClient::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_W) {
        this->m_speed.vy -= 60;
    } else if (event->key() == Qt::Key_S) {
        this->m_speed.vy += 60;
    } else if (event->key() == Qt::Key_A) {
        this->m_speed.vx -= 60;
    } else if (event->key() == Qt::Key_D) {
        this->m_speed.vx += 60;
    } else if (event->key() == Qt::Key_Control) {
        this->m_speed.vr -= 40;
    } else if (event->key() == Qt::Key_Shift) {
        this->m_speed.vr += 40;
    }

    this->m_speed.vy = ZOOM(this->m_speed.vy, -60, 60);
    this->m_speed.vx = ZOOM(this->m_speed.vx, -60, 60);
    this->m_speed.vr = ZOOM(this->m_speed.vr, -40, 40);
}

void MainClient::slt_get_car_speed(const QString& data) {
    QStringList cars = data.split(",");
    if (cars.size() == 4) {
        for (int i =0; i < 4; i++) {
            this->m_encodeSpeed[i] = cars[i].toInt();
        }
    }
    this->ui->ln_v1->display(this->m_encodeSpeed[0]);
    this->ui->ln_v2->display(this->m_encodeSpeed[1]);
    this->ui->ln_v3->display(this->m_encodeSpeed[2]);
    this->ui->ln_v4->display(this->m_encodeSpeed[3]);
}

void MainClient::slt_ctrl_state_changed(bool flag) {
    if (flag == true) {
        this->ui->pbt_connect->setDisabled(true);
        this->ui->pbt_reflash->setDisabled(true);
        this->ui->rb_remote->setDisabled(true);
        this->ui->rb_identify->setDisabled(true);
    } else {
        this->ui->pbt_connect->setDisabled(false);
        this->ui->pbt_reflash->setDisabled(false);
        this->ui->rb_remote->setDisabled(false);
        this->ui->rb_identify->setDisabled(false);
    }
}
