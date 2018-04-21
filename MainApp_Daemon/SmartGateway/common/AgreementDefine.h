#ifndef __AGREEMENT_DEIFNE__
#define __AGREEMENT_DEIFNE__

enum FAS_ID{
    FAS_ID_NONE = 0,
    FAS_ID_JBQB_FS5101, //赋安JB-QB-FS5101
    FAS_ID_JBQT_GST5000, //海湾JB-QT-GST5000
    FAS_ID_JBTT_JBF11S, //北大青鸟JB-TT-JBF11S
    FAS_ID_JBQB_JBF5012, //北大青鸟JB-QB-JBF5012
    FAS_ID_JBQB_LN1010, //北大青鸟JB-QB-LN1010
    FAS_ID_Q100GZ2L_LA040, //泰和安Q100GZ2L-LA040
    FAS_ID_TB_TC3000,  //营口天成TB-TC3000
    FAS_ID_JBQB_OZH4800, //奥瑞娜JB-QB-OZH4800
    FAS_ID_JBLB_QH8000B, //杭州清华JB-LB-QH8000B
    FAS_ID_JB_3208B,    //松江飞繁JB-3208B
    FAS_ID_JBQB_TC3020, //营口天成JB-QB-TC3020
    FAS_ID_JBTB_242,    //泰和安JB-TB-242
    FAS_ID_JBLB_CA2000SZ, //成都安吉斯JB-LB-CA2000SZ
    FAS_ID_JB_QGL_9000,//泛海三江JB-QGL-9000
    FAS_ID_JBQB_GST500, //海湾JB-QB-GST500
    FAS_ID_JB_QBL_MN300, //泛海三江JB_QBL_MN300
    FAS_ID_JB_TB_JBF_11S, //北大青鸟JB-QBF-11S
    FAS_ID_JB_3208G, //上海松江飞繁JB_3208G
    FAS_ID_JBQG_GST5000, //海湾JB-QG-GST5000
    FAS_ID_JB_QT_OZH4800, //奥瑞娜JB-QT-OZH4800
    FAS_ID_JB_LG_QH8000,  //杭州清华JB-LG-QH8000
    FAS_ID_JB_TG_JBF_11SF, //北大青鸟JB-TG-JBF-11SF
    FAS_ID_JB_QGL_9000_PRINTER, //泛海三江JB-QGL-9000打印机
    FAS_ID_JB_SJFF3208,         //松江非凡电子_JB3208
    FAS_ID_JKB_193K,            //北大青鸟JKB_193K
    FAS_ID_JB_TB_JBF_11SF, //北大青鸟JB-TB-JBF-11SF
    FAS_ID_JB_TG_JBF_11S, //北大青鸟JB-TG-JBF-11S
    FAS_ID_GST200_MODBUS_RTU, //海湾MODBUS-RTU-GST200
    FAS_ID_GST5000_MODBUS_RTU, //海湾MODBUS-RTU-GST5000
    FAS_ID_TX_3607,//泰和安TX_3607
    FAS_ID_ESSER_E98,//ESSER_E98
    FAS_ID_ESSER_E98_RS232,//ESSER_E98_RS232
    FAS_ID_NUM,
};

enum SERVER_AGREEMENT_ID {
    SERVER_AGREEMENT_ID_GB = 0, //国标
    SERVER_AGREEMENT_ID_ZDST,
    SERVER_AGREEMENT_ID_NUM,
};

enum FAS_REQUEST_TYPE {
	FAS_REQUEST_INSPECTION = 0, //巡检
	FAS_REQUEST_RESET, //复位
	FAS_REQUEST_TIME,  //时间
	FAS_REQUEST_RECVEVENT, //接收事件
	FAS_REQUEST_SENDEVENT, //发送事件
	FAS_REQUEST_NONE,
};

enum DATA_TYPE
{
    //0 预留
    DATA_TYPE_UPLOAD_1 = 1,              //上传->设施[系统:状态]
    DATA_TYPE_UPLOAD_2,                  //上传->设施[部件:运行状态]
    DATA_TYPE_UPLOAD_3,                  //上传->设施[部件:模拟量值]
    DATA_TYPE_UPLOAD_4,                  //上传->设施[操作信息]
    DATA_TYPE_UPLOAD_5,                  //上传->设施[软件版本]
    DATA_TYPE_UPLOAD_6,                  //上传->设施[系统:配置情况]
    DATA_TYPE_UPLOAD_7,                  //上传->设施[部件:配置情况]
    DATA_TYPE_UPLOAD_8,                  //上传->设施[系统:时间]
    //预留9-20 (设施信息)
    DATA_TYPE_UPLOAD_21 = 21,            //上传->用户信息[传输装置:运行状态]
    //预留22-23
    DATA_TYPE_UPLOAD_24 = 24,            //上传->用户信息[传输装置:操作信息]
    DATA_TYPE_UPLOAD_25,                 //上传->用户信息[传输装置:软件版本]
    DATA_TYPE_UPLOAD_26,                 //上传->用户信息[传输装置:配置情况]
    //预留27
    DATA_TYPE_UPLOAD_28 = 28,            //上传->用户信息[传输装置:系统时间]
    //预留29-40 (用户信息)
    //预留41-60 (控制信息)

    DATA_TYPE_READ_61 = 61,              //读->设施[系统:状态]
    DATA_TYPE_READ_62,                   //读->设施[部件:运行状态]
    DATA_TYPE_READ_63,                   //读->设施[部件:模拟量值]
    DATA_TYPE_READ_64,                   //读->设施[操作信息]
    DATA_TYPE_READ_65,                   //读->设施[软件版本]
    DATA_TYPE_READ_66,                   //读->设施[系统:配置情况]
    DATA_TYPE_READ_67,                   //读->设施[部件:配置情况]
    DATA_TYPE_READ_68,                   //读->设施[系统:时间]
    //预留69-80
    DATA_TYPE_READ_81 = 81,              //读->用户信息[传输装置:运行状态]
    //预留82-83
    DATA_TYPE_READ_84 = 84,              //读->用户信息[传输装置:操作信息]
    DATA_TYPE_READ_85,                   //读->用户信息[传输装置:软件版本]
    DATA_TYPE_READ_86,                   //读->用户信息[传输装置:配置情况]
    //预留87
    DATA_TYPE_READ_88 = 88,              //读->用户信息[传输装置:系统时间]

    DATA_TYPE_READ_89 = 89,              //初始化->用户信息[传输装置]
    DATA_TYPE_READ_90,                   //同步->用户信息[传输装置:时间]
    DATA_TYPE_READ_91,                   //查岗命令
    //预留92-127
    //用户定义128-254
    DATA_TYPE_HEARTBEAT_190 = 190,
    DATA_TYPE_MAX = 256
};

enum SIMULA_TYPE
{
    SIMULA_TYPE_0 = 0,              //未用
    SIMULA_TYPE_EVENT_COUNT,        //事件计数     0~3200      (1件)
    SIMULA_TYPE_HEIGHT,             //高度        0~320       (0.01m)
    SIMULA_TYPE_TEMPERTURE,         //温度        -273~+3200  (0.1℃)
    SIMULA_TYPE_PRESSURE_MP,        //压力(兆)        0~3200      (0.1MPa)
    SIMULA_TYPE_PRESSURE_KP,        //压力(千)        0~3200      (0.1kPa)
    SIMULA_TYPE_GAS_CONCENTRATION,  //气体浓度     0~100       (0.1%LEL)
    SIMULA_TYPE_TIME,               //时间        0~32000     (1s)
    SIMULA_TYPE_VOLTAGE,            //电压        0~3200      (0.1V)
    SIMULA_TYPE_ELECTRICITY,        //电流        0~3200      (0.1A)
    SIMULA_TYPE_FLOW,               //流量        0~3200      (0.1L/s)
    SIMULA_TYPE_AIR_QUANTITY,       //风量        0~3200      (0.1m^3/min)
    SIMULA_TYPE_WIND_SPEED,         //风速        0~20        (1m/s)
    //预留13-127
    //用户定义128-255
    SIMULA_TYPE_MAX = 256
};
//系统类型
enum SYS_TYPE {
    SYS_TYPE_CURRENCY = 0,                          //通用
    SYS_TYPE_FIRE_ALARM_SYSTEM,                     //火灾报警系统
    //2-9 remain
    SYS_TYPE_FIRE_LINKAGE_CONTROLLER = 10,          //消防联动控制器
    SYS_TYPE_FIRE_HYDRANT_SYSTEM,                   //消火栓系统
    SYS_TYPE_ATUOMATIC_SPRINKLER_SYSTEM,            //自动喷水灭火系统
    SYS_TYPE_GAS_EXTINGUISHING_SYSTEM,              //气体灭火系统
    SYS_TYPE_WATER_SPRAY_EXTINGUISHING_SYSTEM_PUMP, //水喷雾灭火系统(泵启动)
    SYS_TYPE_WATER_SPRAY_EXTINGUISHING_SYSTEM_PRESSURE, //水喷雾灭火系统(压力启动)
    SYS_TYPE_FOAM_EXTINGUISHING_SYSTEM,             //泡沫灭火系统
    SYS_TYPE_DRY_POWDER_EXTINGUISHING_SYSTEM,       //干粉灭火系统
    SYS_TYPE_SMOKE_PREVENTION_SYSTEM,               //防烟排烟系统
    SYS_TYPE_FIRE_DOOR_AND_CURTAIN_SYSTEM,          //防火门及卷帘系统
    SYS_TYPE_FIRE_ELEVATOR,                         //消防电梯
    SYS_TYPE_FIRE_EMERGENCY_BROADCAST,              //消防应急广播
    SYS_TYPE_FIRE_LIGHTING_AND_INDICATION_SYSTEM,   //消防应急照明和疏散指示系统
    SYS_TYPE_POWER,                                 //消防电源
    SYS_TYPE_TELEPHONE,                             //消防电话
    //25-127 remain
    //128-255 user define
};

//部件类型
enum PARTS_TYPE
{
    PARTS_TYPE_CURRENCY = 0,                            //通用
    PARTS_TYPE_ALARM_CONTROLLER,                        //火灾报警控制器
    //2-9 remain
    PARTS_TYPE_COMBUSTIBLE_GAS_DETECTOR = 10,           //可燃气体探测器
    PARTS_TYPE_POINT_COMBUSTIBLE_GAS_DETECTOR,          //点型可燃气体探测器
    PARTS_TYPE_INDEPENDENT_COMBUSTIBLE_GAS_DETECTOR,    //独立式可燃气体探测器
    PARTS_TYPE_LINE_COMBUSTIBLE_GAS_DETECTOR,           //线型可燃气体探测器
    //14-15 remain
    PARTS_TYPE_ELECTRICAL_FIRE_MONITORING_ALARMER = 16,                 //电气火灾监控报警器
    PARTS_TYPE_RESIDUAL_ELECTRIC_FIRE_MONITORING_DETECTOR,              //剩余电流式电气火灾监控探测器
    PARTS_TYPE_TEMPERATURE_MEASURING_ELECTRIC_FIRE_MONITORING_DETECTOR, //测温式电气火灾监控探测器
    //19-20 remain
    PARTS_TYPE_DETECTION_CIRCUIT = 21,  //探测回路
    PARTS_TYPE_FIRE_DISPLAY_PANEL,      //火灾显示盘
    PARTS_TYPE_MANUAL_FIRE_ALARM_BTN,   //手动火灾报警按钮
    PARTS_TYPE_FIRE_HYDRANT_BTN,        //消火栓按钮
    PARTS_TYPE_FIRE_DETECTOR,           //火灾探测器
    //26-29 remain
    PARTS_TYPE_HEAT_FIRE_DETECTOR = 30,     //感温火灾探测器
    PARTS_TYPE_POINT_HEAT_FIRE_DETECTOR,    //点型感温火灾探测器
    PARTS_TYPE_POINT_HEAT_FIRE_DETECTOR_S,  //点型感温火灾探测器(S型)
    PARTS_TYPE_POINT_HEAT_FIRE_DETECTOR_R,  //点型感温火灾探测器(R型)
    PARTS_TYPE_LINE_HEAT_FIRE_DETECTOR,     //线型感温火灾探测器
    PARTS_TYPE_LINE_HEAT_FIRE_DETECTOR_S,   //线型感温火灾探测器(S型)
    PARTS_TYPE_LINE_HEAT_FIRE_DETECTOR_R,   //线型感温火灾探测器(R型)
    PARTS_TYPE_FIBER_TEMPERATURE_FIRE_DETECTOR, //光纤感温火灾探测器
    //38 remain
    //39 remain
    PARTS_TYPE_SMOKE_FIRE_DETECTOR = 40,                //感烟火灾探测器
    PARTS_TYPE_POINT_ION_SMOKE_FIRE_DETECTOR,           //点型离子感烟火灾探测器
    PARTS_TYPE_POINT_PHOTOELECTRIC_SMOKE_FIRE_DETECTOR, //点型光电感烟火灾探测器
    PARTS_TYPE_LINE_BEAM_SMOKE_FIRE_DETECTOR,           //点型光束感烟火灾探测器
    PARTS_TYPE_ASPIRATED_SMOKE_FIRE_DETECTOR,           //吸气式感烟火灾探测器
    //45-49 remain
    PARTS_TYPE_COMPOUND_FIRE_DETECTOR = 50,                         //复合式火灾探测器
    PARTS_TYPE_COMPOUND_SMOKE_TEMPERATURE_FIRE_DETECTOR,            //复合式感烟感温火灾探测器
    PARTS_TYPE_COMPOUND_PHOTOSENSITIVE_TEMPERATURE_FIRE_DETECTOR,   //复合式感光感温火灾探测器
    PARTS_TYPE_COMPOUND_PHOTOSENSITIVE_SMOKE_FIRE_DETECTOR,         //复合式感光感烟火灾探测器
    //54-59 remain
    //60 remain
    PARTS_TYPE_ULTRAVIOLET_FLAME_DETECTOR = 61, //紫外火焰探测器
    PARTS_TYPE_INFRARED_FLAME_DETECTOR,         //红色外火焰探测器
    //63-68 remain
    PARTS_TYPE_PHOTOSENSITIVE_FIRE_DETECTOR = 69, //感光火灾探测器
    //70-73 remain
    PARTS_TYPE_GAS_DETECTOR = 74, //气体探测器
    //75-77 remain
    PARTS_TYPE_IMAGE_CAMERA_FIRE_DETECTOR = 78, //图像摄像方式火灾探测器
    PARTS_TYPE_ACOUSTIC_FIRE_DETECTOR,          //感声火灾探测器
    //80 remain
    PARTS_TYPE_GAS_FIRE_CTRL_CONTROLLER = 81,       //气体灭火控制器
    PARTS_TYPE_FIRE_ELECTRICAL_CONTROL_DEVICE,      //消防电气控制装置
    PARTS_TYPE_FIRE_CTRL_ROOM_IMG_DISPLAY_DEVICE,   //消防控制室图像显示装置
    PARTS_TYPE_MODULE,          //模块
    PARTS_TYPE_INPUT_MODULE,   //输入模块
    PARTS_TYPE_OUTPUT_MODULE,   //输出模块
    PARTS_TYPE_INPUT_OUTPUT_MODULE,   //输入/输出模块
    PARTS_TYPE_RELAY_MODULE,//中继模块
    //89-90 ramain
    PARTS_TYPE_FIRE_PUMP = 91, //消防水泵
    PARTS_TYPE_FIRE_WATER_TANK,//消防水箱
    //93-94 remain
    PARTS_TYPE_SPRAY_PUMP = 95,     //喷淋泵
    PARTS_TYPE_WATER_FLOW_INDICATOR,//水流指示器
    PARTS_TYPE_SIGNAL_VALVE,        //信号阀
    PARTS_TYPE_PRALARM_VALVE,       //报警阀
    PARTS_TYPE_PRESSURE_SWITCH,     //压力开关
    //100 remain
    PARTS_TYPE_VALVE_ACTUATOR = 101,    //阀驱动装置
    PARTS_TYPE_FIRE_DOOR,               //防火门
    PARTS_TYPE_FIRE_VAVLE,              //防火阀
    PARTS_TYPE_VENTILATION_CONDITIONER, //通风空调
    PARTS_TYPE_FOAM_LIQUID_PUMP,        //泡沫液泵
    PARTS_TYPE_NETWORK_SOLENOID_VALVE,  //管网电磁阀
    //107-110 remain
    PARTS_TYPE_SMOKE_EXHAUST_FAN = 111,   //防烟排烟风机
    //112 remain
    PARTS_TYPE_EXHAUST_FIRE_DAMPER = 113,   //排烟防火阀
    PARTS_TYPE_OFF_AIR_SUPPLY,              //常闭送风口
    PARTS_TYPE_EXHAUST_PORT,                //排烟口
    PARTS_TYPE_ELEC_CTRL_SMOKE_WALL,        //电控挡烟垂壁
    PARTS_TYPE_FIRE_SHUTTER_CONTROLLER,     //防火卷帘控制器
    PARTS_TYPE_FIRE_DOOR_MONITOR,           //防火门监控器
    //119-120 remain
    PARTS_TYPE_ALARM_DEVICE,     //警报装置
    //122-127 remain
    //128-255 user define
};

enum COMMAND_TYPE {
    //0 remain
    COMMAND_TYPE_CTRL = 1,  //控制命令
    COMMAND_TYPE_SEND,      //发送数据
    COMMAND_TYPE_CONFIRM,   //确认
    COMMAND_TYPE_REQUEST,   //请求
    COMMAND_TYPE_RESPONSE,  //应答
    COMMAND_TYPE_DENY,      //否认
    //7-127 预留
    //128-255 用户自定义
};

#endif //__AGREEMENT_DEIFNE__
