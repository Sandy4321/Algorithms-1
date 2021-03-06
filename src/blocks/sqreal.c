#ifndef __x86_64__
#   define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sq_dsp.h"
#include "sq_utils.h"

//          1         2         3         4         5         6         7
// 123456789012345678901234567890123456789012345678901234567890123456789012
char *usage_text[] =
{
    "                                                                        ",
    "NAME                                                                    ",
    "  sqreal - inputs time-series of complex (real,imag) float coefficient  ",
    "           pairs.  Outputs only the real coefficients and discards the  ",
    "           imaginary coefficients: (r1,i1)(r2,i2)... --> r1, r2, ...    ",
    "SYNOPSIS                                                                ",
    "  sqreal [OPTIONS] ...                                                  ",
    "DESCRIPTION                                                             ",
    "  -l  pos. integer (required),  length of sample buffer (number of      ",
    "      real,imag coefficient pairs input with a block read at a time and ",
    "      number of real coefficients output with a block write at a time)  ",
    "                                                                        "
};

unsigned int sblen = 0;

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "l:")) != -1)
    {
        switch (opt)
        {
            case 'l':
                sscanf(optarg, "%u", &sblen);
                break;
        }
    }

    int status = sq_real(stdin, stdout, sblen);
    
    if(status < 0)
    {
        fprintf(stderr, "%s encountered a fatal error.", argv[0]);
        sq_error_handle(status);
        print_usage(usage_text);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}