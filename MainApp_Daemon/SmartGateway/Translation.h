#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <QObject>
#include"IniFile.h"
#include<QJsonObject>

class Translation : public QObject
{
    Q_OBJECT
public:
    explicit Translation(QObject *parent = 0);
    ~Translation();
public:
  static  QString  getLabelname(QString str);
 private:
      void init();
      void combine(QStringList);
private :
      QJsonObject  combineObj;
    int language_type;
    static  Translation * m_staticSelf;
};

#endif // TRANSLATION_Hc
