#ifndef GPS_NAV
#define GPS_NAV

#define GPS_HEADER "#RAWGPSWORDA"
#define GPS_HEADER_SLICE 10
#define GPS_BODY_SLICE 2
#define GPS_HEX_NAV_LEN 8 //8*4=32 bit (useful: last 30 bit, one word)

typedef struct _gps_info_{
	int week;
	double tow;
	int subframe_id;
	char nav[64];
}GPS_INFO;

int parse_oem615_gpslog(const char *file, int min_time_len, int min_sat_num);

#endif