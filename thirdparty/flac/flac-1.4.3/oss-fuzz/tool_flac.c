/* fuzzer_tool_flac
 * Copyright (C) 2023  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h> /* for memcpy */
#define FUZZ_TOOL_FLAC
#define fprintf(...)
#define printf(...)
#include "../src/flac/main.c"
#include "common.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	size_t size_left = size;
	size_t arglen;
	char * argv[67];
	char exename[] = "flac";
	char filename[] = "/tmp/fuzzXXXXXX";
	int numarg = 0, maxarg;
	int file_to_fuzz;
	int tmp_stdout, tmp_stdin;
	fpos_t pos_stdout;
	bool use_stdin = false;

	/* reset global vars */
	flac__utils_verbosity_ = 0;
	share__opterr = 0;
	share__optind = 0;

	if(size < 2)
		return 0;

	maxarg = data[0] & 63;
	use_stdin = data[0] & 64;
	size_left--;

	argv[0] = exename;
	numarg++;

	/* Check whether input is zero delimited */
	while((arglen = strnlen((char *)data+(size-size_left),size_left)) < size_left && numarg < maxarg) {
		argv[numarg++] = (char *)data+(size-size_left);
		size_left -= arglen + 1;
	}

	file_to_fuzz = mkstemp(filename);

	if (file_to_fuzz < 0)
		abort();
	write(file_to_fuzz,data+(size-size_left),size_left);
	close(file_to_fuzz);

	/* redirect stdout */
	fflush(stdout);
	fgetpos(stdout,&pos_stdout);
	tmp_stdout = dup(fileno(stdout));
	freopen("/dev/null","w",stdout);

	/* redirect stdin */
	tmp_stdin = dup(fileno(stdin));

	if(use_stdin)
		freopen(filename,"r",stdin);
	else {
		freopen("/dev/null","r",stdin);
		argv[numarg++] = filename;
	}

	main_to_fuzz(numarg,argv);

	/* restore stdout */
	fflush(stdout);
	dup2(tmp_stdout, fileno(stdout));
	close(tmp_stdout);
	clearerr(stdout);
	fsetpos(stdout,&pos_stdout);

	/* restore stdin */
	dup2(tmp_stdin, fileno(stdin));
	close(tmp_stdin);
	clearerr(stdin);

	unlink(filename);

	return 0;
}

