# ALSA-Demo
ALSA is the Advanced Linux Sound Architecture, providing audio and MIDI functionality to the Linux operating system. Demos in this repository is showed the basic usage of ALSA.
## Usage
cd demo forld, use cmake to produce Makefile, and then use make commond to compile and build the target.  

## Content
### playfile
Read a pcm/wav file, and send to data to PCM device for playback. Every playback repeats three times, and sleep 3 seconds between each playing.  
Using the cmd below to run the demo.  
```
./playpcm test.wav
./playpcm test.pcm
```

### duplexDemo
There are two short demo, which show a full duplex usage of ALSA, meaning that the demo shows the action that audio capturing and speech playing meanwhile.  

