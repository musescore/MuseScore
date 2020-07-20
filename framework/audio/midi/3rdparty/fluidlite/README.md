FLUIDLITE 1.0 (c) 2016 Robin Lobel
=========

FluidLite is a very light version of FluidSynth
designed to be hardware, platform and external dependency independant.
It only uses standard C libraries.

It also adds support for SF3 files (SF2 files compressed with ogg vorbis)
and an additional setting to remove the constraint of channel 9 (drums):
fluid_settings_setstr(settings, "synth.drums-channel.active", "no");
you can still select bank 128 on any channel to use drum kits.

FluidLite keeps very minimal functionnalities (settings and synth),
therefore MIDI file reading, realtime MIDI events and audio output must be
implemented externally.

Usage:
=====

include "fluidlite.h"

fluid_settings_t* settings=new_fluid_settings();  
fluid_synth_t* synth=new_fluid_synth(settings);  
fluid_synth_sfload(synth, "soundfont.sf3",1);

float* buffer=new float[44100*2];

FILE* file=fopen("float32output.pcm","wb");

fluid_synth_noteon(synth,0,60,127);  
fluid_synth_write_float(synth, 44100,buffer, 0, 2, buffer, 1, 2);  
fwrite(buffer,sizeof(float),44100*2,file);

fluid_synth_noteoff(synth,0,60);  
fluid_synth_write_float(synth, 44100,buffer, 0, 2, buffer, 1, 2);  
fwrite(buffer,sizeof(float),44100*2,file);

fclose(file);

delete_fluid_synth(synth);  
delete_fluid_settings(settings);
