// DIFObj.h : DIF interpretor object class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef DIFOBJ__H
#define DIFOBJ__H

#include <string>

using namespace std;

class CDIFObject
{
    // Variables
public:

    CDIFObject(const char * lpszFileName);

    ~CDIFObject();

    // Field types            

    enum field_types
    {
        INVALID = 0,
        DATAFIELD,
        RESULTFIELD,
        INPUTFIELD
    };

    enum eHeader
    {
        TABLE,
        VECTORS,
        TUPLES,
        DATA,
        LABEL,
        BOT,
        EOD
    };

    enum eErrors
    {
        INVALID_FILE = 1,
        INVALID_LABEL,
        TABLE_OUTOFSEQ,
        VECTOR_OUTOFSEQ,
        BAD_VECTOR_VALUE,
        NO_VECTORS,
        TUPLES_OUTOFSEQ,
        NO_TUPLES,
        BAD_TUPLE_VALUE,
        BAD_FORMAT,
        NOT_ENOUGH_MEM,
        NOT_ENOUGH_TUPLES,
        INVALID_NUMERIC,
        UNKNOWN_HEADER
    };


    char szTitle[80]; // Table title	
    const char *szFile;
    char szBuffer[128]; // Line buffer
    unsigned int uSets; // Sets of data in the file
    unsigned int uDataSets; // Doesn't include the string label set
    unsigned int uFields; // Number of fields in the file
    string *szLabel; // Column labels
    double **dData; // The numeric data
    unsigned int *uFieldType; // Valid fields

    // Parse the file			
    bool ParseDIFFile();

    // Get the numeric value
    bool GetNumericValue(char *szRecord, double *dValue);

    // Get individual field values
    bool GetRecordValue(unsigned int uSet, unsigned int uField, double *dValue);
};

#endif
