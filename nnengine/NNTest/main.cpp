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

 * File:   main.cpp
 * Author: rbross
 *
 * Created on November 28, 2016, 7:25 AM
 */

#include <cstdlib>
#include "Network.h"
#include <iostream>
#include "../../common/CExplode.h"
#include <fstream>
#include <omp.h>



using namespace std;

void ProgressCallback(unsigned int epochs, double error, void *pUserParm);

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << "You must enter the file name on the command line" << endl;
		exit(1);
	}

	// Set max OMP threads
	omp_set_num_threads(8);		
	
	ifstream		csvFile(argv[1]);
	string			csvLine;
	NeuralNet		nNetwork;	// The neural net
	unsigned int	layers[3] = {10, 8, 1}; // The network topology
	TRAINSET_VEC	vTrainSet;
	double			learn_rate = 0.03;
	double			allowable_error = 0.001913;
	double			momentum = 0.5;
		
	// Create it
	nNetwork.Create(3, layers, learn_rate, allowable_error, momentum, DEFAULT_GAIN, true);
	nNetwork.RegisterProgressCallback(ProgressCallback, NULL, 10000);

	// Calculate input and output layer counts
	unsigned int inputs = layers[0];
	unsigned int outputs = layers[(sizeof(layers) / sizeof(unsigned int)) - 1];
	unsigned int total = inputs + outputs;
	
	// Store the data
	while (std::getline(csvFile, csvLine))
	{
		VECTOR_PHRASES	vCol;
		CExplode		cCSVRow;
		if (cCSVRow.Explode((const char *) csvLine.c_str(), ",", vCol) != total)
		{
			cout << "Row did not have 11 values" << endl;
			break;
		}
		sTrainSet *pSet = new sTrainSet;
		pSet->dInputs = new double[inputs];
		pSet->dOutputs = new double[outputs];
		pSet->dScaledInputs = new double[inputs];
		pSet->dScaledOutputs = new double[outputs];		
//		cout.precision(1);
//		cout << endl << "New" << endl;
		for (int iX = 0; iX < inputs; iX++)
		{
			pSet->dInputs[iX] = std::stod(vCol[iX], NULL);
//			cout << fixed << pSet->dInputs[iX] << endl;
		}
		for (int iX = 0; iX < outputs; iX++)
		{
			pSet->dOutputs[iX] = std::stod(vCol[iX + inputs], NULL);
//			cout << fixed << pSet->dOutputs[iX] << endl;
		}
		// This just contains pointers to the data
		vTrainSet.push_back(pSet);
	}
	
	// This is set here, so that we can interrupt training if we desire
	int iTrain = NeuralNet::START_TRAIN;
	nNetwork.Train(vTrainSet, &iTrain);
	
	// Clean up the data
	for (int iX = 0; iX < vTrainSet.size(); iX++)
	{
		sTrainSet *pSet = vTrainSet[iX];
		delete[] pSet->dInputs;
		delete[] pSet->dOutputs;
		delete[] pSet->dScaledInputs;
		delete[] pSet->dScaledOutputs;		
		delete pSet;
	}
	
	// Test saving the network
	nNetwork.SaveNetwork("/home/rbross/temp/networktest.mnd");
	
	return 0;
}

void ProgressCallback(unsigned int epochs, double error, void *pUserParm)
{
	cout << "Epochs " << epochs << " Error " << error << endl;
}
