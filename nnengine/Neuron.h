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
//////////////////////////////////////////////////////////////////////////////
//!    NEURON.H - Neural network processing element object header
///////////////////////////////////////////////////////////////////////////////

#ifndef __NEURON_H
#define __NEURON_H

#include <time.h>
#include <string>

using namespace std;

#define DEFAULT_GAIN            0.65 // Sigmoid scaling constant
#define DEFAULT_LEARN_RATE      0.6
#define DEFAULT_MOMENTUM        0.75                                        
#define DEFAULT_ALLOWABLE_ERROR 0.05

// For RPROP algorithm
#define N_PLUS		1.2f
#define N_MINUS		0.5f
#define N_MAX		50.0f
#define N_MIN		0.000001f

class NeuralNet;

///////////////////////////////////////////////////////////////////////////////
//!   Neuron object - base object of network
//

class Neuron
{
    friend class NeuralNet;
public:
    //! Constructor
    Neuron(NeuralNet *AParent, unsigned int num_inputs, unsigned int anid);

    //! Destructor
    ~Neuron();

    //! Allocate the label string and set it
    bool SetNeuronLabel(const char *cNewLabel);
    //! Get the label
    const char * GetNeuronLabel();
    //! Set a neuron's gain to a different value
    void SetNeuronGain(double dNewGain);

public:
    //! Net we're a part of
    NeuralNet *pNeuralNet; 
    //! Get time in milliseconds
    static unsigned long GetMilliseconds();

protected:
    //! Sigmoid unsigned transfer
    virtual double TransferFunction(double input);
    //! Combine inputs
    void CalculateOutput(Neuron **p_layer);
    //! Calculate error weight for outer layer - RProp
    virtual void CalculateOuterEWRProp(double target, Neuron **pl);
    //! Calculate error weight for hidden layer - RProp
    virtual void CalculateHiddenEWRProp(Neuron **pl, Neuron **nl, unsigned int elements);
    //! Adjust all neuron weights - RProp
    void AdjustWeightsRProp(Neuron **pl);
    //! Calculate error weight for outer layer - backprop
    virtual void CalculateOuterEWBackProp(double target, Neuron **pl);
    //! Calculate error weight for hidden layer - backprop
    virtual void CalculateHiddenEWBackProp(Neuron **pl, Neuron **nl, unsigned int elements);
    //! Adjust all neuron weights - backprop
    void AdjustWeightsBackProp(Neuron **pl, double delta);
    //! Before any training
    void RandomizeNeuronWeights();
    //! Scale in a value and set output to it
    void ScaleIn(double dTempInput);
    //! Scale what's in output and return it
    double ScaleOut();
    //! Return -1, +1, or 0
    double GetSign(double dValue);

protected: 
    //! Number of inputs
    unsigned int inputs; 
    //! Weights
    double *weights; 
    //! Previous error weight
    double *prev_ew; 
    //! Previous change to weight delta
    double *prev_adj; 
    //! The actual previous change to the weight
    double *prev_chg; 
    //! The actual previous change to the weight
    double *delta_weight; 
    //! Error derivative
    double ea; 
    //! Rate of error change
    double ei; 
    //! Error weights for each input
    double *ew; 
    //! Actual output
    double output; 
    //! Pure calculated output
    double sum; 
    //! Our layer
    unsigned int id; 
    //! Scaling factor for outputs
    double scale; 
    //! Offset for output 
    double offset; 
    //! Input prior to scaling if input neuron
    double dInput;
    //! Gain for this neuron
    double dGain; 
    //! For input and output neurons, the name
    string cLabel; 
};
///////////////////////////////////////////////////////////////////////////////

#endif

