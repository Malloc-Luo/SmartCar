#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainClient; }
QT_END_NAMESPACE

class MainClient : public QWidget
{
    Q_OBJECT

public:
    MainClient(QWidget *parent = nullptr);
    ~MainClient();

private:
    Ui::MainClient *ui;
};
