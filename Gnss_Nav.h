#ifndef GNSS_NAV
#define GNSS_NAV

#include "common.h"

#define FILE_NAME "H:\\ooooo\\three.mix"
#define REC_TYPE "OEM615"
#define MIN_LAST_SEC 600
#define MIN_SAT_SUM 6
#define SKIP_LINES 200
#define MAX_SAT_NUM 16
#define MAX_SAT_ID 33

typedef enum _system_type_{
	SYS_GPS = 0,
	SYS_BDS,
	SYS_GLO,
	SYS_GAL,
	SYS_MIX,
	SYS_MAX
}GNSS_SYS_TYPE;
const char LOG_SUFFIX[5][4]={"gps","bds","glo","gal","mix"};
typedef struct _sat_status_{
	double start_time;
	set<int> sat_set;
}SAT_STATUS;

class Gnss_nav
{
public:
	Gnss_nav(void);
	Gnss_nav(const char *filename);
	~Gnss_nav(void);

	int Decode_Rec_Log(int min_time_len, int min_sat_num);

private:
	GNSS_SYS_TYPE log_type;
	SAT_STATUS sat_status;
	const char *logfile;
	map<string, int> sys_map;
};

#endif // GNSS_NAV
