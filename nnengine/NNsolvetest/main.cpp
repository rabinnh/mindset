/*
   Copyright [1994] [Richard Bross]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* 
 * File:   main.cpp
 * Author: rbross
 */

#include <cstdlib>
#include "Network.h"
#include <iostream>
#include "../../common/CExplode.h"
#include <fstream>

using namespace std;

double test[] = {356,221,64.7,3329,7.5,19,5.3,82,6,1.7};

int main(int argc, char** argv)
{
	NeuralNet	nNetwork;
	nNetwork.Create("/home/rbross/temp/networktest.mnd");

	double pOutput;

	nNetwork.SetInputsScaled(test);
	nNetwork.ProcessData();
	nNetwork.GetFinalOutputsScaled(&pOutput);
	
	cout << pOutput << endl;
	
	return 0;
}

