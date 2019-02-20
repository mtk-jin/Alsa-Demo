/********************************************************
 * Creatint for practice, free learning purpose.
 * 
 * filename: file_process.c
 * author: Martin
 * date: Wed Jan 30 2019
 * description: 
 * 
 ********************************************************/

#include "file_process.h"

void *read_wav (char *name, int *len)
{
	FILE *wavfp = fopen (name, "rb");
	if (!wavfp)
	{
		printf ("Can't open %s to read!", name);
		return NULL;
	}
	fseek (wavfp, 0, SEEK_END);
	int data_len = (ftell (wavfp) - 44) / sizeof (char);
	char *data = (char *) malloc (sizeof (char) * data_len);
	fseek (wavfp, 44, SEEK_SET);
	fread (data, sizeof (char), data_len, wavfp);
	fclose (wavfp);
	*len = data_len;
	return data;
}

void *read_pcm (char *name, int *len)
{
	FILE *pcmfp = fopen (name, "rb");
	if (!pcmfp)
	{
		printf ("Can't open %s to read!", name);
		return NULL;
	}
	fseek (pcmfp, 0, SEEK_END);
	int data_len = ftell (pcmfp) / sizeof (char);
	char *data = (char *) malloc (sizeof (char) * data_len);
	fseek (pcmfp, 0, SEEK_SET);
	fread (data, sizeof (char), data_len, pcmfp);
	fclose (pcmfp);
	*len = data_len;
	return data;
}
