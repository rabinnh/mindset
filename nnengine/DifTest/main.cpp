/* 
 * File:   main.cpp
 * Author: rbross
 *
 * Created on August 22, 2014, 8:09 AM
 */

#include <cstdlib>
#include "Difobj.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv)
{
	CDIFObject cDif("/home/rbross/myprojects/Languages/C/Oaktree/mindset/Examples/QB Ratings2.dif");
	
	cDif.ParseDIFFile();
	
	return 0;
}

