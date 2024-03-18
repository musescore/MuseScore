/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * SoundFont loading code borrowed from Smurf SoundFont Editor by Josh Green
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


#ifndef _FLUID_DEFSFONT_H
#define _FLUID_DEFSFONT_H


#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_sffile.h"
#include "fluid_list.h"
#include "fluid_mod.h"
#include "fluid_gen.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------sfont.h----------------------------*/

#define SF_SAMPMODES_LOOP	1
#define SF_SAMPMODES_UNROLL	2

#define SF_MIN_SAMPLERATE	400
#define SF_MAX_SAMPLERATE	50000

#define SF_MIN_SAMPLE_LENGTH	32

/***************************************************************
 *
 *       FORWARD DECLARATIONS
 */
typedef struct _fluid_defsfont_t fluid_defsfont_t;
typedef struct _fluid_defpreset_t fluid_defpreset_t;
typedef struct _fluid_preset_zone_t fluid_preset_zone_t;
typedef struct _fluid_inst_t fluid_inst_t;
typedef struct _fluid_inst_zone_t fluid_inst_zone_t;            /**< Soundfont Instrument Zone */
typedef struct _fluid_voice_zone_t fluid_voice_zone_t;

/* defines the velocity and key range for a zone */
struct _fluid_zone_range_t
{
    int keylo;
    int keyhi;
    int vello;
    int velhi;
    unsigned char ignore;	/* set to TRUE for legato playing to ignore this range zone */
};

/* Stored on a preset zone to keep track of the inst zones that could start a voice
 * and their combined preset zone/instument zone ranges */
struct _fluid_voice_zone_t
{
    fluid_inst_zone_t *inst_zone;
    fluid_zone_range_t range;
};

/*

  Public interface

 */

fluid_sfont_t *fluid_defsfloader_load(fluid_sfloader_t *loader, const char *filename);


int fluid_defsfont_sfont_delete(fluid_sfont_t *sfont);
const char *fluid_defsfont_sfont_get_name(fluid_sfont_t *sfont);
fluid_preset_t *fluid_defsfont_sfont_get_preset(fluid_sfont_t *sfont, int bank, int prenum);
void fluid_defsfont_sfont_iteration_start(fluid_sfont_t *sfont);
fluid_preset_t *fluid_defsfont_sfont_iteration_next(fluid_sfont_t *sfont);


void fluid_defpreset_preset_delete(fluid_preset_t *preset);
const char *fluid_defpreset_preset_get_name(fluid_preset_t *preset);
int fluid_defpreset_preset_get_banknum(fluid_preset_t *preset);
int fluid_defpreset_preset_get_num(fluid_preset_t *preset);
int fluid_defpreset_preset_noteon(fluid_preset_t *preset, fluid_synth_t *synth, int chan, int key, int vel);

int fluid_zone_inside_range(fluid_zone_range_t *zone_range, int key, int vel);

/*
 * fluid_defsfont_t
 */
struct _fluid_defsfont_t
{
    const fluid_file_callbacks_t *fcbs; /* the file callbacks used to load this Soundfont */
    char *filename;           /* the filename of this soundfont */
    unsigned int samplepos;   /* the position in the file at which the sample data starts */
    unsigned int samplesize;  /* the size of the sample data in bytes */
    short *sampledata;        /* the sample data, loaded in ram */

    unsigned int sample24pos;		/* position within sffd of the sm24 chunk, set to zero if no 24 bit sample support */
    unsigned int sample24size;		/* length within sffd of the sm24 chunk */
    char *sample24data;        /* if not NULL, the least significant byte of the 24bit sample data, loaded in ram */

    fluid_sfont_t *sfont;      /* pointer to parent sfont */
    fluid_list_t *sample;      /* the samples in this soundfont */
    fluid_list_t *preset;      /* the presets of this soundfont */
    fluid_list_t *inst;        /* the instruments of this soundfont */
    int mlock;                 /* Should we try memlock (avoid swapping)? */
    int dynamic_samples;       /* Enables dynamic sample loading if set */

    fluid_list_t *preset_iter_cur;       /* the current preset in the iteration */
};


fluid_defsfont_t *new_fluid_defsfont(fluid_settings_t *settings);
int delete_fluid_defsfont(fluid_defsfont_t *defsfont);
int fluid_defsfont_load(fluid_defsfont_t *defsfont, const fluid_file_callbacks_t *file_callbacks, const char *file);
const char *fluid_defsfont_get_name(fluid_defsfont_t *defsfont);
fluid_preset_t *fluid_defsfont_get_preset(fluid_defsfont_t *defsfont, int bank, int prenum);
void fluid_defsfont_iteration_start(fluid_defsfont_t *defsfont);
fluid_preset_t *fluid_defsfont_iteration_next(fluid_defsfont_t *defsfont);
int fluid_defsfont_load_sampledata(fluid_defsfont_t *defsfont, SFData *sfdata, fluid_sample_t *sample);
int fluid_defsfont_load_all_sampledata(fluid_defsfont_t *defsfont, SFData *sfdata);

int fluid_defsfont_add_sample(fluid_defsfont_t *defsfont, fluid_sample_t *sample);
int fluid_defsfont_add_preset(fluid_defsfont_t *defsfont, fluid_defpreset_t *defpreset);


/*
 * fluid_preset_t
 */
struct _fluid_defpreset_t
{
    fluid_defpreset_t *next;
    char name[21];                        /* the name of the preset */
    unsigned int bank;                    /* the bank number */
    unsigned int num;                     /* the preset number */
    fluid_preset_zone_t *global_zone;        /* the global zone of the preset */
    fluid_preset_zone_t *zone;               /* the chained list of preset zones */
    int pinned;                           /* preset samples pinned to sample cache? */
};

fluid_defpreset_t *new_fluid_defpreset(void);
void delete_fluid_defpreset(fluid_defpreset_t *defpreset);
fluid_defpreset_t *fluid_defpreset_next(fluid_defpreset_t *defpreset);
int fluid_defpreset_import_sfont(fluid_defpreset_t *defpreset, SFPreset *sfpreset, fluid_defsfont_t *defsfont, SFData *sfdata);
int fluid_defpreset_set_global_zone(fluid_defpreset_t *defpreset, fluid_preset_zone_t *zone);
int fluid_defpreset_add_zone(fluid_defpreset_t *defpreset, fluid_preset_zone_t *zone);
fluid_preset_zone_t *fluid_defpreset_get_zone(fluid_defpreset_t *defpreset);
fluid_preset_zone_t *fluid_defpreset_get_global_zone(fluid_defpreset_t *defpreset);
int fluid_defpreset_get_banknum(fluid_defpreset_t *defpreset);
int fluid_defpreset_get_num(fluid_defpreset_t *defpreset);
const char *fluid_defpreset_get_name(fluid_defpreset_t *defpreset);
int fluid_defpreset_noteon(fluid_defpreset_t *defpreset, fluid_synth_t *synth, int chan, int key, int vel);

/*
 * fluid_preset_zone
 */
struct _fluid_preset_zone_t
{
    fluid_preset_zone_t *next;
    char *name;
    fluid_inst_t *inst;
    fluid_list_t *voice_zone;
    fluid_zone_range_t range;
    fluid_gen_t gen[GEN_LAST];
    fluid_mod_t *mod;  /* List of modulators */
};

fluid_preset_zone_t *new_fluid_preset_zone(char *name);
void delete_fluid_list_mod(fluid_mod_t *mod);
void delete_fluid_preset_zone(fluid_preset_zone_t *zone);
fluid_preset_zone_t *fluid_preset_zone_next(fluid_preset_zone_t *zone);
int fluid_preset_zone_import_sfont(fluid_preset_zone_t *zone, fluid_preset_zone_t *global_zone, SFZone *sfzone, fluid_defsfont_t *defssfont, SFData *sfdata);
fluid_inst_t *fluid_preset_zone_get_inst(fluid_preset_zone_t *zone);

/*
 * fluid_inst_t
 */
struct _fluid_inst_t
{
    char name[21];
    int source_idx; /* Index of instrument in source Soundfont */
    fluid_inst_zone_t *global_zone;
    fluid_inst_zone_t *zone;
};

fluid_inst_t *new_fluid_inst(void);
fluid_inst_t *fluid_inst_import_sfont(int inst_idx, fluid_defsfont_t *defsfont, SFData *sfdata);
void delete_fluid_inst(fluid_inst_t *inst);
int fluid_inst_set_global_zone(fluid_inst_t *inst, fluid_inst_zone_t *zone);
int fluid_inst_add_zone(fluid_inst_t *inst, fluid_inst_zone_t *zone);
fluid_inst_zone_t *fluid_inst_get_zone(fluid_inst_t *inst);
fluid_inst_zone_t *fluid_inst_get_global_zone(fluid_inst_t *inst);

/*
 * fluid_inst_zone_t
 */
struct _fluid_inst_zone_t
{
    fluid_inst_zone_t *next;
    char *name;
    fluid_sample_t *sample;
    fluid_zone_range_t range;
    fluid_gen_t gen[GEN_LAST];
    fluid_mod_t *mod;  /* List of modulators */
};


fluid_inst_zone_t *new_fluid_inst_zone(char *name);
void delete_fluid_inst_zone(fluid_inst_zone_t *zone);
fluid_inst_zone_t *fluid_inst_zone_next(fluid_inst_zone_t *zone);
int fluid_inst_zone_import_sfont(fluid_inst_zone_t *inst_zone, fluid_inst_zone_t *global_inst_zone, SFZone *sfzone, fluid_defsfont_t *defsfont, SFData *sfdata);
fluid_sample_t *fluid_inst_zone_get_sample(fluid_inst_zone_t *zone);


int fluid_sample_import_sfont(fluid_sample_t *sample, SFSample *sfsample, fluid_defsfont_t *defsfont);
int fluid_sample_in_rom(fluid_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif  /* _FLUID_SFONT_H */
