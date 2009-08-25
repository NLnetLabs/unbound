/* taken from ldns 1.6.1 */
#include "config.h"

#ifdef HAVE_TIME_H
#include <time.h>
#endif

char *ctime_r(const time_t *timep, char *buf)
{
	/* no thread safety. */
	char* result = ctime(timep);
	if(buf && result)
		strcpy(buf, result);
	return result;
}
