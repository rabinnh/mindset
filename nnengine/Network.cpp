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
///////////////////////////////////////////////////////////////////////////////
//    NETWORK.CPP - Neural network object 
//            4/8/94 - RAB
///////////////////////////////////////////////////////////////////////////////
#include "Network.h"
#include "Text.h"
#include "CMindsetConfig.h"
#include "Crc32.h"
#include <cstdio>
#include <errno.h>

// Constructor

NeuralNet::NeuralNet()
{
    // Determine the number of processors 
    elements = NULL;
    pelements = NULL;
    PElement = NULL;
    bCreated = false;
};


///////////////////////////////////////////////////////////////////////////////
//     Create for NeuralNet
//
//     NOTE: Each neuron randomizes it's own weights when it is created
//

int NeuralNet::Create(unsigned int num_layers, unsigned int *layer_elements, double learn_rate,
    double allowable_error, double momentum_factor, double gain_f, bool bRProp)
{
    unsigned int element;
    unsigned int uType;

    // Load our elements array
    elements = new unsigned int[num_layers];
    pelements = new unsigned int[num_layers];

    // We need the gain loaded
    dGain = gain_f;

    ::memcpy(elements, layer_elements, sizeof(unsigned int) * num_layers);
    ::memcpy(pelements, layer_elements, sizeof(unsigned int) * num_layers);

    // If there is a bias, allocate extra neuron except in output
    for(layers = 0; layers < num_layers - 1; layers++)
        elements[layers]++;

    // Create the neurons
    lTotalConnects = 0;
    PElement = new Neuron**[num_layers]; // Allocate layer pointers
    for(layers = 0; layers < num_layers; layers++)
    { // Allocate neuron pointer
        if (layers)
            lTotalConnects += (long) (elements[layers - 1] * elements[layers]);
        PElement[layers] = new Neuron*[elements[layers]];

        // Set our layer enum
        if (!layers)
        {
            uType = INPUT;
            num_inputs = 1;
        }
        else
        {
            num_inputs = elements[layers - 1];
            if (layers == num_layers - 1)
                uType = OUTPUT;
            else
                uType = HIDDEN;
        };

        // Network is "massively connected", 
        // all previous PEs -> this layer
        for(element = 0; element < elements[layers]; element++)
        {
            PElement[layers][element] = AllocateNeuron(this, num_inputs, element, uType);
        };
    };

    layers = num_layers;
    sublayer = layers - 1;
    error = allowable_error;
    delta = learn_rate;
    momentum = momentum_factor;
    prev_error = dLowestError = MAX_DOUBLE; // Some ridiculously high number 
    total_error = fErrorDisplay = 0.0f;
    iTrained = UNTRAINED;
    lIteration = 0;
    dwTotalElapsed = 0;
    bUseRProp = bRProp;

    // If there is a bias, set the last one on the first layer
    PElement[0][elements[0] - 1]->output = 1.0f;

    // Randomize the weights
    RandomizeWeights();
    bCreated = true;
	
	pCallback = NULL;

    return(0);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//  Create a NeuralNet from a saved network file.  This will include saved weights.
//
//  Each neuron randomizes it's own weights when it is created, but after
//  allocation, we will set them based on values in the file. 
//

int NeuralNet::Create(const char *szNetworkFile)
{
    CMindsetConfig cConfig;
    int iReturn;
    unsigned int layer;
    unsigned int pe;
    unsigned int input;
    int iWeightCount = 0;
    int iLabel = 0;

    bVerified = false;
    iReturn = cConfig.ParseXMLFile(szNetworkFile);
    if (iReturn)
        return(iReturn);
    bVerified = cConfig.bVerified;
    if (!bVerified)
        return(-1);

    // Create the network
    Create(cConfig.iLayers,
        (unsigned int *) &(cConfig.vLayerNodeCount[0]),
        cConfig.dLearnRate,
        cConfig.dAllowableError,
        cConfig.dMomentum,
        cConfig.dGain,
        cConfig.bRProp);

    iTrained = cConfig.iTrainingStatus;

    // Load the means and std deviation arrays
    vInputMean = cConfig.vInputMean;
    vInputStdDev = cConfig.vInputStdDev;
    vOutputMean = cConfig.vOutputMean;
    vOutputStdDev = cConfig.vOutputStdDev;

    // Set all the weights
    // Save all weights
    for(layer = 0; layer < layers; layer++)
    {
        for(pe = 0; pe < pelements[layer]; pe++)
        {
            for(input = 0; input < PElement[layer][pe]->inputs; input++)
            {
                SetWeight(layer, pe, input, cConfig.vWeights[iWeightCount++]);
            };
        };
    };

    // Set the neuron labels
    // Inputs
    layer = 0;
    for(pe = 0; pe < pelements[layer]; pe++)
    {
        if (iLabel < (int) cConfig.vLabels.size())
        {
            PElement[layer][pe]->SetNeuronLabel(cConfig.vLabels[iLabel++].c_str());
        };
    };
    // Outputs
    layer = layers - 1;
    for(pe = 0; pe < pelements[layer]; pe++)
    {
        if (iLabel < (int) cConfig.vLabels.size())
        {
            PElement[layer][pe]->SetNeuronLabel(cConfig.vLabels[iLabel++].c_str());
        };
    };

    // All the weights have to be set.
    return(0);
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//  Register a progress callback
//
// pFunc is the user function
// pUserParm is a user parameter that is passed back when the function is called
// uEpochCount is the number of epochs that must have passed for it to be called
void NeuralNet::RegisterProgressCallback(NN_CALLBACK pFunc, void* pUserParm, unsigned int uEpochCount)
{
	pCallback = pFunc;
	this->pUserParm = pUserParm;
	this->uEpochCount = uEpochCount;
}


///////////////////////////////////////////////////////////////////////////////
//  Callback with status
//

void NeuralNet::UpdateStatus(bool bErase, int iTrain) 
{
	if ((pCallback && (lIteration % uEpochCount) == 0) || (pCallback && bErase))
		pCallback(lIteration, total_error, pUserParm);
}

///////////////////////////////////////////////////////////////////////////////
//  Save a NeuralNet to file.  This will include saved weights.
//

int NeuralNet::SaveNetwork(const char * szNetworkFile)
{ // Put this in the same class as the code that reads and parses the file
    FILE *pFile = NULL;
    char szBuffer[1024];
    unsigned int layer;
    unsigned int pe;
    unsigned int input;
    unsigned long lCRC = 0xFFFFFFFF;

    // Open the file.  This will overwrite a file that already exists
    pFile = ::fopen(szNetworkFile, ("wb"));
    if (!pFile)
        return(errno);

    // Start saving stuff
    // MindSetNetwork tag
    FileWriteLine(pFile, szMindSetNetwork, lCRC);
    // Configuration 
    ::sprintf(szBuffer, szConfiguration, layers);
    FileWriteLine(pFile, szBuffer, lCRC);
    // Layers
    for(layer = 0; layer < (int) layers; layer++)
    {
        int iType = HIDDEN;
        if (!layer)
            iType = INPUT;
        else
            if (layer == (layers - 1))
            iType = OUTPUT;
        ::sprintf(szBuffer, szLayer, iType, pelements[layer]);
        FileWriteLine(pFile, szBuffer, lCRC);
    };
    // Neuron labels
    // Inputs
    layer = 0;
    for(pe = 0; pe < pelements[layer]; pe++)
    {
        ::sprintf(szBuffer, szLabel, PElement[layer][pe]->cLabel.c_str());
        FileWriteLine(pFile, szBuffer, lCRC);
    };
    // Outputs
    layer = layers - 1;
    for(pe = 0; pe < pelements[layer]; pe++)
    {
        ::sprintf(szBuffer, szLabel, PElement[layer][pe]->cLabel.c_str());
        FileWriteLine(pFile, szBuffer, lCRC);
    };

    // Allowable error
    ::sprintf(szBuffer, szAllowableError, error);
    FileWriteLine(pFile, szBuffer, lCRC);
    // Learn rate
    ::sprintf(szBuffer, szLearnRate, delta);
    FileWriteLine(pFile, szBuffer, lCRC);
    // Momentum
    ::sprintf(szBuffer, szMomentum, momentum);
    FileWriteLine(pFile, szBuffer, lCRC);
    // Momentum
    ::sprintf(szBuffer, szGain, dGain);
    FileWriteLine(pFile, szBuffer, lCRC);
    // Use RProp
    ::sprintf(szBuffer, szRProp, bUseRProp);
    FileWriteLine(pFile, szBuffer, lCRC);
    // Training status
    ::sprintf(szBuffer, szTrainingStatus, iTrained);
    FileWriteLine(pFile, szBuffer, lCRC);
    // End configuration
    FileWriteLine(pFile, szEndConfiguration, lCRC);
    // Saved training data
    FileWriteLine(pFile, szSavedTrainingData, lCRC);
    // Scaling
    FileWriteLine(pFile, szScaling, lCRC);
    // Inputs
    FileWriteLine(pFile, szInput, lCRC);
    // Scaling stuff
    for(int iX = 0; iX < (int) vInputMean.size(); iX++)
    { // Mean
        ::sprintf(szBuffer, szMean, vInputMean[iX]);
        FileWriteLine(pFile, szBuffer, lCRC);
        // Std dev
        ::sprintf(szBuffer, szStdDev, vInputStdDev[iX]);
        FileWriteLine(pFile, szBuffer, lCRC);
    };
    // End inputs
    FileWriteLine(pFile, szEndInput, lCRC);
    // Outputs
    FileWriteLine(pFile, szOutput, lCRC);
    // Scaling stuff
    for(int iX = 0; iX < (int) vOutputMean.size(); iX++)
    { // Mean
        ::sprintf(szBuffer, szMean, vOutputMean[iX]);
        FileWriteLine(pFile, szBuffer, lCRC);
        // Std dev
        ::sprintf(szBuffer, szStdDev, vOutputStdDev[iX]);
        FileWriteLine(pFile, szBuffer, lCRC);
    };
    // End outputs
    FileWriteLine(pFile, szEndOutput, lCRC);
    // End scaling
    FileWriteLine(pFile, szEndScaling, lCRC);
    // Weights
    FileWriteLine(pFile, szWeights, lCRC);
    // Save all weights
    for(layer = 0; layer < layers; layer++)
    {
        for(pe = 0; pe < pelements[layer]; pe++)
        {
            for(input = 0; input < PElement[layer][pe]->inputs; input++)
            {
                double dWeight = GetWeight(layer, pe, input);
                ::sprintf(szBuffer, "%f", dWeight);
                FileWriteLine(pFile, szBuffer, lCRC);
                if (layer != layers - 1 || pe != elements[layer] - 1 || input != PElement[layer][pe]->inputs - 1)
                {
                    FileWriteLine(pFile, szComma, lCRC);
                };
            };
        };
    };
    // End weights
    FileWriteLine(pFile, szEndWeights, lCRC);
    // End saved training data
    FileWriteLine(pFile, szEndSavedTrainingData, lCRC);
    // Verification code
    ::sprintf(szBuffer, szCRC, lCRC);
    FileWriteLine(pFile, szBuffer, lCRC);
    // MindSetNetwork
    FileWriteLine(pFile, szEndMindSetNetwork, lCRC);

    // Close the file
    ::fclose(pFile);

    return(0);
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//
//  Write a line in the config file and update the CRC
//

int NeuralNet::FileWriteLine(FILE *pFile, char *pBuffer, unsigned long &lCRC)
{
    CRC32 cCRC;
    int iReturn;

    // End weights
    iReturn = ::fwrite(pBuffer, 1, ::strlen(pBuffer), pFile);
    lCRC = cCRC.CalcCRC(lCRC, (unsigned char *) pBuffer, ::strlen(pBuffer));

    return(iReturn);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//
// This allows a derived class to have NeuralNet allocate a derived Neuron
//
// AParent - pointer to parent NeuralNet
// num_inputs - number of inputs to this neuron (# neurons in the previous layer)
// element - a unique id for this neuron
// uLayer - one of the enums, INPUT, HIDDEN, OUTPUT
//

Neuron *NeuralNet::AllocateNeuron(NeuralNet *AParent, unsigned int num_inputs, unsigned int element, unsigned int uLayer)
{
    return(new Neuron(AParent, num_inputs, element));
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//
// Created successfully?
//

bool NeuralNet::Created()
{
    return(bCreated);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//    Destructor for NeuralNet
//

NeuralNet::~NeuralNet()
{
    unsigned int layer;
    unsigned int pe;

    // Delete allocated neurons
    if (PElement)
    {
        for(layer = 0; layer < layers; layer++)
        {
            if (PElement[layer])
            {
                for(pe = 0; pe < elements[layer]; pe++)
                {
                    if (PElement[layer][pe])
                        delete PElement[layer][pe];
                };
                delete[] PElement[layer];
            }
        };
        delete[] PElement;
    };
    if (elements)
        delete[] elements;
    if (pelements)
        delete[] pelements;

}; // End destructor
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//    Set stored weight
//

int NeuralNet::SetWeight(unsigned int layer, unsigned int pe, unsigned int input, double weight)
{
    if (layer >= layers || pe >= elements[layer])
        return(-1);

    PElement[layer][pe]->weights[input] = weight;

    return(0);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//    Randomize weights in prepartion for learning
//

void NeuralNet::RandomizeWeights()
{
    unsigned int layer;
    unsigned int pe;
    unsigned int input;

    for(layer = 0; layer < layers; layer++)
        for(pe = 0; pe < elements[layer]; pe++)
        {
            if (!layer) // Set all weights in input layer to 1
            {
                for(input = 0; input < PElement[layer][pe]->inputs; input++)
                    PElement[layer][pe]->weights[input] = 1.0f;
            }
            else // Set each input weight
                PElement[layer][pe]->RandomizeNeuronWeights();
        };
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//    Get an input's weight
//

double NeuralNet::GetWeight(unsigned int layer, unsigned int pe, unsigned int input)
{
    if (layer >= layers || pe >= elements[layer])
        return(-1.0f);

    return(PElement[layer][pe]->weights[input]);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//	Set a neuron's output - used to set value of input neurons
//		
//	Input neurons are linear; i.e. they are used only to store the values to
//	be processed.  So we simply set their outputs directly.  This is rarely
//	used since we really have to scale the inputs in order to process them.
//

int NeuralNet::SetNeuronOutput(unsigned int layer, unsigned int pe, double out)
{
    if (layer >= layers || pe >= elements[layer])
        return(-1);

    PElement[layer][pe]->output = out;

    return(0);
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//  Set a neuron's output and scale it in - used to set value of input neurons
//
//	Input neurons are linear; i.e. they are used only to store the values to
//	be processed.  So we simply set their outputs directly.  
//		

int NeuralNet::SetNeuronOutputScaled(unsigned int layer, unsigned int pe, double out)
{
    if (layer >= layers || pe >= elements[layer])
        return(-1);

    PElement[layer][pe]->ScaleIn(out);

    return(0);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//     Get a neuron's output
//

double NeuralNet::GetNeuronOutput(unsigned int layer, unsigned int pe)
{
    if (layer >= layers || pe >= elements[layer])
        return(-1.0f);

    return(PElement[layer][pe]->output);
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//  	Get all of the output layer's values.
//    	The double array must be large enough to hold all of the values.
//

int NeuralNet::GetFinalOutputs(double *outputs)
{
    unsigned int pe;

    if (!outputs)
        return(-1);

    for(pe = 0; pe < elements[sublayer]; pe++)
        outputs[pe] = PElement[sublayer][pe]->output;

    return(0);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//    Get all of output layer's output, telling neurons to scale them out
//    double array must be large enough to hold all values
//

int NeuralNet::GetFinalOutputsScaled(double *outputs)
{
    unsigned int pe;

    if (!outputs)
        return(-1);

    for(pe = 0; pe < elements[sublayer]; pe++)
        outputs[pe] = PElement[sublayer][pe]->ScaleOut();

    return(0);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//     Set all inputs
//

int NeuralNet::SetInputs(double *inputs)
{
    unsigned int pe;
    unsigned int els;

    if (!inputs)
        return(-1);

    els = elements[0] - 1;
    for(pe = 0; pe < els; pe++)
        SetNeuronOutput(0, pe, inputs[pe]);

    return(0);
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//     Set all inputs and then tell the neurons to scale them in
//

int NeuralNet::SetInputsScaled(double *inputs)
{
    unsigned int pe;
    unsigned int els;

    if (!inputs)
        return(-1);

    els = elements[0] - 1;
    for(pe = 0; pe < els; pe++)
        SetNeuronOutputScaled(0, pe, inputs[pe]);

    return(0);
};
///////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
//  Scale neurons to minimums and maximums
//
//  Outputs must be between 0 and 1, but a good input range would be between 0 and 3
//
//	The best scaling is accomplished by subtracting the mean and dividing by 
//	the standard deviation.
//

bool NeuralNet::ScaleNeurons(TRAINSET_VEC &vTrainSet)
{
    unsigned int uIndex; // An index
    unsigned int uSet; // Sets
    double dMean;
    double dStdDev;
    DOUBLE_VEC vWork;
    double dWork;

    // Initialize
    vInputMean.clear();
    vInputStdDev.clear();
    vOutputMean.clear();
    vOutputStdDev.clear();

    // Find the mean and std dev for each input column
    // For each column . . .
    for(uIndex = 0; uIndex < pelements[0]; uIndex++)
    { // Collect the column for each set (epoch)
        vWork.clear();
        for(uSet = 0; uSet < (unsigned int) vTrainSet.size(); uSet++)
            vWork.push_back(vTrainSet[uSet]->dInputs[uIndex]);
        // Get the mean and std deviation for this column
        if (!CalculateStandardDeviation(vWork, dMean, dStdDev))
            return(false);
        // Save the results
        if (dStdDev == 0.0f)
            dStdDev = 0.01f;
        vInputMean.push_back(dMean);
        vInputStdDev.push_back(dStdDev);
        // Scale the inputs for this column and save as part of the training sets
        for(uSet = 0; uSet < (unsigned int) vTrainSet.size(); uSet++)
        {
            dWork = vTrainSet[uSet]->dInputs[uIndex] - dMean;
            dWork /= dStdDev;
            dWork += 3; // So make them all positive
            dWork /= 2; // and between 0 and 3
            if (dWork < 0)
                dWork = 0;
            vTrainSet[uSet]->dScaledInputs[uIndex] = dWork;
        };
    };

    // Find the mean and std dev for each output column
    // For each column
    for(uIndex = 0; uIndex < pelements[layers - 1]; uIndex++)
    { // Collect the column for each set (epoch)
        vWork.clear();
        for(uSet = 0; uSet < (unsigned int) vTrainSet.size(); uSet++)
            vWork.push_back(vTrainSet[uSet]->dOutputs[uIndex]);
        // Get the mean and std deviation for this column
        if (!CalculateStandardDeviation(vWork, dMean, dStdDev))
            return(false);
        // Save the results
        if (dStdDev == 0.0f)
            dStdDev = 0.01f;
        vOutputMean.push_back(dMean);
        vOutputStdDev.push_back(dStdDev);
        // Scale the outputs for this column and save as part of the training sets
        for(uSet = 0; uSet < (unsigned int) vTrainSet.size(); uSet++)
        {
            dWork = vTrainSet[uSet]->dOutputs[uIndex] - dMean;
            dWork /= dStdDev; // This gives a result about -3 to +3
            dWork += 3; // So make them all positive
            dWork /= 6; // and between 0 and .999999
            if (dWork < 0)
                dWork = 0;
            if (dWork > 1.0f)
                dWork = 1.0f;
            vTrainSet[uSet]->dScaledOutputs[uIndex] = dWork;
        };
    };

    return(true);
};
/////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//     Process inputs in this array.  Layer 0 is input layer
//

int NeuralNet::ProcessInputs(double *inputs)
{
    unsigned int pe;
    unsigned int els;

    if (!inputs)
        return(-1);

    els = elements[0] - 1;
    for(pe = 0; pe < els; pe++)
        SetNeuronOutput(0, pe, inputs[pe]);

    ProcessData();

    return(0);
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//     Process data already placed in the input layer outputs.
//

void NeuralNet::ProcessData()
{
    unsigned int layer;
    unsigned int pe;

    // For each layer of neurons after the input layer
    for(layer = 1; layer < layers; layer++)
    { // For each neuron in layer
        for(pe = 0; pe < elements[layer]; pe++)
        { // Process all outputs from previous layer
            PElement[layer][pe]->CalculateOutput(PElement[layer - 1]);
        };
        // Make sure all threads are done
        if (layer < sublayer)
            PElement[layer][elements[layer] - 1]->output = 1.0F;
    };
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//    Train the net, use the correct training algorithm
//

void NeuralNet::Train(TRAINSET_VEC &vTrainSet, int volatile *iTrain)
{
    bUseRProp ? TrainRProp(vTrainSet, iTrain) : TrainBackProp(vTrainSet, iTrain);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//  Train the network, pass in an array of structures
///////////////////////////////////////////////////////////////////////////////

void NeuralNet::Train(struct sTrainSet **pTrainSet, int iSize, int volatile *iTrain)
{
    TRAINSET_VEC vTrainSet;

    // Create our own TRAINSET_VEC and pass it to the overloaded Train method
    for(int iX = 0; iX < iSize; iX++)
        vTrainSet.push_back(pTrainSet[iX]);

    Train(vTrainSet, iTrain);
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	Learn by using Resilient Back Propagation.
//  Inputs must be set by SetNeuronOutput prior to doing this.
//
//	NOTE: Since outputs	are scaled to 0 to 1, the allowable error is actually a
//		  percentage of the output.  For example, if the range of an output is
//		  10 (0 to 10), then the actual output after training for a result of 5
//   	  with an allowable error of .05 (5%) could be from 4.5 to 5.5, 
//		  5 +- (.05 * 10).  However, note that the "total error" is the sum
//		  of all errors in the epoch (set), so in practice an individual error 
//		  is rarely that high.
//
//	Note that when using Rprop, we use "epoch" or "batch" learning, 
//	i.e. we adjust the weights after all patterns in the set are calculated.
//	As such, we calculate the error here, but adjust the weights later.
//

void NeuralNet::LearnRProp(double *target)
{
    unsigned int layer;
    unsigned int pe;
    double dTarget;

    // Process loaded inputs
	// *** PARALLELIZE
    ProcessData();

    // Back propagate starting with the output layer
    layer = sublayer;

    // Calculate the error weight (EW)
	// *** PARALLELIZE
    for(pe = 0; pe < elements[layer]; pe++)
    {
        dTarget = target[pe];

        PElement[layer][pe]->CalculateOuterEWRProp(dTarget, PElement[layer - 1]);

        // This is "more correct"
        total_error += PElement[layer][pe]->ea * PElement[layer][pe]->ea;
    };

    // Now calculate EWs for hidden layers (not the input layer)
	// *** PARALLELIZE
    while(--layer) // Next innermost layer
    {
        for(pe = 0; pe < elements[layer]; pe++)
        {
            PElement[layer][pe]->CalculateHiddenEWRProp(PElement[layer - 1], PElement[layer + 1], elements[layer + 1]);
        };
    };
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//    RProp training function
//

void NeuralNet::TrainRProp(TRAINSET_VEC &vTrainSet, int volatile *iTrain)
{
    int iTotalSets;
    int iSet;
    unsigned int uNeuron;
    struct sTrainSet *pcTrainSet;
    unsigned long dwElapsed;
    unsigned long dwStart;
    long lLastIt;

    iTotalSets = vTrainSet.size();
    if (!iTotalSets)
        return;

    // Scale the neurons
	// *** PARALLELIZE
    ScaleNeurons(vTrainSet);

    iTrained = TRAINING;
    prev_total_error = MAX_DOUBLE;
    dwStart = Neuron::GetMilliseconds();
	
    lLastIt = 0;
    do
    { // Start the training process
        total_error = 0.0f;
        for(iSet = 0; iSet < iTotalSets; iSet++)
        { // For this set . . .
            pcTrainSet = vTrainSet[iSet];
            // Load the inputs
			// *** PARALLELIZE
            for(uNeuron = 0; uNeuron < pelements[0]; uNeuron++)
                PElement[0][uNeuron]->output = pcTrainSet->dScaledInputs[uNeuron];
            // Feed forward and calculate the error					
			// *** PARALLELIZE
            LearnRProp(pcTrainSet->dScaledOutputs);
        };

        // Get Root Mean Square of error
        total_error /= iTotalSets;
        total_error = ::sqrt(total_error);

        // Adjust the weights for all training sets (batch or "epoch" mode)
        int layer = layers;
        while(--layer) // Adjust weights	
        {
			// *** PARALLELIZE
            for(int pe = 0; pe < (int) elements[layer]; pe++)
            {
                PElement[layer][pe]->AdjustWeightsRProp(PElement[layer - 1]);
            };
        };

        prev_total_error = total_error;

        if (*iTrain != START_TRAIN)
            break;

        ++lIteration;

        // Save "total_error" after each iteration to provide a meaningful 
        // progress report variable.
        fErrorDisplay = total_error;
        UpdateStatus(false, *iTrain);
    }
    while(total_error > error);

    if (iTrained != UNABLE_TO_CONVERGE)
        iTrained = total_error <= error ? TRAINED : PARTIALLY_TRAINED;

    dwElapsed = Neuron::GetMilliseconds() - dwStart;
    dwTotalElapsed += dwElapsed;

    UpdateStatus(true, *iTrain);
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	Learn by back propagating.
//  Inputs must be set by SetNeuronOutput prior to doing this.
//

void NeuralNet::LearnBackProp(double *target)
{
    unsigned int layer;
    unsigned int pe;
    double dTarget;

    // Process loaded inputs
    ProcessData();

    // Backpropagate starting with the output layer
    layer = sublayer;

    // Calculate the error weight (EW)
    for(pe = 0; pe < elements[layer]; pe++)
    {
        dTarget = target[pe];

        PElement[layer][pe]->CalculateOuterEWBackProp(dTarget, PElement[layer - 1]);

        // This is "more correct"
        total_error += PElement[layer][pe]->ea * PElement[layer][pe]->ea;
    };

    // Now calculate the EWs for hidden layers (not the input layer)
    while(--layer) // Next innermost layer
    {
        for(pe = 0; pe < elements[layer]; pe++)
        {
            PElement[layer][pe]->CalculateHiddenEWBackProp(PElement[layer - 1], PElement[layer + 1], elements[layer + 1]);
        };
    };

    layer = layers;
    while(--layer) // Adjust weights
    {
        for(pe = 0; pe < elements[layer]; pe++)
        {
            PElement[layer][pe]->AdjustWeightsBackProp(PElement[layer - 1], delta);
        };
    };
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//    Backprop training function
//

void NeuralNet::TrainBackProp(TRAINSET_VEC &vTrainSet, int volatile *iTrain)
{
    int iTotalSets;
    int iSet;
    unsigned int uNeuron;
    struct sTrainSet *pcTrainSet;
    unsigned long dwElapsed;
    unsigned long dwStart;
    long lLastIt;

    iTotalSets = vTrainSet.size();
    if (!iTotalSets)
        return;

    // Scale the neurons
    ScaleNeurons(vTrainSet);

    iTrained = TRAINING;
    dwStart = Neuron::GetMilliseconds();

    lLastIt = 0;
    do
    {
        total_error = 0.0f;
        for(iSet = 0; iSet < iTotalSets; iSet++)
        {
            pcTrainSet = vTrainSet[iSet];
            for(uNeuron = 0; uNeuron < pelements[0]; uNeuron++)
                PElement[0][uNeuron]->output = pcTrainSet->dScaledInputs[uNeuron];
            LearnBackProp(pcTrainSet->dScaledOutputs);
        };

        if (*iTrain != START_TRAIN)
            break;

        // Get Root Mean Square of error
        total_error /= iTotalSets;
        total_error = ::sqrt(total_error);

        ++lIteration;

        // total_error gets updated per neuron.  Save after each
        // iteration to provide a meaningful progress report variable.
        fErrorDisplay = total_error;
        UpdateStatus(false, *iTrain);
    }
    while(total_error > error);

    if (iTrained != UNABLE_TO_CONVERGE)
        iTrained = total_error <= error ? TRAINED : PARTIALLY_TRAINED;

    dwElapsed = Neuron::GetMilliseconds() - dwStart;
    dwTotalElapsed += dwElapsed;

    UpdateStatus(true, *iTrain);
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//     Reset net parameters
//

void NeuralNet::ResetNetParameters(double dError, double dMomentum,
    double dGainf, double dLearnRate, bool bRProp)
{
    error = dError;
    momentum = dMomentum;
    dGain = dGainf;
    delta = dLearnRate;
    bUseRProp = bUseRProp;
};
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
//    Get the connections per second
//

unsigned long NeuralNet::GetConnectionsPerSecond()
{
    return(0);
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//	Get the connections/sec
//

const char *NeuralNet::GetElapsedTime()
{
    return(NULL);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//	Calculate standard deviation
//

bool NeuralNet::CalculateStandardDeviation(DOUBLE_VEC &vDoubles, double &dMean, double &dStdDev)
{
    int iX;
    double dWork;
    int iDataPoints;
    double dSum = 0.0f;
    double dSST = 0.0f;


    // Make sure both have elements
    // Save the numner of data points
    iDataPoints = vDoubles.size();

    if (!iDataPoints)
        return(false);

    // Get the sums of the table
    for(iX = 0; iX < iDataPoints; iX++)
        dSum += vDoubles[iX];

    // Get the mean of the tables
    dMean = dSum / iDataPoints;

    // Get the Sum of the Squares for each table
    for(iX = 0; iX < iDataPoints; iX++)
        dSST += ((vDoubles[iX] - dMean) * (vDoubles[iX] - dMean));

    // Get the standard deviation
    dWork = dSST / (iDataPoints - 1);

    dStdDev = ::sqrt(dWork);

    return(true);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//    End of: NETWORK.CPP
///////////////////////////////////////////////////////////////////////////////




