#include "mainclient.h"
#include "controldata.h"
#include <QApplication>
#include <QMetaType>

Q_DECLARE_METATYPE(StatusCode::StatusCode_t);

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    /* 注册类型 */
    qRegisterMetaType<StatusCode::StatusCode_t>("StatusCode::StatusCode_t");

    MainClient w;
    w.show();
    return a.exec();
}
