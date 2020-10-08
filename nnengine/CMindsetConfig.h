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
#pragma once
#include "ExpatImpl.h"

#include <vector>
#include <string>

using namespace std;

#define MAX_ELEMENT_TEXT    1024 * 64        // One value should not be grater than 64K

class CMindsetConfig : public CExpat
{
public:
    CMindsetConfig(void);
    ~CMindsetConfig(void);

    // Read a file, fill a buffer, and parse it
    int ParseXMLFile(const char * lpFilePath);

public: // Variables
    int iLayers;
    vector<int> vLayerNodeCount;
    double dAllowableError;
    double dLearnRate;
    double dMomentum;
    double dGain;
    bool bRProp;
    vector<double> vInputMean;
    vector<double> vInputStdDev;
    vector<double> vOutputMean;
    vector<double> vOutputStdDev;
    vector<double> vWeights;
    vector<string> vLabels;
    int iTrainingStatus;
    string tLastError;
    bool bVerified;

protected:
    // Element start tag
    void OnStartElement(const XML_Char *pszName, const XML_Char **papszAttrs);
    // Element data
    void OnCharacterData(const XML_Char *pszData, int nLength);
    // Element end tag
    void OnEndElement(const XML_Char *pszName);

protected:
    string tElementText;
    char szValue[MAX_ELEMENT_TEXT];
    char *pBuffer;
    bool bFinish;
    bool bInput;
    bool bOutput;
    unsigned long lCRC;
    unsigned long lVerification;
};
