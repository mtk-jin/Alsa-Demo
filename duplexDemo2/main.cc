/********************************************************
 * Creatint for practice, free learning purpose.
 * 
 * filename: main.c
 * author: Martin
 * date: Wed Feb 20 2019
 * description: ALSA duplex demo, for capturing and playing meanwhile.
 * 
 ********************************************************/

#include <cstdlib>
#include <string.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

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

PCMParameter g_pcmParamPlayback, g_pcmParamCapture;

char *g_fileName = NULL;

std::thread g_threadPlayback;
std::thread g_threadCapture;
std::mutex  g_mutexAction;
std::condition_variable g_cvStartPlayback;
std::condition_variable g_cvStartCapture;
bool g_bExitPlayback = false;
bool g_bExitCapture = false;

void threadAudioPlayback(void *userData);
void threadAudioCapture(void *userData);

int PlayerConfiguration(snd_pcm_t **handle, const char* device, PCMParameter *pcmParam);
int CapturerConfiguration(snd_pcm_t **handle, const char* device, PCMParameter *pcmParam);

extern void *read_wav (char *name, int *len);
extern void *read_pcm (char *name, int *len);

int main(int argc, char *argv[])
{
    int fileNameLen = 0;
    char fileType[4] = {0};

    g_pcmParamPlayback.nChannels = NCHANNELS_PLAY;
    g_pcmParamPlayback.nBits = NBITS_PER_SAMPLE;
    g_pcmParamPlayback.sampleRate = SAMPLE_RATE;
    g_pcmParamPlayback.framePerSample = FRAME_PER_SAMPLE_PLAY;

    g_pcmParamCapture.nChannels = NCHANNELS_CAPTRUE;
    g_pcmParamCapture.nBits = NBITS_PER_SAMPLE;
    g_pcmParamCapture.sampleRate = SAMPLE_RATE;
    g_pcmParamCapture.framePerSample = FRAME_PER_SAMPLE_CAPTURE;

    if (argc != 2) {
        printf("Usage: ./Demo filename.wav or filename.pcm\n");
        return 0;
    }

    g_fileName = argv[1];
    fileNameLen = strlen(g_fileName);
    if (fileNameLen < 4) {
        printf("File name format and length error! Try again.\n");
        return 0;
    }
    memcpy(fileType, (char *)(g_fileName + fileNameLen - 3), 3);
    if (0 == strcmp(fileType, "pcm")) {
        printf("PCM data file process.\n");
    }
    else if (0 == strcmp(fileType, "wav")) {
        printf("WAV data file process.\n");
    }
    else{
        printf("File format error! Try again.\n");
        return 0;
    }

    // Configurate the devices
    PlayerConfiguration(&g_handlePlayback, g_devicePlaybackName, &g_pcmParamPlayback);
    CapturerConfiguration(&g_handleCapture, g_deviceCaptureName, &g_pcmParamCapture);

    g_threadCapture = std::thread(threadAudioCapture, g_fileName);
    g_threadPlayback = std::thread(threadAudioPlayback, g_fileName);
    g_threadPlayback.join();
    sleep(1);

    g_threadPlayback = std::thread(threadAudioPlayback, g_fileName);
    g_threadPlayback.join();
    sleep(1);

    g_threadPlayback = std::thread(threadAudioPlayback, g_fileName);
    g_threadPlayback.join();
    sleep(1);

    g_threadPlayback = std::thread(threadAudioPlayback, g_fileName);
    g_threadPlayback.join();
    sleep(1);

    g_threadPlayback = std::thread(threadAudioPlayback, g_fileName);
    g_threadPlayback.join();
    sleep(1);

    g_bExitPlayback = true;
    g_bExitCapture = true;

    if (g_threadPlayback.joinable()) {
        g_threadPlayback.join();
    }
    if (g_threadCapture.joinable()) {
        g_threadCapture.join();
    }

    if (NULL != g_handlePlayback) {
        snd_pcm_close(g_handlePlayback);
    }

    if (NULL != g_handleCapture) {
        snd_pcm_close(g_handleCapture);
    }


    return 0;
}


void threadAudioPlayback(void *userData)
{
    char    *fileName = (char *)userData;
    int     fileNameLen = 0;
    char    fileType[4] = {0};
    char    *dataBufferToPlay = NULL;
    int     dataLen = 0;
    int     frameLen = 0;
    int     err;
    
    printf("Playback thread: file name: %s\n", fileName);

    fileNameLen = strlen(fileName);
    memcpy(fileType, (char *)(fileName + fileNameLen - 3), 3);
    if (0 == strcmp(fileType, "pcm")) {
        dataBufferToPlay = (char *)read_pcm(fileName, &dataLen);
        frameLen = dataLen / (g_pcmParamPlayback.nBits / 8);
    }
    else if (0 == strcmp(fileType, "wav")) {
        dataBufferToPlay = (char *)read_wav(fileName, &dataLen);
        frameLen = dataLen / (g_pcmParamPlayback.nBits / 8);
    }
    else{
        printf("Playback thread exit: File format error.\n");
        return;
    }

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

    // Release data buffer.
    if (NULL != dataBufferToPlay) {
        free(dataBufferToPlay);
    }
}

void threadAudioCapture(void *userData)
{
    printf("Playback thread start.\n"); fflush(stdout);

    int err;
    char dataBufferCapture[8 * 2 * 1024] = {0};

    // Capture
    printf("Record Start.\n"); fflush(stdout);
    while (!g_bExitCapture) {
        snd_pcm_prepare(g_handleCapture);
        err = snd_pcm_readi(g_handleCapture, dataBufferCapture, g_pcmParamCapture.framePerSample);//Get sound recording data
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
                fwrite(dataBufferCapture, g_pcmParamCapture.framePerSample * g_pcmParamCapture.nChannels * g_pcmParamCapture.nBits / 8, 1, fid);
            }
            fclose(fid);
        }
        // snd_pcm_drain(g_handleCapture);
        // snd_pcm_drop(g_handleCapture);
    }
    printf("Record Over.\n"); fflush(stdout);
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
    snd_pcm_hw_params_set_format(*handle, hw_params, format);
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
