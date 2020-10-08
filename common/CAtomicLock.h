/* 
 * File:   CAtomicLock.h
 * Author: rbross
 *
 * Created on June 14, 2013, 11:29 AM
 */

#ifndef CATOMICLOCK_H
#define	CATOMICLOCK_H

#include <atomic>
#include <unistd.h>

using namespace std;

//! Atomic lock class
//! Because atomic locks in effect are "spin locks", they should only be
//! used to protect data structures and only for extremely fast operations.
class CAtomicLock
{
public:
	CAtomicLock();
	virtual ~CAtomicLock();
	
	//! Wait for the lock
	void Lock();
	
	//! Release the lock
	void Unlock();
	
private:
    //! The lock variable
	atomic<bool> bLock;	

};

#endif	/* CATOMICLOCK_H */

