
#ifndef _FLUID_CONV_TABLES_H
#define _FLUID_CONV_TABLES_H

/*
 Attenuation range in centibels.
 Attenuation range is the dynamic range of the volume envelope generator
 from 0 to the end of attack segment.
 fluidsynth is a 24 bit synth, it could (should??) be 144 dB of attenuation.
 However the spec makes no distinction between 16 or 24 bit synths, so use
 96 dB here.

 Note about usefulness of 24 bits:
 1)Even fluidsynth is a 24 bit synth, this format is only relevant if
 the sample format coming from the soundfont is 24 bits and the audio sample format
 chosen by the application (audio.sample.format) is not 16 bits.

 2)When the sample soundfont is 16 bits, the internal 24 bits number have
 16 bits msb and lsb to 0. Consequently, at the DAC output, the dynamic range of
 this 24 bit sample is reduced to the the dynamic of a 16 bits sample (ie 90 db)
 even if this sample is produced by the audio driver using an audio sample format
 compatible for a 24 bit DAC.

 3)When the audio sample format settings is 16 bits (audio.sample.format), the
 audio driver will make use of a 16 bit DAC, and the dynamic will be reduced to 96 dB
 even if the initial sample comes from a 24 bits soundfont.

 In both cases (2) or (3), the real dynamic range is only 96 dB.

 Other consideration for FLUID_NOISE_FLOOR related to case (1),(2,3):
 - for case (1), FLUID_NOISE_FLOOR should be the noise floor for 24 bits (i.e -138 dB).
 - for case (2) or (3), FLUID_NOISE_FLOOR should be the noise floor for 16 bits (i.e -90 dB).
 */
#define FLUID_PEAK_ATTENUATION  960.0f

#define FLUID_CENTS_HZ_SIZE     1200
#define FLUID_VEL_CB_SIZE       128
#define FLUID_CB_AMP_SIZE       1441
#define FLUID_PAN_SIZE          1002

#endif
