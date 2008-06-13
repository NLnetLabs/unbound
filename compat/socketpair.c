/* socketpair.c - windows mingw32 replacement for socketpair.
 * creates a pipe that works like a socketpair a bit
 * Taken from libevent-1.4.3.stable WIN32-Code/misc.c
 * License: BSD.
 */
#include "config.h"
#include <stdio.h>
#include <windows.h>

int
socketpair(int d, int type, int protocol, int *sv)
{
	static int count;
	char buf[64];
	HANDLE fd;
	DWORD dwMode;
	(void)d; (void)type; (void)protocol;
	sprintf(buf, "\\\\.\\pipe\\levent-%d", count++);
	/* Create a duplex pipe which will behave like a socket pair */
	fd = CreateNamedPipe(buf, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_NOWAIT, 
		PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
	if (fd == INVALID_HANDLE_VALUE)
		return (-1);
	sv[0] = (int)fd;

	fd = CreateFile(buf, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd == INVALID_HANDLE_VALUE)
		return (-1);
	dwMode = PIPE_NOWAIT;
	SetNamedPipeHandleState(fd, &dwMode, NULL, NULL);
	sv[1] = (int)fd;

	return (0);
}
