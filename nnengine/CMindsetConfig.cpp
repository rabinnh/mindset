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
#include "CMindsetConfig.h"
#include "Crc32.h"
#include <cstdio>

CMindsetConfig::CMindsetConfig(void) : CExpat()
{
    Create();
    EnableStartElementHandler();
    EnableCharacterDataHandler();
    EnableEndElementHandler();
    pBuffer = NULL;
    bInput = bOutput = false;
    vInputMean.clear();
    vInputStdDev.clear();
    vOutputMean.clear();
    vOutputStdDev.clear();
    vWeights.clear();
    vLabels.clear();
}

CMindsetConfig::~CMindsetConfig(void)
{
}

// Read a file, fill a buffer, and parse it

int CMindsetConfig::ParseXMLFile(const char * lpFilePath)
{
    FILE *pFile;
    unsigned long uLength;
    unsigned long nRead;
    CRC32 cCRC;

    if (lpFilePath == NULL)
    {
        tLastError = ("lpFilePath was NULL.");
        return(-1);
    };

    lCRC = 0xFFFFFFFF;
    lVerification = 0;

    try
    {
        // Read the file
        pFile = ::fopen(lpFilePath, ("rb"));
        if (!pFile)
        {
            tLastError = ("Unable to open the XML file.");
            throw(-1);
        };

        ::fseek(pFile, 0, SEEK_END);
        uLength = ::ftell(pFile);
        pBuffer = new char[uLength + 1];
        ::fseek(pFile, 0, SEEK_SET);
        bool bFinish = true;
        while(!feof(pFile))
        {
            nRead = (unsigned long) ::fread(pBuffer, 1, uLength, pFile);

            bFinish = (nRead < uLength);
            if (bFinish)
            { // NOTE: The Parse code has to have the null character if and only if 
                // it is the last chuck of XML to be processed.
                pBuffer[nRead] = '\0';
                nRead++;
            }
            if (Parse(pBuffer, nRead, bFinish) != 0 && bFinish)
            {
                const XML_LChar *pError = GetErrorString();
                tLastError = pError != NULL ? pError : ("Unknown");
                throw(-1);
            }
        }
        ::fclose(pFile);
        delete[] pBuffer;
        pBuffer = NULL;
    }
    catch(int e)
    {
        if (pFile)
            ::fclose(pFile);
        if (pBuffer)
            delete[] pBuffer;
        return(e);
    }

    // Just do a straight read down to the verification tag
    try
    { // Read the file
        pFile = ::fopen(lpFilePath, ("rb"));
        if (!pFile)
        {
            tLastError = ("Unable to open the XML file.");
            throw(-1);
        };

        // Read the file
        ::fseek(pFile, 0, SEEK_END);
        uLength = ::ftell(pFile);
        pBuffer = new char[uLength + 1];
        ::fseek(pFile, 0, SEEK_SET);
        bool bFinish = true;
        nRead = (unsigned long) ::fread(pBuffer, 1, uLength, pFile);
        // Find the beginning of the <verification> tag
        char *pEnd = ::strstr(pBuffer, "<Verification>");
        if (pEnd)
        {
            *pEnd = '\0';
            // Get the CRC
            lCRC = cCRC.CalcCRC(lCRC, (unsigned char *) pBuffer, ::strlen(pBuffer));
        };
        ::fclose(pFile);
        delete[] pBuffer;
        pBuffer = NULL;
    }
    catch(int e)
    {
        if (pFile)
            ::fclose(pFile);
        if (pBuffer)
            delete[] pBuffer;
        return(e);
    }

    // Cheack the verification
    bVerified = lCRC == lVerification;

    return 0;
}

// Got a starting element tag

void CMindsetConfig::OnStartElement(const XML_Char *pszName, const XML_Char **papszAttrs)
{ // Clear element value placeholder and flags
    tElementText.clear();

    // Check to see if there are any attributes
    if (!papszAttrs)
        return;

    // Check the element.  
    // If configuration, then the number of layers are in the attribute
    if (!::strcasecmp(pszName, "Configuration"))
    {
        if (!::strcasecmp(papszAttrs[0], "Layers"))
            iLayers = ::atoi(papszAttrs[1]);
    }
    else if (!::strcasecmp(pszName, "Layer"))
    { // We don't really care about "type"
    }
    else if (!::strcasecmp(pszName, "Input"))
    {
        bInput = true;
    }
    else if (!::strcasecmp(pszName, "Output"))
    {
        bOutput = true;
    };
}


// Get the element character data

void CMindsetConfig::OnCharacterData(const XML_Char *pszData, int nLength)
{
    // Make sure length is OK
    if (nLength >= MAX_ELEMENT_TEXT)
        return;

    // Ignore line feeds
    while(*pszData == '\n' && nLength)
    {
        *pszData++;
        nLength--;
    };
    if (!nLength)
        return;

    // Copy
    strncpy(szValue, pszData, nLength);
    szValue[nLength] = '\0';

    // Add it to our existing string
    tElementText += szValue;
}

// Got the end data element tag

void CMindsetConfig::OnEndElement(const XML_Char *pszName)
{
    //Get element text
    if (!strcmp(pszName, "Layer"))
    { // Get the number of nodes
        vLayerNodeCount.push_back(::atoi(tElementText.c_str()));
    }
    else if (!strcmp(pszName, "Label"))
    {
        vLabels.push_back(tElementText);
    }
    else if (!strcmp(pszName, "AllowableError"))
    {
        dAllowableError = ::atof(tElementText.c_str());
    }
    else if (!strcmp(pszName, "LearnRate"))
    {
        dLearnRate = ::atof(tElementText.c_str());
    }
    else if (!strcmp(pszName, "Gain"))
    {
        dGain = ::atof(tElementText.c_str());
    }
    else if (!strcmp(pszName, "Momentum"))
    {
        dMomentum = ::atof(tElementText.c_str());
    }
    else if (!strcmp(pszName, "UseRProp"))
    {
        bRProp = ::atoi(tElementText.c_str());
    }
    else if (!strcmp(pszName, "TrainingStatus"))
    {
        iTrainingStatus = ::atoi(tElementText.c_str());
    }
    else if (!strcmp(pszName, "Input"))
    {
        bInput = false;
    }
    else if (!strcmp(pszName, "Output"))
    {
        bOutput = false;
    }
    else if (!strcmp(pszName, "Mean"))
    {
        if (bInput)
            vInputMean.push_back(::atof(tElementText.c_str()));
        if (bOutput)
            vOutputMean.push_back(::atof(tElementText.c_str()));
    }
    else if (!strcmp(pszName, "StdDev"))
    {
        if (bInput)
            vInputStdDev.push_back(::atof(tElementText.c_str()));
        if (bOutput)
            vOutputStdDev.push_back(::atof(tElementText.c_str()));
    }
    else if (!strcmp(pszName, "Weights"))
    { // Tokenize and parse the weights
        // Create a string to tokenize
        char *szBuffer = new char[tElementText.size() + 1];
        char *pToken = NULL;

        // Copy and tokenize
        ::strcpy(szBuffer, tElementText.c_str());
        pToken = ::strtok(szBuffer, ",");
        while(pToken)
        {
            vWeights.push_back(::atof(pToken));
            pToken = ::strtok(NULL, ",");
        };
        delete[] szBuffer;
    }
    else if (!strcmp(pszName, "Verification"))
    {
        lVerification = ::atol(tElementText.c_str());
    }
}



