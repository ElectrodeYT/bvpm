//
// Created by alexander on 5/19/22.
//

#ifndef BVPM_HUMAN_READABLE_H
#define BVPM_HUMAN_READABLE_H

#include <stdio.h>
#include <stdlib.h> // atoll
#include <stdint.h> // uint64_t
#include <inttypes.h> // PRIu64

// Stolen from https://gist.github.com/dgoguerra/7194777

static const char* humanSize(uint64_t bytes)
{
    const char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;
    double dblBytes = bytes;

    if (bytes > 1024) {
        for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
            dblBytes = bytes / 1024.0;
    }

    static char output[200];
    sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
    return output;
}

#endif //BVPM_HUMAN_READABLE_H
