/* getentropy_win.c - get entropy on Windows.
*
 * Copyright (c) 2014, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"

int
getentropy(void *buf, size_t len)
{
	HMODULE lib;

	if(len > 256) {
		errno = EIO;
		return -1;
	}

	/* Get entropy with windows secure random number generator,
	 * for windows XP and later, it is in the ADVAPI32 dll */
	lib = LoadLibrary("ADVAPI32.DLL");
	if(lib) {
		/* Load the RtlGenRandom function */
		BOOLEAN (APIENTRY* genrandom)(void*,ULONG) =
			(BOOLEAN (APIENTRY*)(void*,ULONG))
			GetProcAddress(lib, "SystemFunction036");
		if(genrandom) {
			if(genrandom(buf, len)) {
				FreeLibrary(lib);
				return 0; /* success */
			}
		}
		FreeLibrary(lib);
	}

	errno = EIO;
	return -1;
}
