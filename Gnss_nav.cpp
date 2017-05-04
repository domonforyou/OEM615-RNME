#include "common.h"
#include "util.h"
#include "BdsNav.h"
#include "GloNav.h"
#include "GpsNav.h"
#include "Gnss_nav.h"


Gnss_nav::Gnss_nav(void)
{
	cout << "Hello Kids" << endl;
}

Gnss_nav::Gnss_nav(const char *filename){
	char *p1,fin[MAX_BUF],suffix[8];
	int i;
	if(filename == NULL){
		cout << "INPUT ERROR:: No Log, No Output !!!" << endl;
		Sleep(5000);
		exit(-1);
	}
	logfile = filename;
	strcpy(fin, filename);
	//parse log sys type by suffix 
	if((p1 = strtok(fin, "."))) {
		//strcpy(suffix, p1);
	}
	while((p1 = strtok(NULL, "."))) {
		strcpy(suffix, p1);
	}
	for(i=SYS_GPS; i<SYS_MAX; i++){
		if(strncmp(suffix,LOG_SUFFIX[i],strlen(LOG_SUFFIX[i]))==0)
			break;
	}
	log_type = (GNSS_SYS_TYPE)i;
	cout << "INFO:: System ---> " << LOG_SUFFIX[i] << endl;
}

int Gnss_nav::Decode_Rec_Log(int min_time_len, int min_sat_num){
	int ret=-1;
	switch (log_type)
	{
	case SYS_GPS: 
		ret = parse_oem615_gpslog(logfile, min_time_len, min_sat_num);
		break;
	case SYS_BDS: 
		ret = parse_oem615_bdslog(logfile, min_time_len, min_sat_num);
		break;
	case SYS_GLO:
		ret = parse_oem615_glolog(logfile, min_time_len, min_sat_num);
		break;
	case SYS_GAL: 
		ret = parse_oem615_glolog(logfile, min_time_len, min_sat_num);
		break;
	case SYS_MIX: 
		ret = parse_oem615_gpslog(logfile, min_time_len, min_sat_num);
		ret = parse_oem615_bdslog(logfile, min_time_len, min_sat_num);
		ret = parse_oem615_glolog(logfile, min_time_len, min_sat_num);
		break;
	default:
		break;
	}
	if(!ret){
		cout << "INFO:: EveryThing is OK ..." << endl;
		Sleep(5000);
	}
	else if(ret==-1){
		cout << "ERROR:: Main Process is Wrong ..." << endl;
		Sleep(5000);
	}
	return 0;
}

Gnss_nav::~Gnss_nav(void)
{
	cout << "Bye Bye Kids" << endl;
}