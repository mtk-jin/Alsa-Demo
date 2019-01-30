/********************************************************
 * Creatint for practice, free learning purpose.
 * 
 * filename: main.c
 * author: Martin
 * date: Wed Jan 30 2019
 * description: Read a pcm/wav file, and send to data to PCM device for playback.
 * 
 ********************************************************/

#include "alsa/asoundlib.h"
#include "file_process.h"

static char *device = "default";			/* playback device */

int main(int argc, char *argv[])
{
        int err, i;
        snd_pcm_t *handle;
        snd_pcm_sframes_t frames;

        char *fileName = NULL;
        int fileNameLen = 0;
        char fileType[4] = {0};
        char *dataBuffer = NULL;
        int  dataLen = 0;
        int delayCount = 0;

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
                printf("PCM file data process.\n");
                dataBuffer = (char *)read_pcm(fileName, &dataLen);
        }
        else if (0 == strcmp(fileType, "wav")) {
                printf("WAV file data process.\n");
                dataBuffer = (char *)read_wav(fileName, &dataLen);
        }
        else{
                printf("File name format error! Try again.\n");
                return 0;
        }
        
	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_pcm_set_params(handle,
	                              SND_PCM_FORMAT_S16_LE,
	                              SND_PCM_ACCESS_RW_INTERLEAVED,
	                              1,
	                              16000,
	                              1,
	                              500000)) < 0) {	/* 0.5sec */
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

        for (i = 0; i < 3; i++) {
                snd_pcm_prepare(handle);//prepare the stream to palyback.
                printf("Playback times: %d\n", i + 1); fflush(stdout);
                frames = snd_pcm_writei(handle, dataBuffer, dataLen / 2);
                if (frames == -EPIPE) {
                        printf("Playback underrun occurred.\n");
                        snd_pcm_prepare(handle);
                }
                else if (frames < 0) {
                        printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
                        frames = snd_pcm_recover(handle, frames, 0);//Recover the stream state from an error or suspend, such as -EINT, -EPIPE and -ESTRPIPR, for next I/O.
                }
                else if (frames < (long)dataLen / 2) {
                        printf("Short write (expected %li, wrote %li)\n", (long)dataLen / 2, frames);
                }
                
                printf("Playback over. Delay 3 seconds.\n"); fflush(stdout);
                delayCount = 3;
                while (delayCount >0) {
                        printf("Delay: %d.\n", 4 - delayCount); fflush(stdout);
                        sleep(1);
                        delayCount--;
                }
        }

        snd_pcm_drain(handle);//Stop a PCM preserving pending frames.
	snd_pcm_close(handle);
        if (NULL != dataBuffer) {
                free(dataBuffer);
        }
	return 0;
}
