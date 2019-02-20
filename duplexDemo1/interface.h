#ifndef ALSA_INTERFACE_H
#define ALSA_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <poll.h>
#include <alsa/asoundlib.h>
#include <memory.h>


typedef struct PCMParameter {
    int nChannels;
    int nBits;
    unsigned int sampleRate;
    unsigned int framePerSample;
}PCMParameter;


#endif // ALSA_INTERFACE_H
