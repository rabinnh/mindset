/* 
 * File:   CRAIIAtomicLock.h
 * Author: rbross
 *
 * Created on July 9, 2013, 10:27 AM
 */

#ifndef CRAIIATOMICLOCK_H
#define	CRAIIATOMICLOCK_H

#include "CAtomicLock.h"

//! RAII atomic sychronization object
class CRAIIAtomicLock
{
public:
    //! Constructor implements lock
    CRAIIAtomicLock(CAtomicLock &cLock)
    {
        pLock = &cLock;
        pLock->Lock();
    };
    //! Destructor frees lock
    virtual ~CRAIIAtomicLock()
    {
        pLock->Unlock();
    }

protected:
    CAtomicLock *pLock;
};
#endif	/* CRAIIATOMICLOCK_H */

