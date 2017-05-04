#include "util.h"
#include "Gnss_nav.h"


/* extract field (big-endian) ------------------------------------------------*/
#ifdef IS_BIG_ENDIAN

#define U1(p)       (*((unsigned char *)(p)))
#define I1(p)       (*((char *)(p)))

static unsigned short U2(unsigned char *p)
{
    union {unsigned short u2; unsigned char b[2];} buff;
    buff.b[0]=p[1]; buff.b[1]=p[0];
    return buff.u2;
}
static unsigned int U4(unsigned char *p)
{
    union {unsigned int u4; unsigned char b[4];} buff;
    buff.b[0]=p[3]; buff.b[1]=p[2]; buff.b[2]=p[1]; buff.b[3]=p[0];
    return buff.u4;
}
static double R8(unsigned char *p)
{
    union {double r8; unsigned char b[8];} buff;
    buff.b[0]=p[7]; buff.b[1]=p[6]; buff.b[2]=p[5]; buff.b[3]=p[4];
    buff.b[4]=p[3]; buff.b[5]=p[2]; buff.b[6]=p[1]; buff.b[7]=p[0];
    return buff.r8;
}

#endif

/* get fields (little-endian) ------------------------------------------------*/
#ifdef IS_LITTLE_ENDIAN

#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static int            I4(unsigned char *p) {int            i; memcpy(&i,p,4); return i;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}

#endif
/*******************
 * in: hex string buffer
 * out: unsigned byte array
 * return: num of bytes
 * *******************/
int hexstr2byte(char *buf,unsigned char *ret){
    int i,len=strlen(buf)/2;
    char p[3];
    if(!ret)return -1;
    for(i=0;i<len;i++){
        strncpy(p,buf+i*2,2);
        sscanf(p,"%x",ret+i);
    }
    return len;
}

//将16进制字符串转化为整形的10010001
// 57->'9'; 70->'F'; 97-102->'a'-'f' 
int hex_str2bin(char *hex, int n, char *bin){
	char temp;
	int i,j;
	char *p = bin;
	for(i=0;i<n;i++){
		if(*hex < 58)
			temp = *hex-'0';
		else if(*hex < 71)
			temp = *hex-'A'+10;
		else
			temp = *hex-'a'+10;//TODO::a is different from A
		for(j=0;j<4;j++){
			p[j]=temp>>(3-j)&0x01;
		}
		hex++;
		p=p+4;
	}
	return 0;
}

int union_all_sat_nav(set<int> &sat_set, char *ret_file){
	
	FILE *fp;
	FILE *fps[MAX_SAT_ID]={NULL};
	char line[MAX_BUF];
	if (!(fp=fopen(ret_file,"w"))) {
		fprintf(stderr,"final nav file open error : %s\n",ret_file);
		return -1;
	}
	//open all sat nav txt file for reading nav data
	for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++){
		stringstream f;
		f << *set_it << ".txt";
		if (!(fps[*set_it]=fopen(f.str().c_str(),"r"))) {
			fprintf(stderr,"result file open error : %s\n",f.str().c_str());
			return -1;
		}
		while (!feof(fps[*set_it]) && fgets(line, MAX_BUF, fps[*set_it])){
			fwrite(line, sizeof(char), strlen(line), fp);
		}
		fwrite("\n", sizeof(char), strlen("\n"), fp);
		fclose(fps[*set_it]);
		if( remove(f.str().c_str()) == 0 )
			printf("Debug:: Removed %s. \n", f.str().c_str());
		else
			perror("remove sat txt file failed \n");
	}
	fclose(fp);

	return 0;
}