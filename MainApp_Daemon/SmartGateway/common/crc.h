#ifndef CRC_H
#define CRC_H


void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf);
void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf);
void InvertUint32(unsigned int *dBuf,unsigned int *srcBuf);

/***下面是一般会用到的crc校验，都在这****/
unsigned short CRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen);
unsigned short CRC16_CCITT_FALSE(unsigned char *puchMsg, unsigned int usDataLen);
unsigned short CRC16_XMODEM(unsigned char *puchMsg, unsigned int usDataLen);

unsigned short CRC16_X25(unsigned char *puchMsg, unsigned int usDataLen);
unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen);
unsigned short CRC16_IBM(unsigned char *puchMsg, unsigned int usDataLen);
unsigned short CRC16_MAXIM(unsigned char *puchMsg, unsigned int usDataLen);

unsigned short CRC16_USB(unsigned char *puchMsg, unsigned int usDataLen);
unsigned int CRC32(unsigned char *puchMsg, unsigned int usDataLen);

#endif // CRC_H
