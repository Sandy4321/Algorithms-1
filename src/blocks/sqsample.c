#ifndef __x86_64__
#define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "sq_constants.h"
#include "sq_dsp.h"
#include "sq_utils.h"

int main(int argc, char **argv)
{
    int status = sq_sample(stdin, stdout, SMPLS_PER_READ);
    
    if(status < 0)
    {
        fprintf(stderr, "%s encountered a fatal error.", argv[0]);
        sq_error_handle(status);
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
