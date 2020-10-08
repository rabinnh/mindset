
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
//    NEURON.CPP - Neural network processing element 
///////////////////////////////////////////////////////////////////////////////
//    Net design notes:
//        For statistical analysis, number of hidden neurons should
//        be about 4 x number of input units.  If there are a
//        sufficient number, more than 1 hidden layer is possible.
//    Total number of weights should be less than number of test
//        patterns * bits required (integer) to represent the data.
//        See pg. 172 of Sept, 1992 Scientific American.
//////////////////////////////////////////////////////////////////////////////

#include "Network.h"
#include "Neuron.h"
#include "math.h"
#include <cstdlib>
#include <sys/time.h>
#include <iostream>
#include "omp.h"

//#define USE_TANH // doesn't work


///////////////////////////////////////////////////////////////////////////////
//    Neuron constructor
//

Neuron::Neuron(NeuralNet *AParent, unsigned int num_inputs, unsigned int anid)
{
    // Load values
    pNeuralNet = AParent; // Our net
    inputs = num_inputs; // Number of inputs
    weights = new double[inputs]; // Weights
    prev_ew = new double[inputs]; // Current adjustments
    prev_adj = new double[inputs]; // Previous adjustments
    delta_weight = new double[inputs]; // Previous weight change
    ew = new double[inputs]; // Error change
    ei = ea = 0.0f; // Error variables
    id = anid; // Element in the layer
    dGain = pNeuralNet->dGain; // Gain is equal to default
    offset = 0;

    // Initialize weight deltas
    for(int count = 0; count < (int) inputs; count++)
        prev_adj[count] = 0.1f;
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//     Destructor

Neuron::~Neuron()
{
    delete[] weights;
    delete[] prev_ew;
    delete[] prev_adj;
    delete[] delta_weight;
    delete[] ew;
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//      Randomize the weights in preparation for learning
//

void Neuron::RandomizeNeuronWeights()
{
    unsigned int count;
    double w;

    // Initialize random number generator
    ::srand(time(NULL));

    // Assign weights.  
    // The formula for assigning initial weights is -2.4/I < w > 2.4/I where I is # of inputs
    for(count = 0; count < inputs; count++)
    {
        w = (double) rand();
        w = (int) w % 48;
        w /= 10;
        w -= 2.4f;
        w /= inputs;

	    weights[count] = w;
        ew[count] = delta_weight[count] = prev_ew[count] = 0.0f;
        prev_adj[count] = 0.1f;
    };
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//      Change a neuron's gain
//

void Neuron::SetNeuronGain(double dNewGain)
{
    dGain = dNewGain;
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//     Calculate output from all inputs
//

void Neuron::CalculateOutput(Neuron **p_layer)
{
    unsigned int count;

    // Adaptive linear combiner
    sum = 0.0f;

    for(count = 0; count < inputs; count++)
        sum += weights[count] * p_layer[count]->output;

    output = TransferFunction(sum);
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//      Transfer function for SIGMOID_UNSIGNED
//

double Neuron::TransferFunction(double input)
{
    double tf_transfer;

    // Sigmoid function
#ifdef USE_TANH
    tf_transfer = (double) ::tanh(input); // dGain?  Yes, should be '::tanh(-dGain * input)'
#else
    tf_transfer = (double) (1 / (1 + ::exp(-dGain * input)));
#endif

    return(tf_transfer);
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//     Calculate the EW for output layer
//

void Neuron::CalculateOuterEWBackProp(double target, Neuron **p_layer)
{
    unsigned int count; // Counter

    // Error derivative
    ea = target - output; // Our error

    // Derivative of the SIGMOID function
#ifdef USE_TANH
	// This doesn't look like the proper derivative.  S/B (1.0f - (::tanh(output) * ::tanh(output)
    ei = ea * (1.0f - (output * output));  
#else
    ei = ea * output * (1.0f - output);
#endif

    // Error weight
    for(count = 0; count < inputs; count++)
        ew[count] = ei * p_layer[count]->output;

};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//     Calculate the EW for hidden layers
//

void Neuron::CalculateHiddenEWBackProp(Neuron **p_layer, Neuron **n_layer, unsigned int elements)
{ // elements is number of neurons in n_layer (next layer)
    unsigned int count;

    // Calculate EA for this layer by adding
    // next layer's EI * weight for this neuron output
    ea = 0.0f;
    for(count = 0; count < elements; count++)
        ea += n_layer[count]->ei * n_layer[count]->weights[id];

    // Derivative of the SIGMOID function
#ifdef USE_TANH
	// This doesn't look like the proper derivative.  S/B (1.0f - (::tanh(output) * ::tanh(output))
    ei = ea * (1.0f - (output * output));
#else
    ei = ea * output * (1.0f - output);
#endif

    // Now calculate derivative error weight for each input
    for(count = 0; count < inputs; count++)
        ew[count] = ei * p_layer[count]->output;
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//  Make the neuron learn by adjusting weights
//
//	Note that when doing straight back-prop, we use "pattern" learning, 
//	i.e. we adjust the weights after each pattern is fed forward and the error calculated.
//

void Neuron::AdjustWeightsBackProp(Neuron **p_layer, double delta)
{
    unsigned int count;
    double adjust;

    for(count = 0; count < inputs; count++)
    { // Calculated from formulas plus a little extra oomph
        adjust = (delta * ew[count] * p_layer[count]->output) + (pNeuralNet->momentum * prev_adj[count]);
        weights[count] += adjust;
        prev_adj[count] = adjust;
    };
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//     Calculate the EW for output layer
//

void Neuron::CalculateOuterEWRProp(double target, Neuron **p_layer)
{
    unsigned int count; // Counter

    // Error derivative
    ea = target - output; // Our error

    // Derivative of the SIGMOID function
#ifdef USE_TANH
	// See: http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=227257&url=http%3A%2F%2Fieeexplore.ieee.org%2Fiel2%2F632%2F5902%2F00227257
	// This doesn't look like the proper derivative.  S/B (1.0f - (::tanh(output) * ::tanh(output))
    ei = ea * (1.0f - (output * output));
#else
    ei = ea * output * (1.0f - output);
#endif

    // Error weight
    // NOTE: RProp modification - sum error weights

    for(count = 0; count < inputs; count++)
        ew[count] -= ei * p_layer[count]->output;
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//     Calculate the EW for hidden layers
//

void Neuron::CalculateHiddenEWRProp(Neuron **p_layer, Neuron **n_layer, unsigned int elements)
{ // elements is number of neurons in n_layer (next layer)
    unsigned int count;

    // Calculate EA for this layer by adding
    // next layer's EI * weight for this neuron output
    ea = 0.0f;
    for(count = 0; count < elements; count++)
        ea += n_layer[count]->ei * n_layer[count]->weights[id];

    // Derivative of the SIGMOID function
#ifdef USE_TANH
	// This doesn't look like the proper derivative.  S/B (1.0f - (::tanh(output) * ::tanh(output))
    ei = ea * (1.0f - (output * output));
#else
    ei = ea * output * (1.0f - output);
#endif

    // Now calculate derivative error weight for each input
    // NOTE: RProp modification - sum error weights
    for(count = 0; count < inputs; count++)
        ew[count] -= ei * p_layer[count]->output;
};
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//  Make the neuron learn by adjusting weights, 
//	using the RPROP (Resilient Backpropagation) algorithm
//
//	Note that when use Rprop, we use "epoch" or "batch" learning, 
//	i.e. we adjust the weights after all patterns in the set are calculated.
//

void Neuron::AdjustWeightsRProp(Neuron **p_layer)
{
    unsigned int count;
    double dSign;

    for(count = 0; count < inputs; count++)
    { // RPROP adjustment.  Note that in normal "batch" training
        // we would divide the accumulated error by the number of patterns. 
        // However, RProp is only interested in the sign, so this isn't necessary.
        dSign = GetSign(ew[count] * prev_ew[count]);
        if (dSign > 0) // Error weight sign changed to positive
        {
            prev_adj[count] *= N_PLUS;
            if (prev_adj[count] > N_MAX)
                prev_adj[count] = N_MAX;
            // Save our change in weight size
            delta_weight[count] = -(GetSign(ew[count])) * prev_adj[count];
            weights[count] += delta_weight[count];
            prev_ew[count] = ew[count];
        }
        else
        {
            if (dSign < 0) // Error weight sign change to negative
            {
                prev_adj[count] *= N_MINUS;
                if (prev_adj[count] < N_MIN)
                    prev_adj[count] = N_MIN;
                // Weight backtracking added from paper by Christian Igel and Michael Husken
                // NOTE, I'm not absolutely sure that the batch error is being computed correctly.
                //		 If it isn't, this code may "hide" that fact by correcting incorrect adjustments.
                //       The effect is that the training may be slower than it otherwise would be.
                if (pNeuralNet->total_error > pNeuralNet->prev_total_error)
                    weights[count] -= delta_weight[count];
                prev_ew[count] = 0.0f;
            }
            else // No sign change in error weight
                delta_weight[count] = -(GetSign(ew[count])) * prev_adj[count];
            weights[count] += delta_weight[count];
            prev_ew[count] = ew[count];
        };
        ew[count] = 0.0f; // Prepare for next batch
    };
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//  Get an input and scale it in.  
//	This is only used for the input layer when solving a problem. 
//	Training sets are prescaled.
//

void Neuron::ScaleIn(double dTempInput)
{
    double dWork;

    dInput = dTempInput;

    dWork = dInput - pNeuralNet->vInputMean[id];
    dWork /= pNeuralNet->vInputStdDev[id];
    dWork += 3; // So make them all positive
    dWork /= 2; // and between 0 and 3
    if (dWork < 0)
        dWork = 0;

    output = dWork;
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//     Scale the output and return it to the caller.  
//     This is only used for the output layer.
//

double Neuron::ScaleOut()
{
    double dWork;
    double dMean;
    double dStdDev;

    dMean = pNeuralNet->vOutputMean[id];
    dStdDev = pNeuralNet->vOutputStdDev[id];
    dWork = output * 6;
    dWork -= 3;
    dWork *= dStdDev;
    dWork += dMean;

    return(dWork);
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//     Set the neuron label
//     
//

bool Neuron::SetNeuronLabel(const char * cNewLabel)
{
    cLabel = cNewLabel;
    return(true);
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//		Get the label
//     
//

const char * Neuron::GetNeuronLabel()
{
    return(cLabel.c_str());
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//    Return the sign of a value )(-1, +1, 0)
//

double Neuron::GetSign(double dValue)
{
    if (dValue > 0)
        return 1.0f;
    if (dValue < 0)
        return -1.0f;
    return 0.0f;
};


///////////////////////////////////////////////////////////////////////////////
//    Get the time in ms
//

unsigned long Neuron::GetMilliseconds()
{
    struct timeval curr;

    unsigned long mtime;

    ::gettimeofday(&curr, NULL);
    mtime = (((curr.tv_sec) * 1000) + (curr.tv_usec / 1000.0)) + 0.5;

    return(mtime);
}

///////////////////////////////////////////////////////////////////////////////
//     End of: NEURON.CPP
///////////////////////////////////////////////////////////////////////////////


