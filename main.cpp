// main.cpp : 定义控制台应用程序的入口点。
//

#include "common.h"
#include "Gnss_nav.h"

using namespace std;

void Usage(){
	string str = "Usage:: ./Nav_Trans.exe Log_File Min_Time_Len Min_Sat_Num, Log_File Should Be Put In The Exe Dir";
	cout << str << endl;
	cout << endl;
	str = "If U Run ./Nav_Trans.exe Log_File, It May Equal to ./Nav_Trans.exe Log_File 600 6";
	cout << str << endl;
}

//APP:: OEM615 Nav Log Decoder
//TODO:: Gal System Support
int main(int argc, char* argv[])
{
	int min_time_len,min_sat_num;
	char *filename = NULL;
	cout << "Hello Kids !!!" << endl;
	if(argc != 4 && argc != 2){
		Usage();
		Sleep(10000);
		exit(-1);
	}
	else if(argc == 2){
		filename = argv[1];
		min_time_len = MIN_LAST_SEC;
		min_sat_num = MIN_SAT_SUM;
	}
	else{
		filename = argv[1];
		min_time_len = atoi(argv[2]);
		min_sat_num = atoi(argv[3]);
	}
	Gnss_nav trans(filename);
	trans.Decode_Rec_Log(min_time_len, min_sat_num);
	return 0;
}