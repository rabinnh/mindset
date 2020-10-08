#if !defined(cqstats_log_H__INCLUDED_)
#define cqstats_log_H__INCLUDED_

/*!
* \ingroup Logging
*/

//! Syslog macros
#include <syslog.h>

#define LOG(severity, msg, ...) syslog(severity, msg, ##__VA_ARGS__)
#define INIT_LOG(name, flags, type) 


#endif
