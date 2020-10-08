// DIFObj.cpp : DIF interpretor object class
//
/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <string.h>
#include <cstdlib>
#include "Difobj.h"


static const char *szHeader[7] = {"TABLE", "VECTORS", "TUPLES", "DATA", "LABEL", "BOT", "EOD"};

static const char *szParseError[14] = {"Invalid file", "Invalid label", "TABLE out of sequence",
	"VECTOR out of sequence", "Bad VECTOR value",
	"VECTOR value is 0", "TUPLES out of sequence",
	"TUPLE value is 0", "Invalid TUPLE value",
	"Not enough TUPLES",
	"Invalid file format", "Not enough memory to parse the file",
	"Invalid vector string, cannot convert to numeric",
	"Unknown DIF header type"};
static const char *szExcel = "Excel";

/////////////////////////////////////////////////////////////////////////////
// Read and parse the DIF file

CDIFObject::CDIFObject(const char *lpszFileName)
{ // Initialize variables
	szTitle[0] = szBuffer[0] = '\0';
	this->szFile = lpszFileName;
	uSets = uFields = 0;
	dData = NULL;
	szLabel = NULL;
	uFieldType = NULL;
};

bool CDIFObject::ParseDIFFile()
{
	char *szString; // Temporary string
	double dWork; // Temp double variable
	int iHeader; // The header type
	int iSequence; // The order we get the headers
	unsigned int uRecord; // # of records (TUPLES)
	unsigned int uIndex; // # of fields (VECTORS)
	bool bGotLabels; // Did we already get fields labels?
	bool bGotaLabel; // Got one label
	bool bGotData; // Was there numeric data in this set?
	bool bExcel; // Bug in Excel swaps TUPLE and VECTOR values
	string line;

	// Start at the front
	ifstream ifDIFile(szFile);

	iSequence = 0;
	dData = NULL;
	szLabel = NULL;
	bExcel = false;

	try
	{
		// Parse the data portion of the file, getting file information
		do
		{ // Read a header
			getline(ifDIFile, line);
			if (!line.length())
				throw(INVALID_FILE); // Faulty file
			for (iHeader = TABLE; iHeader <= EOD; iHeader++)
			{
				if (!strncmp(line.c_str(), szHeader[iHeader], strlen(szHeader[iHeader])))
					break;
			};

			// Invalid header?
			if (iHeader > EOD)
				throw(INVALID_LABEL);

			// Increment iSequence
			iSequence++;

			// Process the header record 
			switch(iHeader)
			{
				case TABLE: // Skip next 2 records
					if (iSequence != 1)
						throw(TABLE_OUTOFSEQ); // Table record out of sequence error
					getline(ifDIFile, line);
					// Read the alpha record, table title
					getline(ifDIFile, line);
					szString = (char *) line.c_str();
					::strncpy(szBuffer, szString, sizeof(szBuffer));
					// Replace \n with 0
					if (szString)
					{
						szString = strchr(szBuffer, '"');
						if (szString)
						{
							strcpy(szTitle, szString + 1);
							szString = strchr(szTitle, '"');
							if (szString)
								*szString = '\0';
						}
						else
						{
							strcpy(szTitle, szBuffer);
						};
						szString = strchr(szTitle, '\n');
						if (szString)
							*szString = '\0';
					};
					// See if this is Excel
					if (!strncasecmp(szTitle, szExcel, strlen(szExcel)))
						bExcel = true;
					break;
				case VECTORS: // Get the number of ALL sets (including alpha)
					if (iSequence != 2)
						throw(VECTOR_OUTOFSEQ); // Vector record out of sequence
					getline(ifDIFile, line);
					szString = (char *) line.c_str();
					if (!szString)
						throw(BAD_VECTOR_VALUE);
					if (szString[0] != '0')
						throw(BAD_VECTOR_VALUE);

					// Convert it
					if (!GetNumericValue(szString, &dWork))
					{ // Bad vector value goes here
						throw(BAD_VECTOR_VALUE);
					};
					if ((uFields = (long) dWork) == 0)
					{ // No records in table message goes here
						throw(NO_VECTORS);
					};
					// Read the alpha record, chucked for this part of the file
					getline(ifDIFile, line);
					szString = (char *) line.c_str();
					break;
				case TUPLES: // Get the number of ALL columns
					if (iSequence != 3)
						throw(TUPLES_OUTOFSEQ); // TUPLES record out of sequence
					getline(ifDIFile, line);
					szString = (char *) line.c_str();
					if (!szString) // No TUPLES values
						throw(NO_TUPLES);
					if (szString[0] != '0')
						throw(NO_TUPLES);

					// Convert it
					if (!GetNumericValue(szString, &dWork))
					{ // Bad tuple value goes here
						throw(BAD_TUPLE_VALUE);
					};
					if ((uSets = (long) dWork) == 0)
					{ // No tuples in table message goes here
						throw(NO_TUPLES);
					};
					// Read the alpha record, chucked for this part of the file
					getline(ifDIFile, line);
					szString = (char *) line.c_str();
					break;
				case LABEL: // Ignore label types
				case DATA: // Signifies that rest of file is data
					if (iSequence < 4)
						throw(BAD_FORMAT); // Bad format
					getline(ifDIFile, line);
					szString = (char *) line.c_str();
					// Read the alpha record, chucked for this part of the file
					getline(ifDIFile, line);
					szString = (char *) line.c_str();
					break;
			}; // switch(iHeader)
		} // do
		while(iHeader != DATA);

		// We now have the parameters for the file, we need to process the data.
		// Allocate data space.  If some records turn out to be alpha,
		// we'll only waste a row or two.

		// If this is Excel, straighten out the values
		if (bExcel)
		{
			uRecord = uFields;
			uFields = uSets;
			uSets = uRecord;
		};

		// Allocate data space
		dData = new double*[uSets];
		if (!dData)
			throw(NOT_ENOUGH_MEM); // Not enough memory
		memset(dData, 0, sizeof(sizeof(double*) * uSets));
		for (uRecord = 0; uRecord < uSets; uRecord++)
		{
			dData[uRecord] = new double[uFields];
			if (!dData[uRecord])
				throw(NOT_ENOUGH_MEM); // Not enough memory
			memset(dData[uRecord], 0, sizeof(double) * uFields);
		};
		szLabel = new string[uFields];
		if (!szLabel)
			throw(NOT_ENOUGH_MEM); // Not enough memory
		uFieldType = new unsigned int[uFields];
		if (!uFieldType)
			throw(NOT_ENOUGH_MEM); // Not enough memory
		memset(uFieldType, 0, sizeof(unsigned int) * uFields);
		bGotLabels = bGotaLabel = false;
		uRecord = 0; // Count records
		uDataSets = 0;

		for (uRecord = 0; uRecord < uSets; uRecord++)
		{ // Now process the data.  The tough part is deciding whether
			getline(ifDIFile, line);
			szString = (char *) line.c_str();
			if (!szString) // Bad format
				throw(BAD_FORMAT);

			::strncpy(szBuffer, szString, sizeof(szBuffer));
			// Beginning of TUPLE starts with a -1,0 record
			if (szBuffer[0] != '-' && szBuffer[1] != '1')
				throw(BAD_FORMAT);

			// Next record should be a BOT (beginning of TUPLE) or an
			// EOD (end of data)
			getline(ifDIFile, line);
			szString = (char *) line.c_str();
			if (!szString)
				throw(BAD_FORMAT);
			::strncpy(szBuffer, szString, sizeof(szBuffer));
			// Determine the header type
			for (iHeader = BOT; iHeader <= EOD; iHeader++)
			{
				if (!strncmp(szBuffer, szHeader[iHeader], strlen(szHeader[iHeader])))
					break;
			};

			// If end of data, we're through                
			if (iHeader == EOD)
				break;

			// Has to be BOT if we're here
			if (iHeader != BOT)
				throw(BAD_FORMAT); // Invalid file format

			// Now read the data
			bGotData = false;
			for (uIndex = 0; uIndex < uFields; uIndex++)
			{
				getline(ifDIFile, line);
				szString = (char *) line.c_str();
				if (!szString)
					throw(NOT_ENOUGH_TUPLES); // Invalid number of TUPLES

				// Process the field
				iSequence = szString[0]; // Field type

				switch(iSequence)
				{
					case '1': // Get the string
						getline(ifDIFile, line);
						szString = (char *) line.c_str();
						::strncpy(szBuffer, szString, sizeof(szBuffer));
						if (!szString)
							throw(BAD_FORMAT); // Invalid format

						// If we already have all labels, break
						if (bGotLabels)
							break;

						// Ignore the quotes
						szString = strchr(szBuffer + 1, '"');
						if (szString)
							*szString = '\0';

						// Save the string in the CString object
						szLabel[uIndex] = szBuffer + 1;
						bGotaLabel = true;
						break;
					case '0': // Number
						if (!GetNumericValue(szString, &dData[uDataSets][uIndex]))
						{ // Bad number
							throw(INVALID_NUMERIC);
						};
						getline(ifDIFile, line);
						szString = (char *) line.c_str();
						if (!szString)
							throw(BAD_FORMAT); // Where's my 'V'
						if (szString[0] == 'V')
						{
							uFieldType[uIndex] = DATAFIELD; // This field is valid
							bGotData = true;
						}
						else
						{ // If not a vector, it's a blank field, signified by ""
							if (szString[0] != '"' || szString[1] != '"')
								throw(BAD_FORMAT);
						};
						break;
					default: // Unknown type
						throw(UNKNOWN_HEADER);
				}; // switch
			}; // for (uIndex
			// We got at least one label, and we finished a set.
			// We decree that all must be on the same line, so we have all labels.
			if (bGotaLabel)
				bGotLabels = true;
			// If we got data, increment uDataSets 
			if (bGotData)
				uDataSets++;
		}; // for uRecord
	} // End of try

	catch(int iError)
	{
		return(iError);
	};

	return(0);
};


/////////////////////////////////////////////////////////////////////////////
// Obtain the numeric value from a record

bool CDIFObject::GetNumericValue(char * szRecord, double *dValue)
{
	char * lpWork;

	// Now get the number of sets
	szRecord = strtok(szRecord + 2, "\n");
	if (!szRecord)
		return(false);

	// Skip dollar sign, if present
	lpWork = szRecord;
	while((*lpWork == ' ' || *lpWork == '$') && *lpWork != '\0')
		lpWork++;

	*dValue = atof(lpWork);

	return(true);
};


/////////////////////////////////////////////////////////////////////////////
// Get a field within a record

bool CDIFObject::GetRecordValue(unsigned int uSet, unsigned int uField, double *dValue)
{
	if (uSet >= uDataSets || uField >= uFields)
	{
		*dValue = 0;
		return(false);
	};
	*dValue = dData[uSet][uField];
	return(true);
};

/////////////////////////////////////////////////////////////////////////////
// Object destructor

CDIFObject::~CDIFObject()
{
	unsigned int uRecord;

	if (dData)
	{
		for (uRecord = 0; uRecord < uSets; uRecord++)
		{
			delete [] dData[uRecord];
		};
	};
	delete [] dData;
	if (szLabel)
		delete [] szLabel;
	if (uFieldType)
		delete [] uFieldType;
};
