#ifndef LINKAGECONFIGURATION_H
#define LINKAGECONFIGURATION_H

#include <QWidget>

namespace Ui {
class linkageConfiguration;
}

class linkageConfiguration : public QWidget
{
    Q_OBJECT

public:
    explicit linkageConfiguration(QWidget *parent = 0);
    ~linkageConfiguration();

private:
    Ui::linkageConfiguration *ui;
    void init();
private slots:
    void  CancelBtnClick();
    void SureBtnClick();
};

#endif // LINKAGECONFIGURATION_H
