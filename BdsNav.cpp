#include "BdsNav.h"
#include "Gnss_nav.h"
#include "util.h"

static int cur_ok_sat_nums = 0;
map<int, BDS_INFO> bds_infos;

//BCH(15,11,1)编码,11bits进，15bits出；
int UnitBCHEncode(char *inbuf/*11+1bits in*/,char *outbuf/*15+1bits out*/)
{
	char *pbuf = NULL;
	int i=0, transbit=0, ret = 0, D3=0, D2=0, D1=0, D0=0;
	char bufD[5] = {0};
	if(inbuf==NULL || outbuf==NULL)//buf:11bits,str:15bits
	{
		ret = -1;
		printf("UnitBCHEncode:inbuf || outbuf error:%d\n",ret);
		return ret;
	}
	pbuf = inbuf;
	for(i=0;i<11;i++)
	{
		if(pbuf[i] != 0 && pbuf[i] != 1)
		{
			ret = -2;
			printf("UnitBCHEncode:pbuf[%d] error:\n",ret);
			return ret;
		}
	}
	for(i=0;i<11;i++)
	{
		transbit = pbuf[i] + (D3%2);
		D3 = D2 % 2;
		D2 = D1 % 2;
		D1 = (D0%2) + (transbit%2);
		D0 = transbit % 2;
	}
	for(i=0;i<11;i++){
		pbuf[i]+=48;
	}
	strcpy(outbuf,pbuf);
	bufD[0] = D3%2 + 48;
    bufD[1] = D2%2 + 48;
	bufD[2] = D1%2 + 48;
    bufD[3] = D0%2 + 48;
	strcat(outbuf,bufD);
	return 0;
}
//数据块BCH编码，224bits进，300bits出；
int DataBCHEncode(char *inbuf/*224+1bits in*/,char *outbuf/*300+1bits out*/)
{ 
	char tbuf[16] = {0};
	int i = 0, ret = 0;
	char operbuf[19][12] = {0};
	char destbuf[19][16] = {0};
	char *pbuf = NULL;
	if(inbuf==NULL || outbuf==NULL)
	{
		ret = -1;
		printf("DataBCHEncode:inbuf || outbuf error:%d\n",ret);
		return ret;
	}
	//do nothing with the first 15 bit
	for(i=0;i<15;i++)
		tbuf[i]=inbuf[i]+48;

	pbuf = &inbuf[15];
	for(i=0;i<19;i++)
	{
		strncpy(operbuf[i],pbuf,11);
		pbuf = pbuf + 11;
	}
	for(i=0;i<19;i++)
	{
		UnitBCHEncode(operbuf[i],destbuf[i]);
	}
	strcpy(outbuf,tbuf);
	for(i=0;i<19;i++)
	{
		strcat(outbuf,destbuf[i]);
	}
	return 0;
}
//300bits数据交织函数
int StringMingle(char *buf/*in 300+1bits out*/)
{
	char pbuf[2][136] = {0};
	char frontbuf[31] = {0};
	char destbuf[271] = {0};
	int i = 0, j = 0, ret = 0;
	char *pMove = NULL;
	if(buf == NULL || strlen(buf) != 300)
	{
		ret = -1;
		printf("StringMingle:buf || strlen(buf) error:%d\n",ret);
		return ret;
	}
	strncpy(frontbuf,buf,30);
	pMove = &buf[30];
	for(i=0;i<strlen(buf)/15-2/*18次循环*/;i++)
	{
		if(i%2==0)
		{
			for(j=0;j<15;j++)
			{
				pbuf[0][j+(i/2)*15] = *(pMove+j); 
			}
		}
		else if(i%2==1)
		{
			for(j=0;j<15;j++)
			{
				pbuf[1][j+((i-1)/2)*15] = *(pMove+j); 
			}
		}
		else
		{
			ret = -2;
			printf("StringMingle: error:%d\n",ret);
			return ret;
		}
		pMove = pMove + 15;
	}
	for(i=0;i<sizeof(destbuf)-1;i++)
	{
		if(i%2==0)
		{
			destbuf[i] = pbuf[0][i/2];
		}
		else if(i%2==1)
		{
			destbuf[i] = pbuf[1][(i-1)/2];
		}
		else
		{
			ret = -3;
			printf("StringMingle: error:%d\n",ret);
			return ret;
		}
	}
	memset(buf,0,sizeof(buf));
	strcpy(buf,frontbuf);
	strcat(buf,destbuf);
	return 0;
}
//整数转换成二进制补码输出函数
int IntToComplementBin(int integer/*in*/,int len/*in二进制位所需输出长度*/,char *buf/*out*/)
{
	int tmp = 0, i = 0, n = 0, ret = 0;
	char tbuf[33]={0};
	tmp = integer;
	if(buf==NULL)
	{
		ret = -1;
		printf("IntToComplementBin:buf error:%d\n",ret);
		return ret;
	}
	for(i=len-1;i>=0;i--)
	{		
		n = (tmp>>i) & 1;
		tbuf[len-1-i] = n + 48;
	}
	strcpy(buf,tbuf);
	return 0;       
}
//double数据转换成定义的整数
// in in out
/*int DoubleToInt(BDSRinexRecord **pbd,BDSAlmanacRecord **pbdA,int id,BDSAllBinData *pbdbd)
{
	BDSAllBinData bdbd = {0};
	BDSAlmanacsBinData ex_AlmaData = {0};
	int tmp = 0, j = 0, WNa = 0, pos;
	unsigned long tmpSqrtA = 0;
	char str[179] = {0};
	if(pbd==NULL || pbdbd==NULL)
	{
		printf("DoubleToInt error:\n");
		return -1;
	}
	for(j=0;j<178;j++)
	{
		str[j] = '0';
	}
	//---星历数据处理---；
	strcpy(bdbd.Pre,"11100010010");
	strcpy(bdbd.Rev,"0000");
	strcpy(bdbd.EpheData.afa0,"00000000");
	strcpy(bdbd.EpheData.afa1,"00000000");
	strcpy(bdbd.EpheData.afa2,"00000000");
	strcpy(bdbd.EpheData.afa3,"00000000");
	strcpy(bdbd.EpheData.beita0,"00000000");
	strcpy(bdbd.EpheData.beita1,"00000000");
	strcpy(bdbd.EpheData.beita2,"00000000");
	strcpy(bdbd.EpheData.beita3,"00000000");
	strcpy(bdbd.EpheData.AODC,"00000");
	strcpy(bdbd.EpheData.AODE,"00000"); 
	if(pbd[id]->Data[0]<0) tmp = (int)(pbd[id]->Data[0]*pow(2.0,33) - 0.5);
	else if(pbd[id]->Data[0]>0) tmp = (int)(pbd[id]->Data[0]*pow(2.0,33) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,24,bdbd.EpheData.a0);

	if(pbd[id]->Data[1]<0) tmp = (int)(pbd[id]->Data[1]*pow(2.0,50) - 0.5);
	else if(pbd[id]->Data[1]>0) tmp = (int)(pbd[id]->Data[1]*pow(2.0,50) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,22,bdbd.EpheData.a1);

	if(pbd[id]->Data[2]<0) tmp = (int)(pbd[id]->Data[2]*pow(2.0,66) - 0.5);
	else if(pbd[id]->Data[2]>0) tmp = (int)(pbd[id]->Data[2]*pow(2.0,66) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,11,bdbd.EpheData.a2);

	if(pbd[id]->Data[3]<0) tmp = (int)(pbd[id]->Data[3] - 0.5);
	else if(pbd[id]->Data[3]>0) tmp = (int)(pbd[id]->Data[3] + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,5,bdbd.EpheData.IDOE);

	if(pbd[id]->Data[4]<0) tmp = (int)(pbd[id]->Data[4]*pow(2.0,6) - 0.5);
	else if(pbd[id]->Data[4]>0) tmp = (int)(pbd[id]->Data[4]*pow(2.0,6) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,18,bdbd.EpheData.Crs);

	if(pbd[id]->Data[5]<0) tmp = (int)(pbd[id]->Data[5]*pow(2.0,43)/Pi - 0.5);
	else if(pbd[id]->Data[5]>0) tmp = (int)(pbd[id]->Data[5]*pow(2.0,43)/Pi + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,16,bdbd.EpheData.DTn);

	if(pbd[id]->Data[6]<0) tmp = (int)(pbd[id]->Data[6]*pow(2.0,31)/Pi - 0.5);
	else if(pbd[id]->Data[6]>0) tmp = (int)(pbd[id]->Data[6]*pow(2.0,31)/Pi + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,32,bdbd.EpheData.Mo);

	if(pbd[id]->Data[7]<0) tmp = (int)(pbd[id]->Data[7]*pow(2.0,31) - 0.5);
	else if(pbd[id]->Data[7]>0) tmp = (int)(pbd[id]->Data[7]*pow(2.0,31) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,18,bdbd.EpheData.Cuc);

	if(pbd[id]->Data[8]<0) tmp = (int)(pbd[id]->Data[8]*pow(2.0,33) - 0.5);
	else if(pbd[id]->Data[8]>0) tmp = (int)(pbd[id]->Data[8]*pow(2.0,33) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,32,bdbd.EpheData.e);
		
	if(pbd[id]->Data[9]<0) tmp = (int)(pbd[id]->Data[9]*pow(2.0,31) - 0.5);
	else if(pbd[id]->Data[9]>0) tmp = (int)(pbd[id]->Data[9]*pow(2.0,31) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,18,bdbd.EpheData.Cus);

	if(pbd[id]->Data[10]<0) tmpSqrtA = (unsigned long)(pbd[id]->Data[10]*pow(2.0,19) - 0.5);
	else if(pbd[id]->Data[10]>0) tmpSqrtA = (unsigned long)(pbd[id]->Data[10]*pow(2.0,19) + 0.5);
	else tmpSqrtA = 0;
	IntToComplementBin(tmpSqrtA,32,bdbd.EpheData.sqrtA);

	if(pbd[id]->Data[11]<0) tmp = (int)(pbd[id]->Data[11]*pow(2.0,-3) - 0.5);
	else if(pbd[id]->Data[11]>0) tmp = (int)(pbd[id]->Data[11]*pow(2.0,-3) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,17,bdbd.EpheData.Toc);
	if(pbd[id]->Data[11]<0) tmp = (int)(pbd[id]->Data[11]*pow(2.0,-3) - 0.5);
	else if(pbd[id]->Data[11]>0) tmp = (int)(pbd[id]->Data[11]*pow(2.0,-3) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,17,bdbd.EpheData.Toe);
		
	if(pbd[id]->Data[12]<0) tmp = (int)(pbd[id]->Data[12]*pow(2.0,31) - 0.5);
	else if(pbd[id]->Data[12]>0) tmp = (int)(pbd[id]->Data[12]*pow(2.0,31) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,18,bdbd.EpheData.Cic);

	if(pbd[id]->Data[13]<0) tmp = (int)(pbd[id]->Data[13]*pow(2.0,31)/Pi - 0.5);
	else if(pbd[id]->Data[13]>0) tmp = (int)(pbd[id]->Data[13]*pow(2.0,31)/Pi + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,32,bdbd.EpheData.OMEGA0);

	if(pbd[id]->Data[14]<0) tmp = (int)(pbd[id]->Data[14]*pow(2.0,31) - 0.5);
	else if(pbd[id]->Data[14]>0) tmp = (int)(pbd[id]->Data[14]*pow(2.0,31) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,18,bdbd.EpheData.Cis);

	if(pbd[id]->Data[15]<0) tmp = (int)(pbd[id]->Data[15]*pow(2.0,31)/Pi - 0.5);
	else if(pbd[id]->Data[15]>0) tmp = (int)(pbd[id]->Data[15]*pow(2.0,31)/Pi + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,32,bdbd.EpheData.i0);

	if(pbd[id]->Data[16]<0) tmp = (int)(pbd[id]->Data[16]*pow(2.0,6) - 0.5);
	else if(pbd[id]->Data[16]>0) tmp = (int)(pbd[id]->Data[16]*pow(2.0,6) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,18,bdbd.EpheData.Crc);

	if(pbd[id]->Data[17]<0) tmp = (int)(pbd[id]->Data[17]*pow(2.0,31)/Pi - 0.5);
	else if(pbd[id]->Data[17]>0) tmp = (int)(pbd[id]->Data[17]*pow(2.0,31)/Pi + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,32,bdbd.EpheData.w);

	if(pbd[id]->Data[18]<0) tmp = (int)(pbd[id]->Data[18]*pow(2.0,43)/Pi - 0.5);
	else if(pbd[id]->Data[18]>0) tmp = (int)(pbd[id]->Data[18]*pow(2.0,43)/Pi + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,24,bdbd.EpheData.OMEGADOT);

	if(pbd[id]->Data[19]<0) tmp = (int)(pbd[id]->Data[19]*pow(2.0,43)/Pi - 0.5);
	else if(pbd[id]->Data[19]>0) tmp = (int)(pbd[id]->Data[19]*pow(2.0,43)/Pi + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,14,bdbd.EpheData.IDOT);

	if(pbd[id]->Data[21]<0) tmp = (int)(pbd[id]->Data[21] - 0.5);
	else if(pbd[id]->Data[21]>0) tmp = (int)(pbd[id]->Data[21] + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,13,bdbd.EpheData.WN);
	WNa = tmp%256;

	if(pbd[id]->Data[23]<0) tmp = (int)(pbd[id]->Data[23] - 0.5);
	else if(pbd[id]->Data[23]>0) tmp = (int)(pbd[id]->Data[23] + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,4,bdbd.EpheData.URAI);

	if(pbd[id]->Data[24]<0) tmp = (int)(pbd[id]->Data[24] - 0.5);
	else if(pbd[id]->Data[24]>0) tmp = (int)(pbd[id]->Data[24] + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,1,bdbd.EpheData.SatH1);

	if(pbd[id]->Data[25]<0) tmp = (int)(pbd[id]->Data[25]*pow(10.0,10) - 0.5);
	else if(pbd[id]->Data[25]>0) tmp = (int)(pbd[id]->Data[25]*pow(10.0,10) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,10,bdbd.EpheData.TGD1);

	if(pbd[id]->Data[26]<0) tmp = (int)(pbd[id]->Data[26]*pow(10.0,10) - 0.5);
	else if(pbd[id]->Data[26]>0) tmp = (int)(pbd[id]->Data[26]*pow(10.0,10) + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,10,bdbd.EpheData.TGD2);

	if(pbd[id]->Data[27]<0) tmp = (int)(pbd[id]->Data[27] - 0.5);
	else if(pbd[id]->Data[27]>0) tmp = (int)(pbd[id]->Data[27] + 0.5);
	else tmp = 0;
	IntToComplementBin(tmp,20,bdbd.EpheData.SOW);
	//---
	//---历书数据处理---：
	if(pbdA != NULL)
	{
		for(j=0;j<30;j++)
		{
			if(pbdA[j]->Data[0]<0) tmp = (int)(pbdA[j]->Data[0]*pow(2.0,-12) - 0.5);
			else if(pbdA[j]->Data[0]>0) tmp = (int)(pbdA[j]->Data[0]*pow(2.0,-12) + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,8,bdbd.AlmaData[j].Toa);

			if(pbdA[j]->Data[1]<0) tmpSqrtA = (unsigned long)(pbdA[j]->Data[1]*pow(2.0,11) - 0.5);
			else if(pbdA[j]->Data[1]>0) tmpSqrtA = (unsigned long)(pbdA[j]->Data[1]*pow(2.0,11) + 0.5);
			else tmpSqrtA = 0;
			IntToComplementBin(tmpSqrtA,24,bdbd.AlmaData[j].sqrtA);

			if(pbdA[j]->Data[2]<0) tmp = (int)(pbdA[j]->Data[2]*pow(2.0,21) - 0.5);
			else if(pbdA[j]->Data[2]>0) tmp = (int)(pbdA[j]->Data[2]*pow(2.0,21) + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,17,bdbd.AlmaData[j].e);

			if(pbdA[j]->Data[3]<0) tmp = (int)(pbdA[j]->Data[3]*pow(2.0,23) - 0.5);
			else if(pbdA[j]->Data[3]>0) tmp = (int)(pbdA[j]->Data[3]*pow(2.0,23) + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,24,bdbd.AlmaData[j].w);

			if(pbdA[j]->Data[4]<0) tmp = (int)(pbdA[j]->Data[4]*pow(2.0,23)/Pi - 0.5);
			else if(pbdA[j]->Data[4]>0) tmp = (int)(pbdA[j]->Data[4]*pow(2.0,23)/Pi + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,24,bdbd.AlmaData[j].Mo);

			if(pbdA[j]->Data[5]<0) tmp = (int)(pbdA[j]->Data[5]*pow(2.0,23)/Pi - 0.5);
			else if(pbdA[j]->Data[5]>0) tmp = (int)(pbdA[j]->Data[5]*pow(2.0,23)/Pi + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,24,bdbd.AlmaData[j].OMEGA0);

			if(pbdA[j]->Data[6]<0) tmp = (int)(pbdA[j]->Data[6]*pow(2.0,38)/Pi - 0.5);
			else if(pbdA[j]->Data[6]>0) tmp = (int)(pbdA[j]->Data[6]*pow(2.0,38)/Pi + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,17,bdbd.AlmaData[j].OMEGADOT);
	
			if(pbdA[j]->Data[7]<0) tmp = (int)(pbdA[j]->Data[7]*pow(2.0,19)/Pi - 0.5);
			else if(pbdA[j]->Data[7]>0) tmp = (int)(pbdA[j]->Data[7]*pow(2.0,19)/Pi + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,16,bdbd.AlmaData[j].detai);

			if(pbdA[j]->Data[8]<0) tmp = (int)(pbdA[j]->Data[8]*pow(2.0,20)/Pi - 0.5);
			else if(pbdA[j]->Data[8]>0) tmp = (int)(pbdA[j]->Data[8]*pow(2.0,20)/Pi + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,11,bdbd.AlmaData[j].a0);

			if(pbdA[j]->Data[9]<0) tmp = (int)(pbdA[j]->Data[9]*pow(2.0,38) - 0.5);
			else if(pbdA[j]->Data[9]>0) tmp = (int)(pbdA[j]->Data[9]*pow(2.0,38) + 0.5);
			else tmp = 0;
			IntToComplementBin(tmp,11,bdbd.AlmaData[j].a1);
		}
	}
	else
	{
		pos = id%24;
		for(j=0;j<30;j++)
		{
			if(j>=0 && j<=12)
			{
				if(pbd[pos+24*j]->Data[11]<0) tmp = ((int)(pbd[pos+24*j]->Data[11]*pow(2.0,-12) - 0.5))%256;
				else if(pbd[pos+24*j]->Data[11]>0) tmp = ((int)(pbd[pos+24*j]->Data[11]*pow(2.0,-12) + 0.5))%256;
				else tmp = 0;
				IntToComplementBin(tmp,8,bdbd.AlmaData[j].Toa);

				if(pbd[pos+24*j]->Data[10]<0) tmpSqrtA = (unsigned long)(pbd[pos+24*j]->Data[10]*pow(2.0,11) - 0.5);
				else if(pbd[pos+24*j]->Data[10]>0) tmpSqrtA = (unsigned long)(pbd[pos+24*j]->Data[10]*pow(2.0,11) + 0.5);
				else tmpSqrtA = 0;
				IntToComplementBin(tmpSqrtA,24,bdbd.AlmaData[j].sqrtA);

				if(pbd[pos+24*j]->Data[8]<0) tmp = (int)(pbd[pos+24*j]->Data[8]*pow(2.0,21) - 0.5);
				else if(pbd[pos+24*j]->Data[8]>0) tmp = (int)(pbd[pos+24*j]->Data[8]*pow(2.0,21) + 0.5);
				else tmp = 0;
				IntToComplementBin(tmp,17,bdbd.AlmaData[j].e);

				if(pbd[pos+24*j]->Data[17]<0) tmp = (int)(pbd[pos+24*j]->Data[17]*pow(2.0,23) - 0.5);
				else if(pbd[pos+24*j]->Data[17]>0) tmp = (int)(pbd[pos+24*j]->Data[17]*pow(2.0,23) + 0.5);
				else tmp = 0;
				IntToComplementBin(tmp,24,bdbd.AlmaData[j].w);

				if(pbd[pos+24*j]->Data[6]<0) tmp = (int)(pbd[pos+24*j]->Data[6]*pow(2.0,23)/Pi - 0.5);
				else if(pbd[pos+24*j]->Data[6]>0) tmp = (int)(pbd[pos+24*j]->Data[6]*pow(2.0,23)/Pi + 0.5);
				else tmp = 0;
				IntToComplementBin(tmp,24,bdbd.AlmaData[j].Mo);

				if(pbd[pos+24*j]->Data[13]<0) tmp = (int)(pbd[pos+24*j]->Data[13]*pow(2.0,23)/Pi - 0.5);
				else if(pbd[pos+24*j]->Data[13]>0) tmp = (int)(pbd[pos+24*j]->Data[13]*pow(2.0,23)/Pi + 0.5);
				else tmp = 0;
				IntToComplementBin(tmp,24,bdbd.AlmaData[j].OMEGA0);

				if(pbd[pos+24*j]->Data[18]<0) tmp = (int)(pbd[pos+24*j]->Data[18]*pow(2.0,38)/Pi - 0.5);
				else if(pbd[pos+24*j]->Data[18]>0) tmp = (int)(pbd[pos+24*j]->Data[18]*pow(2.0,38)/Pi + 0.5);
				else tmp = 0;
				IntToComplementBin(tmp,17,bdbd.AlmaData[j].OMEGADOT);
	
				if(j>=0 && j<=4)
				{
					if(pbd[pos+24*j]->Data[15]<0) tmp = (int)(pbd[pos+24*j]->Data[15]*pow(2.0,19)/Pi - 0.5);
					else if(pbd[pos+24*j]->Data[15]>0) tmp = (int)(pbd[pos+24*j]->Data[15]*pow(2.0,19)/Pi + 0.5);
					else tmp = 0;
					IntToComplementBin(tmp,16,bdbd.AlmaData[j].detai);
				}
				else
				{
					if(pbd[pos+24*j]->Data[15]<0) tmp = (int)((pbd[pos+24*j]->Data[15]/Pi-0.3)*pow(2.0,19) - 0.5);
					else if(pbd[pos+24*j]->Data[15]>0) tmp = (int)((pbd[pos+24*j]->Data[15]/Pi-0.3)*pow(2.0,19) + 0.5);
					else tmp = 0;
					IntToComplementBin(tmp,16,bdbd.AlmaData[j].detai);
				}

				if(pbd[pos+24*j]->Data[0]<0) tmp = (int)(pbd[pos+24*j]->Data[0]*pow(2.0,20)/Pi - 0.5);
				else if(pbd[pos+24*j]->Data[0]>0) tmp = (int)(pbd[pos+24*j]->Data[0]*pow(2.0,20)/Pi + 0.5);
				else tmp = 0;
				IntToComplementBin(tmp,11,bdbd.AlmaData[j].a0);

				if(pbd[pos+24*j]->Data[1]<0) tmp = (int)(pbd[pos+24*j]->Data[1]*pow(2.0,38) - 0.5);
				else if(pbd[pos+24*j]->Data[1]>0) tmp = (int)(pbd[pos+24*j]->Data[1]*pow(2.0,38) + 0.5);
				else tmp = 0;
				IntToComplementBin(tmp,11,bdbd.AlmaData[j].a1);
			}
			else
			{
				strncpy(bdbd.AlmaData[j].Toa,str,sizeof(bdbd.AlmaData[j].Toa)-1);
				strncpy(bdbd.AlmaData[j].sqrtA,str,sizeof(bdbd.AlmaData[j].sqrtA)-1);
				strncpy(bdbd.AlmaData[j].e,str,sizeof(bdbd.AlmaData[j].e)-1);
				strncpy(bdbd.AlmaData[j].w,str,sizeof(bdbd.AlmaData[j].w)-1);
				strncpy(bdbd.AlmaData[j].Mo,str,sizeof(bdbd.AlmaData[j].Mo)-1);
				strncpy(bdbd.AlmaData[j].OMEGA0,str,sizeof(bdbd.AlmaData[j].OMEGA0)-1);
				strncpy(bdbd.AlmaData[j].OMEGADOT,str,sizeof(bdbd.AlmaData[j].OMEGADOT)-1);
				strncpy(bdbd.AlmaData[j].detai,str,sizeof(bdbd.AlmaData[j].detai)-1);
				strncpy(bdbd.AlmaData[j].a0,str,sizeof(bdbd.AlmaData[j].a0)-1);
				strncpy(bdbd.AlmaData[j].a1,str,sizeof(bdbd.AlmaData[j].a1)-1);
			}
		}
		ex_AlmaData = bdbd.AlmaData[12];
		bdbd.AlmaData[12] = bdbd.AlmaData[13];
		bdbd.AlmaData[13] = ex_AlmaData;
	} 
	//strncpy(bdbd.WNa,str,sizeof(bdbd.WNa)-1);
	IntToComplementBin(WNa,8,bdbd.WNa);
	for(j=0;j<30;j++)
	{
		strncpy(bdbd.Hea[j],str,sizeof(bdbd.Hea[j])-2);
		strcat(bdbd.Hea[j],"1");
	}
	strncpy(bdbd.A0utc,str,sizeof(bdbd.A0utc)-1);
	strncpy(bdbd.A1utc,str,sizeof(bdbd.A1utc)-1);
	strncpy(bdbd.detaTls,str,sizeof(bdbd.detaTls)-1);
	strncpy(bdbd.WNlsf,str,sizeof(bdbd.WNlsf)-1);
	strncpy(bdbd.DN,str,sizeof(bdbd.DN)-1);
	strncpy(bdbd.detaTlsf,str,sizeof(bdbd.detaTlsf)-1);
	strncpy(bdbd.A0gps,str,sizeof(bdbd.A0gps)-1);
	strncpy(bdbd.A1gps,str,sizeof(bdbd.A1gps)-1);
	strncpy(bdbd.A0gal,str,sizeof(bdbd.A0gal)-1);
	strncpy(bdbd.A1gal,str,sizeof(bdbd.A1gal)-1);
	strncpy(bdbd.A0glo,str,sizeof(bdbd.A0glo)-1);
	strncpy(bdbd.A1glo,str,sizeof(bdbd.A1glo)-1);
	//---
	//-----------可修改----------；
	//---D2部分导航电文, 数据处理；
	strncpy(bdbd.DIIOnlyData.Pnum2,str,sizeof(bdbd.DIIOnlyData.Pnum2)-1);
	strncpy(bdbd.DIIOnlyData.SatH2,str,sizeof(bdbd.DIIOnlyData.SatH2)-1);
	strncpy(bdbd.DIIOnlyData.BDIDi,str,sizeof(bdbd.DIIOnlyData.BDIDi)-1);
	for(j=0;j<18;j++)
	{
		strncpy(bdbd.DIIOnlyData.UDREI[j],str,sizeof(bdbd.DIIOnlyData.UDREI[j])-1);
		strncpy(bdbd.DIIOnlyData.detaTi[j],str,sizeof(bdbd.DIIOnlyData.detaTi[j])-1);
		strncpy(bdbd.DIIOnlyData.RURAI[j],str,sizeof(bdbd.DIIOnlyData.RURAI[j])-1);
	}
	for(j=0;j<320;j++)
	{
		strncpy(bdbd.DIIOnlyData.Ion[j],str,sizeof(bdbd.DIIOnlyData.Ion[j])-1);
	}
	//---
	memcpy(pbdbd,&bdbd,sizeof(bdbd));
	return 0;
}*/
void parse_bdslog_header(char *header, BDS_INFO &info){
	int cnt=0;
	char *p1;

	if((p1 = strtok(header, ","))) {
		cnt++;
	}
	while((p1 = strtok(NULL, ","))) {
		if(cnt==4 && strncmp(p1,"SATTIME",strlen("SATTIME"))!=0){
			cout << "ERROR:: Parse BDS Log Header Failed, SATTIME Wrong !!!" << endl;
			break;
		}
		else if(cnt==5)
			sscanf(p1,"%d",&info.week);
		else if(cnt==6)
			sscanf(p1,"%lf",&info.tow);
		cnt++;
	}
	if(cnt!=BDS_HEADER_SLICE)
		cout << "ERROR:: Parse BDS Log Header Failed, Slice Wrong !!!" << endl;
}

int parse_bdslog_body(char *body, BDS_INFO &info){
	int cnt=0,sat_id=0;
	char *p1;

	if((p1 = strtok(body, ","))) {
		cnt++;
	}
	while((p1 = strtok(NULL, ","))) {
		if(cnt==1)
			sscanf(p1,"%d",&sat_id);
		else if(cnt==2)
			strcmp(p1,"B1D1")?info.isD1=false:info.isD1=true;
		else if(cnt==3)
			sscanf(p1,"%d",&info.subframe_id);
		else if(cnt==4)
			strcpy(info.nav, p1);
		cnt++;
	}
	if(!strtok(info.nav, "*")){
		cout << "ERROR:: Parse BDS Log Body Failed, Nav Wrong !!!" << endl;
		return -1;
	}
	if(cnt!=BDS_BODY_SLICE){
		cout << "ERROR:: Parse BDS Log Body Failed, Slice Wrong !!!" << endl;
		return -1;
	}
	return sat_id;
}
/**********************************
|---------------------------------
	|-----------------------------
		|-------------------------
			|-------- |-----------
|--------------- |----------------

		|--->   final data   <---|
	line record
**********************************/
int preparse_oem615_bdslog(const char *file, set<int> &final_sat_set, int &line_end, int min_time_len, int min_sat_num){

	FILE *fp;
	int sat_id,sat_num=0;
	int i,week,ret_line=0,cnt=0;
	int is_sat_exist[MAX_SAT_ID]={0};
	int is_sat_healthy[MAX_SAT_ID]={0};
	int diff_tow_cnt=0;
	double tow=0;
	double first_tow[MAX_SAT_ID]={0};
	double last_tow[MAX_SAT_ID]={0};
	int duration[MAX_SAT_ID]={0};
	int line_record[MAX_SAT_ID]={0};
	bool is_init_ok = false;
	char *s,*p1,*id;
	BDS_INFO info={0};
	int delta[2]={6,60};
	char line[MAX_BUF];
	set<int> sat_set;

	if (!(fp=fopen(file,"r"))) {
		fprintf(stderr,"bds file open error : %s\n",file);
		return -1;
	}
	while (!feof(fp) && fgets(line, MAX_BUF, fp)){
		//skip some lines
		if(cnt++ < SKIP_LINES)
			continue;
		//find bds sign
		if(strncmp(line,BDS_HEADER,strlen(BDS_HEADER))!=0)
			continue;

		s = line;
		//change the line end to string end
		if (s[strlen(s) - 1] == '\n')
			s[strlen(s) - 1] = '\0';
		//split header and body
		if ((p1 = strchr(s, ';'))) {
			p1[0] = '\0';
		}
		//store xxx in info
		parse_bdslog_header(s, info);

		if(p1){
			p1++;
			sat_id = parse_bdslog_body(p1, info);
		}
		//init progress
		if(!is_init_ok){
			if(info.tow!=tow){
				tow=info.tow;
				diff_tow_cnt++;
				diff_tow_cnt > 4 ? is_init_ok=true : is_init_ok=false;
			}
			else{
				continue;
			}
		}
		//main process
		if(is_init_ok){
			if(!is_sat_exist[sat_id]){
				is_sat_exist[sat_id]=1;
				duration[sat_id]=0;
				first_tow[sat_id]=info.tow;
				last_tow[sat_id]=info.tow;
				line_record[sat_id]=cnt;
				sat_set.insert(sat_id);
				sat_num++;
				//infos.insert(make_pair(sat_id, info));
			}
			else{
				int delta_mul_10 = (int)((info.tow-last_tow[sat_id])*10+0.5);
				if(delta_mul_10 == delta[info.isD1])
					duration[sat_id] = info.tow - first_tow[sat_id];
				else{
					is_sat_exist[sat_id]=0;
					sat_set.erase(sat_id);
					sat_num--;
				}
				last_tow[sat_id]=info.tow;
			}
			if(sat_num >= min_sat_num && duration[sat_id] > min_time_len){
				int find_cnt=0;
				for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++){
					if(duration[*set_it]>min_time_len)
						find_cnt++;
				}
				if(find_cnt>=min_sat_num){
					for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++){
						if(duration[*set_it]>min_time_len){
							final_sat_set.insert(*set_it);
							ret_line = max(line_record[*set_it], ret_line);
						}
					}
					cout << "INFO:: Find OK BDS NAV DATA In The Log File !!!" << endl;
					fclose(fp);
					line_end = cnt;
					return ret_line;
				}
			}
		}
	}
	if(cnt < 500){
		fprintf(stderr,"file: %s is too short \n",file);
		fclose(fp);
		return -2;
	}
	fclose(fp);
	return -3;
}

int parse_oem615_bdslog(const char *file, int min_time_len, int min_sat_num){

	FILE *fp;
	FILE *fps[MAX_SAT_ID]={NULL};
	char *s,*p1;
	int is_sat_exist[MAX_SAT_ID]={0};
	int cnt = 0,sat_id,line_begin,line_end;
	char line[MAX_BUF];
	char data[256];
	char encode_data[512];
	BDS_INFO info={0};
	set<int> sat_set;

	line_begin = preparse_oem615_bdslog(file, sat_set, line_end, min_time_len, min_sat_num);
	if(line_begin<0){
		cout << "ERROR:: Preparse BDS Log Failed !!!" << endl;
		return -1;
	}
	//open many sat nav txt file to store nav data
	for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++){
		is_sat_exist[*set_it] = 1;
		stringstream f;
		f << *set_it << ".txt";
		if (!(fps[*set_it]=fopen(f.str().c_str(),"w"))) {
			fprintf(stderr,"result file open error : %s\n",f.str().c_str());
			return -1;
		}
		f.str("");
		f << *set_it << "#";
		fwrite(f.str().c_str(), sizeof(char), f.str().size(), fps[*set_it]);
	}
	if (!(fp=fopen(file,"r"))) {
		fprintf(stderr,"e1b file open error : %s\n",file);
		return -1;
	}
	while (!feof(fp) && fgets(line, MAX_BUF, fp)){
		//skip some lines
		if(cnt++ < line_begin)
			continue;
		//end in line_end
		if(cnt > line_end)
			break;
		//find bds sign
		if(strncmp(line,BDS_HEADER,strlen(BDS_HEADER))!=0)
			continue;

		s = line;
		//change the line end to string end
		if (s[strlen(s) - 1] == '\n')
			s[strlen(s) - 1] = '\0';
		//split header and body
		if ((p1 = strchr(s, ';'))) {
			p1[0] = '\0';
		}
		//store xxx in info
		parse_bdslog_header(s, info);
		if(p1){
			p1++;
			sat_id = parse_bdslog_body(p1, info);
			if(is_sat_exist[sat_id]){
				hex_str2bin(info.nav,strlen(info.nav),data);
				DataBCHEncode(data,encode_data);
				fwrite(encode_data, sizeof(char), 300, fps[sat_id]);
			}
		}
		
	}
	for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++)
		fclose(fps[*set_it]);

	if(cnt < 500){
		fprintf(stderr,"file: %s is too short \n",file);
		return -1;
	}
	if(union_all_sat_nav(sat_set, "BDSNAV.txt")!=0){
		fprintf(stderr,"union_all_sat_nav failed \n");
		return -1;
	}
	return 0;
}