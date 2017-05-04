// main.cpp : 定义控制台应用程序的入口点。
//

#include "common.h"
#include "Gnss_nav.h"


int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "Hello Tiny !!!" << std::endl;
	Gnss_nav test(FILE_NAME);
	return 0;
}