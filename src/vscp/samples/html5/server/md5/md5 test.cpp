/*
 * A little test program for md5 code which does a md5 on all the
 * bytes sent to stdin.
 */


//#include <iostream>
//#include <string>
//#include <regex>
//#include <vector>
#include <Windows.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"
//#include "md5_loc.h"

//using namespace std;

int main()
{	
	char* s =  "d41d8cd98f00b204e9800998ecf8427e";
	char buf[16];
	char* sig = (char*)calloc(16,sizeof(char));
	md5_buffer(buf, 16, sig);
	return 0;
}

