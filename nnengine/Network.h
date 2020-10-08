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

    NETWORK.H - Neural network object header

	To do:
        1) Allow user to specify number of iterations for training
		2) Check error every X epochs to see if error has been reduced. If not, stop training.
			a) Check box to activate/deactivate this feature
		3) Dynamic topology?
		4) Check tanh (use Java code example)

    NOTE: During batch training, one technique is to drop patterns once the error for
		  that pattern drops below the error threshold and only train on the remaining
		  patterns.  Very questionable!	
*/

#ifndef __NETWORK_H
#define __NETWORK_H

#define		MAX_DOUBLE          (double) (1.7E308)
#define     UNTRAINED           0
#define     PARTIALLY_TRAINED   1
#define     TRAINING            2
#define     TRAINED             3
#define     UNABLE_TO_CONVERGE  4

#include "Neuron.h"
#include "Trainset.h"
#include "math.h"

typedef vector<double> DOUBLE_VEC;
typedef void (*NN_CALLBACK)(unsigned int epochs, double error, void *pUserParm);

///////////////////////////////////////////////////////////////////////////////
//!    NeuralNet object - The actual net
//
class NeuralNet
{
    friend class Neuron;
public: 
    //! Constructor
    NeuralNet();

    //! Destructor
    ~NeuralNet();

    enum
    {   //! Orders to training thread
        STOP_NODISPLAY,
        STOP_DISPLAY,
        START_TRAIN
    };

    enum
    {   //! Identify the layer a neuron belongs to 
        INPUT = 0,
        HIDDEN = 1,
        OUTPUT = 2
    };

    //! Create the network from entered parameters
    virtual int Create(unsigned int num_layers, unsigned int *layer_elements, double learn_rate, 
                double allowable_error, double momentum_factor, double gain_f = DEFAULT_GAIN, bool bRProp = false);
    //! Create a network from a saved network file.  This will include saved weights.
    virtual int Create(const char *szNetworkFile);
	// Register a progress callback function, function, a user parameter
	void RegisterProgressCallback(NN_CALLBACK pFunc, void *pUserParm, unsigned int uEpochCount = 10000);
    //! Save network
    virtual int SaveNetwork(const char *szNetworkFile);
    //! Set all inputs and tell neurons to scale them.  This is for inputs that are NOT pre-scaled
    int SetInputsScaled(double *inputs);
    //! Return outputs from output layer, scaled to our dimensions
    int GetFinalOutputsScaled(double *outputs);
    //! Process the data if input layer is loaded with SetNeuronOutput
    virtual void ProcessData();
    //! Reset network parameters
    void ResetNetParameters(double dError, double dMomentum, double dGainf, double dLearnRate, bool bRProp);
    //! Train the network, pass in a vector of structures
    virtual void Train(TRAINSET_VEC &vTrainSet, int volatile *iTrain);
    //! Train the network, pass in an array of structures
    virtual void Train(struct sTrainSet **pTrainSet, int iSize, int volatile *iTrain);
    //! Process these inputs - this is for pre-scaled inputs
    virtual int ProcessInputs(double *inputs);
    //! Created successfully?
    bool Created();
    //! Get the connections/sec
    unsigned long GetConnectionsPerSecond();
    //! Get the elapsed time for training
    const char *GetElapsedTime();

public:
    //! Neuron layers
    Neuron ***PElement;
    //! For external use
    unsigned int *pelements;
    //! Number of inputs in input layer
    unsigned int num_inputs;
    //! Number of layers    
    unsigned int layers; 
    //! Allowable error    
    double error;
    //! Learn rate
    double delta; 
    //! Momentum factor
    double momentum; 
    //! The gain
    double dGain; 
    //! Is the net trained
    int iTrained; 
    //! Total connections (including BIAS units)
    long lTotalConnects; 
    //! How many iterations during training.
    long lIteration; 
    //! Saved total error during training for status display
    double fErrorDisplay; 
    //! Total elapsed time
    unsigned long dwTotalElapsed; 
    //! Use the RProp algorithm for training
    bool bUseRProp; 
    //! If created using parse XML.
    bool bVerified; 

protected:
    //! Set the inputs
    int SetInputs(double *inputs);
    //! Get scaling factors (offset and scale) for inputs and outputs
    bool ScaleNeurons(TRAINSET_VEC &vTrainSet);
    //! Get a particular neuron's output
    double GetNeuronOutput(unsigned int layer, unsigned int pe);
    //! Return outputs from output layer - array must be large enough
    int GetFinalOutputs(double *outputs);
    //! Set neuron output (for input layer)
    int SetNeuronOutput(unsigned int layer, unsigned int pe, double out);
    //! Set neuron output (for input layer) and tell neuron to scale it
    int SetNeuronOutputScaled(unsigned int layer, unsigned int pe, double out);
    //! This allows a derived class to have NeuralNet allocate a derived Neuron
    virtual Neuron *AllocateNeuron(NeuralNet *AParent, unsigned int num_inputs, unsigned int element, unsigned int uType);
    //! Set stored weights.
    int SetWeight(unsigned int layer, unsigned int pe, unsigned int input, double weight);
    //! Get calculated weights, used after learning.
    double GetWeight(unsigned int layer, unsigned int pe, unsigned int input);
    //! Set all inputs in preparation for learning
    void RandomizeWeights();
    //! Train the network
    virtual void TrainBackProp(TRAINSET_VEC &vTrainSet, int volatile *iTrain);
    //! Train the network
    virtual void TrainRProp(TRAINSET_VEC &vTrainSet, int volatile *iTrain);
    //! For each layer
    virtual void LearnBackProp(double *target);
    //! For each layer
    virtual void LearnRProp(double *target);
    //! Update training status.  Implement to get a callbeck to display status.
    virtual void UpdateStatus(bool bErase, int iTrain);
    //! Calculate the standard deviation
    bool CalculateStandardDeviation(DOUBLE_VEC &vDoubles, double &dMean, double &dStdDev);
    //! Write a line in the file and update the CRC
    int FileWriteLine(FILE *pFile, char *pBuffer, unsigned long &lCRC);

protected:
    //! Total error
    double total_error;
    //! For adjustable learning rate
    double prev_error; 
    //! For internal use, this count includes bias units
    unsigned int *elements;
    //! Shortcut which is equal (layers - 1)
    unsigned int sublayer; 
    //! The lowest error we've achieved
    double dLowestError; 
    //! Creation flag
    bool bCreated; 
    //! Used for RProp weight backtracking
    double prev_total_error; 
    //! We have to scale the inputs and outputs when the user uses the net, so save the values that we need			
    //! Save the mean for all outputs
    DOUBLE_VEC vInputMean; 
    //! Save the standard deviation for all outputs
    DOUBLE_VEC vInputStdDev;
    //! Save the mean for all outputs
    DOUBLE_VEC vOutputMean; 
    //! Save the standard deviation for all outputs
    DOUBLE_VEC vOutputStdDev; 
	// Callback variables
	NN_CALLBACK pCallback;
	void *pUserParm;
	unsigned int uEpochCount;
};
///////////////////////////////////////////////////////////////////////////////

#endif


