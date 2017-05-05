// main.cpp : 定义控制台应用程序的入口点。
//

#include "common.h"
#include "Gnss_nav.h"
#include "util.h"

using namespace std;

void Usage(){
	string str = "Usage:: ./Nav_Trans.exe [config file (option)], the default config file --> RNME.conf, u can change by yourself.";
	cout << str << endl;
}

//APP:: OEM615 Nav Log Decoder
//TODO:: Gal System Support
int main(int argc, char* argv[])
{
	int min_time_len,min_sat_num;
	char *filename = NULL;
	s_config *config = config_get_config();
	config_init();

	if(argc != 1 && argc != 2){
		Usage();
		Sleep(10000);
		exit(-1);
	}
	else if(argc == 2){
		config->confile = argv[1];
	}

	/* Initialize the config */
	config_read(config->confile);

	Gnss_nav trans(config->logfile);
	trans.Decode_Rec_Log(config->min_sec, config->min_sat);
	return 0;
}