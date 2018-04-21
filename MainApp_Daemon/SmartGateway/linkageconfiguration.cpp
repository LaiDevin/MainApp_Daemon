#include "linkageconfiguration.h"
#include "ui_linkageconfiguration.h"

linkageConfiguration::linkageConfiguration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::linkageConfiguration)
{
    ui->setupUi(this);
    init();
    connect(ui->CancelBtn,&QPushButton::clicked,this,&linkageConfiguration::CancelBtnClick);
    connect(ui->SureBtn,&QPushButton::clicked,this,&linkageConfiguration::SureBtnClick);
}

linkageConfiguration::~linkageConfiguration()
{
    delete ui;
}

void linkageConfiguration::init(){
    ui->linkageNumber->setText("linkage Number");
    ui->linkageSerPort->setText("linkage Serial Port");
}

void linkageConfiguration::CancelBtnClick(){

}

void linkageConfiguration::SureBtnClick(){

}
