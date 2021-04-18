#include "mainclient.h"
#include "ui_mainclient.h"

MainClient::MainClient(QWidget *parent): QWidget(parent) , ui(new Ui::MainClient) {
    ui->setupUi(this);
}

MainClient::~MainClient() {
    delete ui;
}

