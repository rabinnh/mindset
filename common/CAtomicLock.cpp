/* 
 * File:   CAtomicLock.cpp
 * Author: rbross
 * 
 * Created on June 14, 2013, 11:29 AM
 */

#include "CAtomicLock.h"

CAtomicLock::CAtomicLock()
{
	bLock = ATOMIC_VAR_INIT(false);
}

CAtomicLock::~CAtomicLock()
{
}


// Wait for the lock
void CAtomicLock::Lock()
{	// usleep is not strictly necessary, but if this isn't here, a long running lock waiting will eat 100% of a core
    while(atomic_exchange_explicit(&bLock, true, memory_order_acquire))
        ::usleep(1); 
}

	
// Release the lock
void CAtomicLock::Unlock()
{
	atomic_store_explicit(&bLock, false, memory_order_release);
}
