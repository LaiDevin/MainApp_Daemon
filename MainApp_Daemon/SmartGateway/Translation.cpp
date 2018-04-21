#include "Translation.h"
#include<QDebug>
#include<QApplication>
#include<QFile>
#include<QDir>
#include<QJsonArray>
#include<QJsonObject>

Translation *  Translation::m_staticSelf;
Translation::Translation(QObject *parent) : QObject(parent),combineObj({}),language_type(0)
{
    m_staticSelf = this;
    init();
}

Translation::~Translation(){

}

void Translation::init(){
    IniFile configFile;
    QString path = QDir::currentPath() + "/";
    configFile.LoadFile(path.toStdString() + "config.ini");
    std::string  translation=   configFile.ReadSection("Translation");
    if(translation == ""){
        configFile.WriteInteger("Translation","language",0);
    } else{
        int type = configFile.ReadInteger("Translation","language");
        qDebug()<<"read config file. language type is:"<<type;
        this->language_type = type;

    }
    // read translation file  , In language Type ,0 means English,1 means Chinese
    QString  fileFullPath = QDir::currentPath()+"/translation.csv";

    //    QString  fileFullPath = qApp->applicationDirPath()+"/translation.csv";
    qDebug()<<fileFullPath;
    QFile csvFile(fileFullPath);
    QStringList CSVList;
    CSVList.clear();
    if(csvFile.open(QIODevice::ReadOnly)){
        QTextStream stream(&csvFile);
        stream.setCodec("GBK");
        while (!stream.atEnd())
        {
            CSVList.push_back(stream.readLine());
        }
        csvFile.close();
    }else{
        qDebug()<<"read error";
    }
//    qDebug()<<"......"<<CSVList;
    combine(CSVList);
}

void Translation::combine(QStringList CSVList ){
    QJsonArray combineArr;
    Q_FOREACH(QString str, CSVList){
        QStringList  strList = str.split(",");

        QJsonObject obj;
        QJsonObject engObj;
        QJsonObject chnObj;
        QJsonObject keyObj;
        QJsonArray  tmpArr;
        //        keyObj.insert("keyvalue",strList.at(0));
        engObj.insert("english",strList.at(1));
        chnObj.insert("chinese",strList.at(2));
        tmpArr.push_back(engObj);
        tmpArr.push_back(chnObj);
        obj.insert(strList.at(0),tmpArr);
        combineArr.push_back(obj);
    }
    combineObj.insert("combine",combineArr);
//    qDebug()<<"print combine object :"<<combineObj;
}

//  type 0 == english,type 1 == chinese
QString  Translation::getLabelname(QString str){
//    qDebug()<<"str is:"<<str;
    QString lowerStr = str.toLower();
    QString keyStr = lowerStr.remove(QRegExp("\\s"));
    QJsonArray getCombineArr =  m_staticSelf->combineObj.value("combine").toArray();
    int combine_size = getCombineArr.size();
    for(int i =0 ; i<combine_size;++i){
        QJsonObject  value = getCombineArr.at(i).toObject();
        QString keyValue = value.begin().key();
        //        qDebug()<<"print key:"<<keyValue<<"----src str:"<<keyStr<<"combine value:"<<value<<"keyValue:"<<keyValue;
        if(keyValue == keyStr){
            //            qDebug()<<"...match in......m_staticSelf->language_type:"<<m_staticSelf->language_type;
            QJsonArray  getValueArr = value.value(keyValue).toArray();
            //            qDebug()<<"getValueArr:"<<getValueArr;
            //            qDebug()<<"getValueArr.at(0).toObject()"<<getValueArr.at(0).toObject()<<"getValueArr.at(1).toObject()"<<getValueArr.at(1).toObject();
            if(m_staticSelf->language_type == 0){
                return   getValueArr.at(0).toObject().value("english").toString();
            }else if(m_staticSelf->language_type== 1){
                return   getValueArr.at(1).toObject().value("chinese").toString();
            }
            //..... other language
        }
    }
}
