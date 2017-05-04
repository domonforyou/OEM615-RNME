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

//BCH(15,11,1)����,11bits����15bits����===>>
int UnitBCHEncode(char *inbuf/*11+1bits in*/,char *outbuf/*15+1bits out*/);
//���ݿ�BCH���룬224bits����300bits����===>>
int DataBCHEncode(char *inbuf/*224+1bits in*/,char *outbuf/*300+1bits out*/);
//300bits���ݽ�֯����                  ===>>
int StringMingle(char *buf/*in 300+1bits out*/);
//����ת���ɶ����Ʋ����������         ===>>
int IntToComplementBin(int integer/*in*/,int len/*in������λ�����������*/,char *buf/*out*/);
//double����ת���ɶ��������
//int DoubleToInt(BDSRinexRecord **pbd/*in*/,BDSAlmanacRecord **pbdA/*in*/,int id/*in*/,BDSAllBinData *bdbd/*out*/);

int parse_oem615_bdslog(const char *file, int min_time_len, int min_sat_num);

#endif