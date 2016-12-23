/*
 * Copyright 2016 Michael Brumlow <mbrumlow@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#define SUCCESS (0)
#define FAILURE (1)
#define INVALID_PARAM (2)

#define DEFAULT_SAMPLE_RATE_HZ (10000000)
#define DEFAULT_SYMBOL_PERID_US (127)

uint32_t sample_rate_hz = DEFAULT_SAMPLE_RATE_HZ;
uint32_t symbol_period_us = DEFAULT_SYMBOL_PERID_US;

int binaryIn(FILE* in, int repeate){

    do {

        int ret;
        uint8_t byte = 0;

        while( ret = fread(&byte, sizeof(byte), 1, in ) ) {
            for(int bit = 0; bit < 8; bit++ ) {

                int8_t i = 0;
                int8_t q = 0;

                if( byte & 1 << bit ) {
                    i = 127;
                }

                for(uint32_t x = 0; x < (symbol_period_us * sample_rate_hz) / 1e6; x++) {
                    write(1, &i, sizeof(int8_t));
                    write(1, &q, sizeof(int8_t));
                }
            }
        }

        rewind(in);

    } while(repeate);

    return 0;
}

int textIn(FILE* in, int repeate) {

    do {

        int ret;
        char ch = 0;

        while( ret = fread(&ch, sizeof(ch), 1, in ) ) {

            int8_t i = 0;
            int8_t q = 0;

            if( ch == '1' ) {
                i = 127;
            } else if ( ch != '0' ){
                continue;
            }

            for(uint32_t x = 0; x < (symbol_period_us * sample_rate_hz) / 1e6; x++) {
                write(1, &i, sizeof(int8_t));
                write(1, &q, sizeof(int8_t));
            }
        }

        rewind(in);

    } while(repeate);

    return 0;
}

static void usage() {
    printf("Usage:\n");
    printf("\t-s sample_rate_hz # Sample rate in Hz.\n");
    printf("\t-p symbol_period_us # Symbol period time in us.\n");
    printf("\t-i <filename> # Input file (use '-' for stdin).\n");
    printf("\t-r # Repeate.\n");
    printf("\t-r # Decode ascii 1's and 0's.\n");
}

int main(int argc, char ** argv) {

    int opt;
    int text = 0;
    int repeate = 0;
    int result = SUCCESS;
    char *endptr;
    double f_hz;

    const char* path_in = NULL;
    FILE *in = NULL;

    while( (opt = getopt(argc, argv, "s:p:o:i:rt")) != EOF)  {

        result = SUCCESS;

        switch(opt) {
            case 's':
                f_hz = strtod(optarg, &endptr);
                if(optarg == endptr) {
                    result = INVALID_PARAM;
                    break;
                }
                sample_rate_hz = f_hz;
                break;
            case 'p':
                symbol_period_us = atoi(optarg);
                break;
            case 'i':
                path_in = optarg;
                break;
            case 'r':
                repeate = 1;
                break;
            case 't':
                text = 1;
                break;
            default:
                fprintf(stderr, "unknown argument '-%c %s'\n", opt, optarg);
                usage();
                return EXIT_FAILURE;
        }

        if( result != SUCCESS ) {
            fprintf(stderr, "argument error: '-%c %s' (%d)\n", opt, optarg, result);
            usage();
            return EXIT_FAILURE;
        }
    }

    if( !path_in ) {
        fprintf(stderr, "must specify an input file with -i .\n");
        usage();
        return EXIT_FAILURE;
    }

    if(strcmp(path_in, "-") == 0) {
        in = stdin;
    } else {
        in = fopen(path_in, "rb");
    }

    if(text) {
        return textIn(in, repeate);
    }

    return binaryIn(in, repeate);
}


