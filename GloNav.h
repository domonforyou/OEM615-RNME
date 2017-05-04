#ifndef GLO_NAV
#define GLO_NAV

#define GLO_HEADER "#GLORAWSTRINGA"
#define GLO_HEADER_SLICE 10
#define GLO_BODY_SLICE 4
#define GLO_HEX_NAV_LEN 22 //22*4=88 bit (useful: last 85 bit, one string nav data)

const char MB[30]={1,1,1,1,1,0,0,0,1,1,0,1,1,1,0,1,0,1,0,0,0,0,1,0,0,1,0,1,1,0};
typedef struct _glo_info_{
	int week;
	double tow;
	int freq;
	char nav[32];
}GLO_INFO;

int parse_oem615_glolog(const char *file, int min_time_len, int min_sat_num);

#endif