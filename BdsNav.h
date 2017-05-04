#ifndef _BDSDATAPROCESS_H_
#define _BDSDATAPROCESS_H_
#include "common.h"

#define BDS_HEADER "#BDSRAWNAVSUBFRAMEA"
#define BDS_HEADER_SLICE 10
#define BDS_BODY_SLICE 5
#define BDS_HEX_NAV_LEN 56 //56*4=224 bit

typedef struct _bds_info_{
	int week;
	double tow;
	int subframe_id;
	bool isD1;
	char nav[64];
}BDS_INFO;

//BCH(15,11,1)编码,11bits进，15bits出；===>>
int UnitBCHEncode(char *inbuf/*11+1bits in*/,char *outbuf/*15+1bits out*/);
//数据块BCH编码，224bits进，300bits出；===>>
int DataBCHEncode(char *inbuf/*224+1bits in*/,char *outbuf/*300+1bits out*/);
//300bits数据交织函数                  ===>>
int StringMingle(char *buf/*in 300+1bits out*/);
//整数转换成二进制补码输出函数         ===>>
int IntToComplementBin(int integer/*in*/,int len/*in二进制位所需输出长度*/,char *buf/*out*/);
//double数据转换成定义的整数
//int DoubleToInt(BDSRinexRecord **pbd/*in*/,BDSAlmanacRecord **pbdA/*in*/,int id/*in*/,BDSAllBinData *bdbd/*out*/);

int parse_oem615_bdslog(const char *file, int min_time_len, int min_sat_num);

#endif