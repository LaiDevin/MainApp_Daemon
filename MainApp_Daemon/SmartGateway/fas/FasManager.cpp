#include "FasManager.h"
#include "FasFuan5101.h"
#include "FasJadeBird.h"
#include "FasJadeBird5012.h"
#include "FasJadeBird1010.h"
#include "FasHaiwanGST5000.h"
#include "FasJB3208B.h"
#include "FasHZQH8000B.h"
#include "FasQ100GZ2L_LA040.h"
#include "FasTBTC3000.h"
#include "FasQBOZH4800.h"
#include "FasCA2000SZ.h"
#include "FasJBTB242.h"
#include "FasQBTC3020.h"
#include "FasFHSJ_JB_QGL_9000.h"
#include "FasHaiwanGST500.h"
#include "FasFHSJ_JB_QBL_MN300.h"
#include "Fas_JB_JBF_11S.h"
#include "FasJB3208G.h"
#include "FasQGGST5000.h"
#include "Fas_JB_QT_OZH4800.h"
#include "FasJBLGQH8000.h"
#include "Fas_JB_TG_JBF_11SF.h"
#include "FasFHSJ_JB_QGL_9000_Printer.h"
#include "FASJBSJFF3208.h"
#include "FasJKB_193K.h"
#include "FasHaiwanGST200_ModbusRTU.h"
#include "FasHaiwanGST5000_ModbusRTU.h"
#include "FasTaihean_TX3607.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool s_bInitFlag = false;
static FasFuan5101 s_objFasFuan5101;
static FasJadeBird s_objFasJadeBird;
static FasHaiwanGST5000 s_objFasHaiwanGST5000;
static FasJB3208B s_objFasJB3208B;
static FasJadeBird1010 s_objFasJadeBird1010;
static FasJadeBird5012 s_objFasJadeBird5012;
static FasHZQH8000B s_objFasHZQH8000B;
static FasQ100GZ2L_LA040 s_objFasQ100GZ2L_LA040;
static FasTBTC3000 s_objFasTBTC3000;
static FasQBOZH4800 s_objFasQBOZH4800;
static FasCA2000SZ s_objFasCA2000SZ;
static FasJBTB242 s_objFasJBTB242;
static FasQBTC3020 s_objFasQBTC3020;
static FasFHSJ_JB_QGL_9000 s_objFasFHSJ_JB_QGL_9000;
static FasHaiwanGST500 s_objFasHaiwanGST500;
static FasFHSJ_JB_QBL_MN300 sobjFasFHSJ_JB_QBL_MN300;
static Fas_JB_JBF_11S sobjFas_JB_JBF_11S;
static FasJB3208G s_objFasJB3208G;
static FasQGGST5000 s_objFasQGGST5000;
static Fas_JB_QT_OZH4800 s_objFas_JB_QT_OZH4800;
static FasJBLGQH8000 s_objFasJBLGQH8000;
static Fas_JB_TG_JBF_11SF s_objFas_JB_TG_JBF_11SF;
static FasFHSJ_JB_QGL_9000_Printer s_objFasFHSJ_JB_QGL_9000_Printer;
static FAS_JB_SJFF3208 s_objFAS_JB_SJFF3208;
static FasJKB_193K s_ojbFasJKB_193K;
static FasHaiwanGST200_ModbusRTU s_objFasHaiwanGST200_ModbusRTU;
static FasHaiwanGST5000_ModbusRTU s_objFasHaiwanGST5000_ModbusRTU;
static FasTaihean_TX3607 s_objFas_Taihean_TX3607;

static FasInfo s_stFasInfo[] = {
	{FAS_ID_NONE, "关闭"},
	{FAS_ID_JBQB_FS5101, "赋安JB-QB-FS5101"},
    {FAS_ID_JBQT_GST5000, "海湾JB-QT-GST5000"},
    {FAS_ID_JBTT_JBF11S, "北大青鸟JB—TT-JBF11S"},
    {FAS_ID_JBQB_JBF5012, "北大青鸟JB-QB-JBF5012"},
    {FAS_ID_JBQB_LN1010, "北大青鸟JB-QB-LN1010"},
    {FAS_ID_Q100GZ2L_LA040, "泰和安Q100GZ2L-LA040"},
    {FAS_ID_TB_TC3000, "营口天成TB-TC3000"},
    {FAS_ID_JBQB_OZH4800, "奥瑞娜JB-QB-OZH4800"},
    {FAS_ID_JBLB_QH8000B, "杭州清华JB-LB-QH8000B"},
    {FAS_ID_JB_3208B,  "上海松江飞繁JB-3208B"},
    {FAS_ID_JBQB_TC3020, "营口天成JB-QB-TC3020"},
    {FAS_ID_JBTB_242,   "泰和安JB-TB-242"},
    {FAS_ID_JBLB_CA2000SZ, "成都安吉斯JB-LB-CA2000SZ"},
    {FAS_ID_JB_QGL_9000, "泛海三江JB-QGL-9000"},
	{FAS_ID_JBQB_GST500, "海湾JB-QB-GST500"},
	{FAS_ID_JB_QBL_MN300, "泛海三江JB_QBL_MN300"},
	{FAS_ID_JB_TB_JBF_11S, "北大青鸟JB-TB-JBF-11S"},
	{FAS_ID_JB_3208G,  "上海松江飞繁JB-3208G"},
	{FAS_ID_JBQG_GST5000,  "海湾JB-QG-GST5000"},
	{FAS_ID_JB_QT_OZH4800,  "奥瑞娜JB-QT-OZH4800"},
 	{FAS_ID_JB_LG_QH8000,  "杭州清华JB-LG-QH8000"},
	{FAS_ID_JB_TG_JBF_11SF,   "北大青鸟JB-TG-JBF-11SF"},
	{FAS_ID_JB_QGL_9000_PRINTER, "泛海三江JB-QGL-9000 打印机"},
	{FAS_ID_JB_SJFF3208, "松江非凡电子_JB3208"},
	{FAS_ID_JKB_193K, "北大青鸟JKB_193K"},
	{FAS_ID_JB_TB_JBF_11SF,   "北大青鸟JB-TB-JBF-11SF"},
	{FAS_ID_JB_TG_JBF_11S,   "北大青鸟JB-TG-JBF-11S"},
    {FAS_ID_GST200_MODBUS_RTU, "海湾MODBUS-RTU-GST200"},
    {FAS_ID_GST5000_MODBUS_RTU, "海湾MODBUS-RTU-GST5000"},
    {FAS_ID_TX_3607,"泰和安TX_3607"},
    {FAS_ID_ESSER_E98,"ESSER_E98"},
    {FAS_ID_ESSER_E98_RS232,"ESSER_E98_RS232"}
};

FasManager::Release FasManager::m_objRelease;
FasManager *FasManager::m_pobjFasManager = new FasManager();
static vector<FasComInfo> s_stFasList;

FasManager::FasManager(void)
{
#ifdef LINUX
	m_pobjGPIO = NULL;
#endif
}

FasManager::~FasManager(void)
{
}

FasManager *FasManager::getInstance(void)
{
	return m_pobjFasManager;
}

Fas *FasManager::getFasObj(int nFasID)
{
	switch(nFasID) {
		case FAS_ID_JBQB_FS5101://maybe create en enum type for FasID
			return &s_objFasFuan5101;
		case FAS_ID_JBTT_JBF11S://maybe create en enum type for FasID
			return &s_objFasJadeBird;
		case FAS_ID_JBQT_GST5000:
            return &s_objFasHaiwanGST5000;
		case FAS_ID_JB_3208B:
			return &s_objFasJB3208B;
		case FAS_ID_JBQB_LN1010:
			return &s_objFasJadeBird1010;
		case FAS_ID_JBQB_JBF5012:
			return &s_objFasJadeBird5012;
		case FAS_ID_JBLB_QH8000B:
			return &s_objFasHZQH8000B;
		case FAS_ID_Q100GZ2L_LA040:
			return &s_objFasQ100GZ2L_LA040;
		case FAS_ID_TB_TC3000:
			return &s_objFasTBTC3000;
		case FAS_ID_JBQB_OZH4800:
			return &s_objFasQBOZH4800;
		case FAS_ID_JBLB_CA2000SZ:
			return &s_objFasCA2000SZ;
        case FAS_ID_JBTB_242:
            return &s_objFasJBTB242;
        case FAS_ID_JBQB_TC3020:
            return &s_objFasQBTC3020;
        case FAS_ID_JB_QGL_9000:
            return &s_objFasFHSJ_JB_QGL_9000;
		case FAS_ID_JBQB_GST500:
            return &s_objFasHaiwanGST500;
		case FAS_ID_JB_QBL_MN300:
            return &sobjFasFHSJ_JB_QBL_MN300;
        case FAS_ID_JB_TB_JBF_11S:
		case FAS_ID_JB_TG_JBF_11S:
            return &sobjFas_JB_JBF_11S;
        case FAS_ID_JB_3208G:
			return &s_objFasJB3208G;
		case FAS_ID_JBQG_GST5000:
			return &s_objFasQGGST5000;
		case FAS_ID_JB_QT_OZH4800:
			return &s_objFas_JB_QT_OZH4800;
		case FAS_ID_JB_LG_QH8000:
			return &s_objFasJBLGQH8000;
		case FAS_ID_JB_TB_JBF_11SF:
		case FAS_ID_JB_TG_JBF_11SF:
		    return &s_objFas_JB_TG_JBF_11SF;
		case FAS_ID_JB_QGL_9000_PRINTER:
			return &s_objFasFHSJ_JB_QGL_9000_Printer;
		case FAS_ID_JB_SJFF3208:
			return &s_objFAS_JB_SJFF3208;
		case FAS_ID_JKB_193K:
			return &s_ojbFasJKB_193K;
        case FAS_ID_GST200_MODBUS_RTU:
           return &s_objFasHaiwanGST200_ModbusRTU;
        case FAS_ID_GST5000_MODBUS_RTU:
           return &s_objFasHaiwanGST5000_ModbusRTU;
        case  FAS_ID_TX_3607:
           return &s_objFas_Taihean_TX3607;
		default:
			break;
	}

	return NULL;
}

#ifdef LINUX
void FasManager::setGPIOOperator(GPIO *pobjGPIO)
{
	m_pobjGPIO = pobjGPIO;
}
#endif

bool FasManager::init(const vector<FasComInfo>& stFasList)
{
	if(s_bInitFlag) {
		return true;
	}

	for(size_t i = 0; i < stFasList.size(); ++ i){
		Fas *pobjFas = getFasObj(stFasList[i].nID);
		if(NULL != pobjFas) {
			pobjFas->setFasID(stFasList[i].nID);
			if(!pobjFas->init(stFasList[i].nComType, stFasList[i].strComName, stFasList[i].nBitrate, stFasList[i].nParity, stFasList[i].nDatabits, stFasList[i].nStopbits, stFasList[i].nFlowCtrl)){
				return false;
			}
		}
#ifdef LINUX
		if(stFasList[i].nComType == COM_TYPE_RS485){
			pobjFas->setGPIOOperator(m_pobjGPIO);
		}
#endif
	}

	s_stFasList.clear();
	s_stFasList.insert(s_stFasList.end(), stFasList.begin(), stFasList.end());
	s_bInitFlag = true;
	return true;
}

bool FasManager::unInit(void)
{
	if(s_bInitFlag) {
		for(size_t i = 0; i < s_stFasList.size(); ++ i){
			Fas *pobjFas = getFasObj(s_stFasList[i].nID);
			if(NULL != pobjFas) {
				pobjFas->setFasID(FAS_ID_NONE);
				pobjFas->unInit();
			}
		}
		s_bInitFlag = false;
	}

	return true;
}

int FasManager::recvData(int nFasID, int nComType, vector<PartRunStatus> &stPartStatus)
{
	Fas *pobjFas = getFasObj(nFasID);
	if(pobjFas == NULL) {
		return -1;
	}

	return pobjFas->recvData(nComType, stPartStatus);
}

int FasManager::sendData(int nFasID, int nComType/*zdst define data*/)
{
	Fas *pobjFas = getFasObj(nFasID);
	if(pobjFas == NULL) {
		return -1;
	}

	return pobjFas->sendData(nComType/*zdst define data*/);
}

int FasManager::getFasName(int nFasID, char *strName)
{
	int nArrayLen = ARRAY_SIZE(s_stFasInfo);
	for(int i = 0; i < nArrayLen; ++ i) {
		if(nFasID == s_stFasInfo[i].nID) {
			int nLen = strlen(s_stFasInfo[i].strName);
			strncpy(strName, s_stFasInfo[i].strName, nLen);
			return strlen(strName);
		}
	}

	return 0;
}

int FasManager::getFasList(vector<FasInfo>& stFasInfoList)
{
    int nLen = ARRAY_SIZE(s_stFasInfo);
    stFasInfoList.clear();
    for(int i = 0; i < nLen; ++ i){
        stFasInfoList.push_back(s_stFasInfo[i]);
    }

    return 0;
}
