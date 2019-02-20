/*
*/
#include <stdio.h>
#include <stdlib.h>

#include "interface.h"
#include "file_process.h"

static const char   *g_devicePlaybackName = "plughw:1,0";
static const char   *g_deviceCaptureName = "plughw:1,0";

static const int    NCHANNELS_PLAY = 1;
static const int    NCHANNELS_CAPTRUE = 8;
static const long   NBITS_PER_SAMPLE = 16;
static const long   SAMPLE_RATE = 16000;
static const unsigned long   FRAME_PER_SAMPLE_PLAY = 32;
static const unsigned long   FRAME_PER_SAMPLE_CAPTURE = 800;

snd_pcm_t    *g_handlePlayback = NULL;
snd_pcm_t    *g_handleCapture = NULL;

int PlayerConfiguration(snd_pcm_t **handle, const char* device, PCMParameter *pcmParam);
int CapturerConfiguration(snd_pcm_t **handle, const char* device, PCMParameter *pcmParam);

int main(int argc, char *argv[])
{
    int err, i, dir = 0;
    char *fileName = NULL;
    int fileNameLen = 0;
    char fileType[4] = {0};
    char *dataBufferToPlay = NULL;
    int  dataLen = 0;
    int  frameLen = 0;
    PCMParameter pcmParamPlay, pcmParamCapture;
    snd_pcm_hw_params_t *hw_params = NULL;
    char dataBufferCapture[8 * 2 * 1024] = {0};


    pcmParamPlay.nChannels = NCHANNELS_PLAY;
    pcmParamPlay.nBits = NBITS_PER_SAMPLE;
    pcmParamPlay.sampleRate = SAMPLE_RATE;
    pcmParamPlay.framePerSample = FRAME_PER_SAMPLE_PLAY;

    pcmParamCapture.nChannels = NCHANNELS_CAPTRUE;
    pcmParamCapture.nBits = NBITS_PER_SAMPLE;
    pcmParamCapture.sampleRate = SAMPLE_RATE;
    pcmParamCapture.framePerSample = FRAME_PER_SAMPLE_CAPTURE;

    if (argc != 2) {
        printf("Usage: ./Demo filename.wav or filename.pcm\n");
        return 0;
    }

    fileName = argv[1];
    fileNameLen = strlen(fileName);
    if (fileNameLen < 4) {
        printf("File name format and length error! Try again.\n");
        return 0;
    }
    memcpy(fileType, (char *)(fileName + fileNameLen - 3), 3);
    if (0 == strcmp(fileType, "pcm")) {
        dataBufferToPlay = (char *)read_pcm(fileName, &dataLen);
        frameLen = dataLen / (pcmParamPlay.nBits / 8);
        printf("PCM file data process. Data len: %d\n", dataLen);
    }
    else if (0 == strcmp(fileType, "wav")) {
        dataBufferToPlay = (char *)read_wav(fileName, &dataLen);
        frameLen = dataLen / (pcmParamPlay.nBits / 8);
        printf("PCM file data process. Data len: %d\n", dataLen);
    }
    else{
        printf("File name format error! Try again.\n");
        return 0;
    }

    // Configurate the devices
    PlayerConfiguration(&g_handlePlayback, g_devicePlaybackName, &pcmParamPlay);
    CapturerConfiguration(&g_handleCapture, g_deviceCaptureName, &pcmParamCapture);

    // Playback
    printf("Playback Start.\n"); fflush(stdout);
    snd_pcm_prepare(g_handlePlayback);//prepare the stream to palyback.
    err = snd_pcm_writei(g_handlePlayback, dataBufferToPlay, frameLen);
    if (err == -EPIPE) {
        printf("Playback underrun occurred.\n");
        snd_pcm_prepare(g_handlePlayback);
    }
    else if (err < 0) {
        printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
        snd_pcm_recover(g_handlePlayback, err, 0);//Recover the stream state from an error or suspend, such as -EINT, -EPIPE and -ESTRPIPR, for next I/O.
    }
    else if (err < frameLen) {
        printf("Short write (expected %d, wrote %d)\n", frameLen, err);
    }

    printf("Playback Over.\n"); fflush(stdout);

    // Capture
    printf("Record Start.\n"); fflush(stdout);
    for (i = 0; i < 100; i++) {
        err = snd_pcm_readi(g_handleCapture, dataBufferCapture, pcmParamCapture.framePerSample);//Get sound recording data
        if (err == -EPIPE) {
            fprintf(stderr, "Capture error: overrun occurred.\n");
            snd_pcm_prepare(g_handleCapture);
        } else if (err < 0) {
            fprintf(stderr, "Capture error: %s.\n", snd_strerror(err));
        } else {
            fprintf(stderr, "Capture successed: %dframs.\n", err);

            FILE *fid;
            fid = fopen("./test.pcm", "ab");
            if (NULL!= fid) {
                fwrite(dataBufferCapture, pcmParamCapture.framePerSample * pcmParamCapture.nChannels * pcmParamCapture.nBits / 8, 1, fid);
            }
            fclose(fid);
        }
    }
    printf("Record Over.\n"); fflush(stdout);

    snd_pcm_drop(g_handleCapture);

    // Playback
    printf("Playback Start.\n"); fflush(stdout);
    snd_pcm_prepare(g_handlePlayback);//prepare the stream to palyback.
    err = snd_pcm_writei(g_handlePlayback, dataBufferToPlay, frameLen);
    if (err == -EPIPE) {
        printf("Playback underrun occurred.\n");
        snd_pcm_prepare(g_handlePlayback);
    }
    else if (err < 0) {
        printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
        snd_pcm_recover(g_handlePlayback, err, 0);//Recover the stream state from an error or suspend, such as -EINT, -EPIPE and -ESTRPIPR, for next I/O.
    }
    else if (err < frameLen) {
        printf("Short write (expected %d, wrote %d)\n", frameLen, err);
    }

    printf("Playback Over.\n"); fflush(stdout);

    if (NULL != g_handlePlayback) {
        snd_pcm_close(g_handlePlayback);
    }

    if (NULL != g_handleCapture) {
        snd_pcm_close(g_handleCapture);
    }


    return 0;
}


int PlayerConfiguration(snd_pcm_t **handle, const char* device, PCMParameter *pcmParam)
{
    int err;
    snd_pcm_format_t format;
    switch (pcmParam->nBits) {
    case 8:
        format = SND_PCM_FORMAT_S8;
        break;
    case 16:
        format = SND_PCM_FORMAT_S16_LE;
        break;
    case 24:
        format = SND_PCM_FORMAT_S24_LE;
        break;
    case 32:
        format = SND_PCM_FORMAT_S32_LE;
        break;
    default:
        format = SND_PCM_FORMAT_S16_LE;
        break;
    }

    if ((err = snd_pcm_open(handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "Cannot open output audio device %s: %s\n", device, snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_set_params(*handle,
                                  format,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  pcmParam->nChannels,
                                  pcmParam->sampleRate,
                                  1,
                                  500000)) < 0) {	// 0.5sec
        printf("Playback open error: %s\n", snd_strerror(err));
        return -1;
    }

    return 0;
}


int CapturerConfiguration(snd_pcm_t **handle, const char* device, PCMParameter *pcmParam)
{
    int err, dir = 0;
    snd_pcm_hw_params_t *hw_params = NULL;
    snd_pcm_format_t    format;
    snd_pcm_uframes_t   framePerSample;
    int nChannel;
    int nBits;
    unsigned int sampleRate;

    nChannel        = pcmParam->nChannels;
    nBits           = pcmParam->nBits;
    sampleRate      = pcmParam->sampleRate;
    framePerSample  = pcmParam->framePerSample;

    switch (pcmParam->nBits) {
    case 8:
        format = SND_PCM_FORMAT_S8;
        break;
    case 16:
        format = SND_PCM_FORMAT_S16_LE;
        break;
    case 24:
        format = SND_PCM_FORMAT_S24_LE;
        break;
    case 32:
        format = SND_PCM_FORMAT_S32_LE;
        break;
    default:
        format = SND_PCM_FORMAT_S16_LE;
        break;
    }

    if ((err = snd_pcm_open(handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "Cannot open capture audio device %s: %s\n", device, snd_strerror(err));
        return -1;
    }

    //Parameters setting
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(*handle, hw_params);
    snd_pcm_hw_params_set_access(*handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(*handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(*handle, hw_params, nChannel);
    snd_pcm_hw_params_set_rate_near(*handle, hw_params, &sampleRate, &dir);
    snd_pcm_hw_params_set_period_size_near(*handle, hw_params, &framePerSample, &dir);

    //Start the PCM device using the parameter setted above. The PCM device is start untill snd_pcm_hw_params executed
    if ((err = snd_pcm_hw_params(*handle, hw_params)) < 0) {
        fprintf(stderr, "Capture open error: %s.\n", snd_strerror(err));
        return -1;
    }

    return 0;
}
