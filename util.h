#ifndef _UTIL_H_
#define _UTIL_H_

#include "common.h"

#define IS_LITTLE_ENDIAN 1

#define DEFAULT_CONFILE "RNME.conf"
#define DEFAULT_LOGFILE "GPS_BDS_GLO.mix"
#define DEFAULT_MIN_SEC 600
#define DEFAULT_MIN_SAT 6

typedef struct _s_config_{
	char *confile;
	char logfile[64];
	int min_sec;
	int min_sat;
}s_config;

/** @brief Get the current gateway configuration */
s_config *config_get_config(void);

/** @brief Initialise the conf system */
void config_init(void);

/** @brief Initialize the variables we override with the command line*/
void config_init_override(void);

/** @brief Reads the configuration file */
void config_read(const char *filename);

int hexstr2byte(char *buf,unsigned char *ret);
int hex_str2bin(char *hex, int n, char *bin);
int union_all_sat_nav(set<int> &sat_set, char *ret_file);

#endif