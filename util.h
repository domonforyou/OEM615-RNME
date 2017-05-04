#ifndef _UTIL_H_
#define _UTIL_H_

#include "common.h"

#define IS_LITTLE_ENDIAN 1

int hexstr2byte(char *buf,unsigned char *ret);
int hex_str2bin(char *hex, int n, char *bin);
int union_all_sat_nav(set<int> &sat_set, char *ret_file);

#endif