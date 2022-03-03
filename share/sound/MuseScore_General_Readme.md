# MuseScore_General.sf2

**Version 0.2**

---

Please see **MuseScore_General_License.md** for authorship and license information.

The purpose of this README is to provide useful information on instruments contained within MuseScore_General. It is currently a work-in-progress.

## About

This is a scaled-down version of **MuseScore_General-HQ.sf2** that replaces some of the larger instruments to save memory and CPU on older PCs. This SoundFont is currently a work-in-progress. Detailed information on presets and sample sources used can be found in "MuseScore_General_Sample_Sources.csv". All instruments without attribution are still using samples from FluidR3Mono.

## SoundFont Compatibility

**MuseScore_General** makes full use of SoundFont 2.01 specification modulators (particularly in the newer instruments) and requires a player/sampler with robust support for the standard. To my knowledge, the only SoundFont players that can accurately play this SoundFont are:

* [MuseScore](https://musescore.org)
* [FluidSynth](http://www.fluidsynth.org/)
* [Sobanth VSTi](https://blog.rosseaux.net/page/e5ca75d98990e33b31dadc78a8df1333/Sobanth)
* Sound Blaster Audigy/Audigy2 hardware SoundFont synth (probably X-Fi as well)

The only SoundFont editors that can play this SoundFont correctly are:

* Creative Vienna SoundFont Studio (requires Sound Blaster or E-MU hardware synth with SoundFont 2.01 modulator support)
* [SWAMI](http://www.swamiproject.org/) (uses FluidSynth)

## Presets

### General MIDI Presets

**MuseScore_General** is compatible with the [General MIDI standard](https://en.wikipedia.org/wiki/General_MIDI) with some additional presets from the [Roland GS standard](https://en.wikipedia.org/wiki/Roland_GS) as well.

### Fluid r3 Additional Drum Kits

Additional drum kits have been inherited from **Fluid r3**, beyond the kits specified in the [Roland GS standard](https://en.wikipedia.org/wiki/Roland_GS). It is possible that some of these kits will be removed in the future when new drum samples are added.

### Instrument Variations

In addition to the General MIDI presets, further instrument variations can be found on banks 20 and above, utilizing identical preset numbers so that General MIDI preset fallback can occur if ever the instrument becomes no longer available on the higher bank number. In other words, if you have a track assigned to bank #40, preset #48 "Celli Fast", then try to play it using a different General MIDI device or SoundFont, the preset will fall back to bank #0, preset #48 "Fast Strings" instead, and playback will at least sound somewhat correct.

### MuseScore Marching Percussion

The following marching percussion presets exist in the percussion bank (bank 128):
* 56: Marching Snare
* 57: OldMarchingBass
* 58: Marching Cymbals
* 59: Marching Bass
* 95: OldMarchingTenor
* 96: Marching Tenor

These presets are used for marching percussion support in MuseScore and do not conform to GM layout.

### Expressive Presets

As of version 0.1.5, **MuseScore_General** features expressive variants of all sustained presets, indicated by "Expr." at the end of the preset name. The dynamics of these presets are controlled using MIDI Control Change #2 (CC2), allowing fluid crescendos and diminuendos while a note is being held. This makes for much more realistic expression of strings, brass, woodwinds, etc. Note velocity no longer controls dynamics in these presets, but in some instruments, velocity will have some effect on the speed of the note attack. In MuseScore, the default (and ideal) behavior is for expressive instruments to have their dynamics controlled by sending identical values to both CC2 and note velocity (the latter only during note-on, naturally).

The expressive presets exist on higher bank numbers but use the same preset number as their non-expressive defaults. You can see what bank numbers the expressive presets use in column #2 ("Expr. Bank #") of the included **MuseScore_General_Sample_Sources.csv** file. The general rule is as follows:

* Bank 0 expressive presets are on Bank 17
* Bank 8 expressive presets are on Bank 18
* Bank 20-126 expressive presets are one bank higher (e.g., Bank 20 Expr. presets are on Bank 21)

### Dummy Presets

To maintain preset compatibility with the "HQ" version, **MuseScore_General** version contains dummy presets that are simply duplicates of the similar instruments found on bank 0. For example, the full SoundFont has the following instruments assigned to preset #48, all on different banks (presets listed in bank#:preset# format):

- 000:048 - Strings Fast
- 020:048 - Violins Fast
- 025:048 - Violins2 Fast
- 030:048 - Violas Fast
- 040:048 - Celli Fast
- 050:048 - Basses Fast

In the HQ version of the SoundFont, each of these sounds different since they feature unique samples for each section, but in this **MuseScore_General**, these presets on banks 20 and higher are mere duplicates of **000:048 - Strings Fast**, and only exist to avoid issues transitioning between the HQ and lighter versions of the SoundFont.

All dummy presets are indicated as such in the included **MuseScore_General_Sample_Sources.csv** file.
