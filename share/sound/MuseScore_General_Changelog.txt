# MuseScore_General.sf2

**Changelog**

---

## 0.2

* Restored [missing hi-hat and ride cymbal](https://musescore.org/en/node/305308) from **"128:056 Marching Snare"**.
* Fixed [bad loop in "Cello G4" sample](https://musescore.org/en/node/305333) in **"000:043 Contrabass"** and **"017:043 Contrabass Expr."**.
* Altered **"000:081 Saw Lead"** and **"017:081 Saw Lead Expr."** to reduce detuning effect and improve the sound. Users who prefer the old sound can find it at **"020:081 Detuned Saw"** and **"021:081 Detuned Saw Expr."**

## 0.1.9

* Worked around [MuseScore's broken volume envelope delay phase](https://musescore.org/en/node/291501) by removing the use of the delay phase from the following instruments:
  - **"000:016 Drawbar Organ"**
  - **"000:122 Sea Shore"**
  - **"008:014 Church Bell"** - this is the only instrument that sounds significantly different without the delay phase
  - **"008:016 Detuned Organ 1"**
  - **"008:017 Detuned Organ 2"**
  - **"008:030 Feedback Guitar"**
  - **"017:016 Drawbar Organ Expr."**
  - **"018:016 Detuned Org. 1 Expr."**
  - **"018:017 Detuned Org. 2 Expr."**
* **"008:031 Guitar Feedback"**
  - Fixed click on note release.
* **"000:022 Harmonica", "017:022 Harmonica Expr."**
  - Upper range extended to C8. Fixes part of [issue #299545](https://musescore.org/en/node/299545).
* **"000:052 Choir Aahs", "017:052 Choir Aahs Expr."**
  - Filtered out a high frequency noise present near the start of the "Ahh Choir F#4" sample. Thanks to forum user HuBandiT for the sample edit. Fixes [issue #299366](https://musescore.org/en/node/299366).
* **"000:091 Space Voice", "017:091 Space Voice Expr."**
  - Fixed panning on low notes. Fixes [issue #232886](https://musescore.org/en/node/232886).
  - Removed unnecessary use of duplicated sample zones--two copies of the same sample were playing simultaneously, panned hard left and right respectively. I presume this was done by the FluidR3 author to create a louder instrument, but this approach is unnecessary and chews up additional synth voices. I have removed the extra samples without any loss in instrument volume.

## 0.1.8

* Tweaked the use of the modulation envelope on the following presets and their expressive variants:
  - **"000:056 Trumpet"**
  - **"000:057 Trombone"**
  - **"000:059 Harmon Mute Trumpet"**
  - **"000:060 French Horns"**
  - **"000:071 Clarinet"**
  - **"008:063 Synth Brass 4"**
* Fixed the pitch of the sample at Ab2-A2 in **"000:017 Percussive Organ"**, **"008:017 Detuned Organ 2"** and their expressive variants.
* All instruments have been updated to cancel the default SoundFont 2.04 "velocity-to-filter cutoff" modulator. Previously, only the SoundFont 2.01 version of this modulator was being canceled, but now both versions should be canceled when using synths that support modulators. This change only impacts the use of the SoundFont outside of MuseScore.
* Added missing velocity override modulators in **"000:102 Echo Drops"** and its expressive variant.

## 0.1.6

* The SoundFont was renamed from "MuseScore_General_Lite.sf2" to "MuseScore_General.sf2" and will be the version that ships with MuseScore by default. The SoundFont formerly named "MuseScore_General.sf2" has been renamed to "MuseScore_General_HQ.sf2" and will be downloadable through the MuseScore resource manager. Currently, the only difference between the two versions is that the HQ version includes the new ensemble strings based on VSCO 2 samples. Over time, more instruments will be upgraded in the HQ version, and the difference between the two versions will grow.
* Added the new pianos from the full version of the SoundFont, updating the following presets:
  - **"000:000 Grand Piano"**
  - **"008:000 Mellow Grand Piano"**
  - **"000:001 Bright Grand Piano"**
  - **"000:002 Honky-Tonk Piano"**

## 0.1.5

* MuseScore_General now includes additional presets labeled "Expr." that can be dynamically controlled via MIDI Control Change #2 (CC2). To accommodate this new functionality, many instruments were reprogrammed to use modulators for velocity-based filtering rather than separate instrument layers within the preset. Please refer to the included "MuseScore_General_Readme.md" file for more information on these new presets.
* Reprogrammed the velocity-based effects for the following instruments:
  - **"000:056 Trumpet"**
  - **"000:057 Trombone"**
  - **"000:059 Harmon Mute Trumpet"**
  - **"000:060 French Horns"**
  - **"000:071 Clarinet"**

## 0.1.4

* Fixed clarinet & flute notes taking too long to sound when played in bass flute & bass clarinet range. <https://musescore.org/en/node/280907>, <https://musescore.org/en/node/280904>
* Filtered annoying overtones from Viola note E4 (sample "Viola E3") and softened the attack. <https://musescore.org/en/node/272992>
* Re-numbered ensemble strings bank numbers to make room for future expansion.

## 0.1.3

* There are now two versions of the SoundFont:
  - **MuseScore_General**: This is the version that will include all of the new instrument sounds as they are developed. To reach a higher sound quality, new instrument presets will often require more RAM and CPU than the older versions.
  - **MuseScore_General_Lite**: This version is intended for more limited computers and uses less RAM and CPU by retaining the older, smaller instrument sounds where it is advantageous to do so. Currently, the only difference between the two versions is the acoustic pianos and ensemble strings (plus some synth-style presets that also use the strings samples: "Warm Pad", "Orchestra Pad", "Synth Strings 3"), but this difference will grow much greater over time.
* Added "dummy" presets to **MuseScore_General_Lite** for preset compatibility with the new ensemble strings in the full version of **MuseScore_General**. These strings presets can be found on banks 20-32, but are merely duplicates of the ensemble strings presets present on bank 0 (Tremolo, Pizzicato, Fast and Slow strings).
* Removed the superfluous **"001:048 Dry Strings"** preset.
* Returned to the original FluidR3Mono pianos for lower memory consumption in **MuseScore_General_Lite**.
* Optimized the use of generators in all instruments, freeing thousands of generators for future instrument use (the limit is 65,535 instrument-level generators).

## 0.1.2

* **000-045: Pizzicato Strings**
  - Restored original stereo samples.
* **000-048: Strings** (also **"044: Tremolo Strings"** and **"049: Slow Strings"**, etc.)
  - Restored original stereo samples.
* **000-052: Choir Aahs**
  - Restored original stereo samples.
  - Fixed bad tuning. <https://musescore.org/en/node/272125>
  - Improved balance between the samples.
  - The tuning and balance fixes were also applied to the choir samples used in "000-102: Echo Drops".
* **001-115: Temple Blocks**
  - Normalized samples for accurate velocity-to-attenuation scaling.
  - Turned broken stereo samples (left channel only) into proper mono samples.
* Proper tuning fixes for violin, viola and recorder samples (fixed at sample level instead of using mod envelope).
* Updated marching percussion using samples from MDL.

## 0.1.1 (version released with MuseScore 2.2)
* All drum kits:
  - Reduced the volume of the hi-hat and cymbals as they were too loud compared to the bass and snare.
  - Made the closed hi-hat resonance vary with velocity.
  - Countered the reverb built into the sample for the cross stick rim tap (MIDI note 37).
  - Reduced the volume of the lowest two toms (standard kits).
* **000-011: Vibraphone** -- Softened the attack a bit. <https://musescore.org/en/node/231996>
* **000-024: Nylon String Guitar** -- Added more lowpass filter at lower velocities and made the high strings less bright.
* **000-029: Overdrive Guitar** -- fixed lowest sample being louder than the rest. Increased release time.
* **000-030: Distortion Guitar** -- increased release time.
* **000-031: Guitar Harmonics** -- increased release time.
* **000-040: Violin** -- Restored the missing B6 sample and fixed the pitch bend in it. Also improved the tuning on the remaining samples, countering pitch bends using the modulation envelope. <https://musescore.org/en/node/85636>
* **000-041: Viola** -- Fixed the pitch bend in the C#3 sample. Also improved the tuning on the remaining samples, countering pitch bends using the modulation envelope. <https://musescore.org/en/node/154801>
* **000-090: Polysynth** -- Fixed velocity-to-attack to avoid popping noise.
* **128-024: Electronic** -- Fixed toms that were panned hard left.
* **128-040: Brush** (and also "Brush 1" and "Brush 2") -- Fixed brushed snare samples being panned hard left.
* **128-048: Orchestra Kit** -- many fixes including:
  - Fixed samples not sustaining on note release.
  - Made it so that closed/foot hi-hat will silence open hi-hat.
  - Fixed triangle samples to loop instead of just cutting off.
  - Made muted triangle sound short; it was no different from sustained triangle. Muted triangle will now also silence the open triangle.
* **128-056: MarchingSnare** (and the rest of the marching percussion presets) -- many fixes including:
  - Fixed samples not sustaining on note release.
  - Fixed clicking loops on the tenor drum roll samples.
  - Fixed click at beginning of some of the tenor drum roll samples.
  - Slightly reduced the volume of the tenor drum rolls.
  - Enabled loop for cymbal rolls.
* **128-096: OldMarchingTenor** -- Remapped the tenor drum "stick click" from MIDI note 43 to 123. <https://musescore.org/en/node/196321#comment-823829>


## 0.1 (pre-release alpha)

* This is the first version that branches off from FluidR3_Mono.
* Renamed presets to remove brand names ("Yamaha", "Rhodes").
* Altered the velocity scale for all instruments to be less extreme between FF and PP.
* Added velocity-to-filter for a mellower sound at low velocities for several instruments.
* Reprogrammed strange velocity response behaviors on several instruments.
* Replaced the following instruments. Most of the synth sounds are newly programmed based on custom analog waveforms, leading to low RAM consumption yet very high-quality sound:
  - **000-000: Grand Piano** (also "Bright Grand", "Mellow Grand" and "Honky-Tonk") -- New, high-quality pianos based on the public domain "Splendid Grand" samples. These samples were originally created by AKAI for the AKAI S5000 sampler, and I was able to verify their public domain status via conversation with AKAI in 2007. The original close-mic sample set contained 250 MB of samples, but I have been able to program a very expressive instrument using only 94 MB of the samples. I also used the same sample-shifting technique as Roland to create "bright" and "mellow" versions of the pianos that actually have a different tonality to them rather than just using filter variance as the old pianos did. This same technique allowed me to create a honky-tonk piano that sounds somewhat legit, with none of the flanging effect that is typically heard on such presets. This allows all four pianos to sound unique while still sharing the same 94 MB sample set.
  - **000-005: FM Electric Piano**
  - **008-005: Detuned FM EP**
  - **000-010: Music Box**
  - **000-038: Synth Bass 1**
  - **000-039: Synth Bass 2**
  - **008-038: Synth Bass 3**
  - **008-039: Synth Bass 4**
  - **000-042: Cello**
  - **000-050: Synth Strings 1**
  - **000-051: Synth Strings 2**
  - **008-050: Synth Strings 3**
  - **000-062: Synth Brass 1**
  - **000-063: Synth Brass 2**
  - **008-062: Synth Brass 3**
  - **008-063: Synth Brass 4**
  - **000-080: Square Lead**
  - **008-080: Sine Wave**
  - **000-081: Saw Lead**
  - **000-086: 5th Saw Wave**
  - **000-087: Bass & Lead**
  - **000-088: Fantasia**
  - **000-090: Polysynth**
  - **000-092: Bowed Glass**
  - **000-093: Metal Pad**
  - **000-094: Halo Pad**
  - **000-095: Sweep Pad**
  - **000-095: Ice Rain**
  - **000-097: Soundtrack**
  - **000-098: Crystal**
  - **000-099: Atmosphere**
  - **000-100: Brightness**
  - **000-103: Star Theme**
	
* Performed the following noteworthy per-instrument improvements and fixes:
  - **000-004: Tine Electric Piano** -- Added velocity-to-filter for a mellower sound at low velocities.
  - **000-008: Celesta** -- The conversion to mono used the left samples, but "000-098: Crystal" used the right samples, so both left and right samples were still in the SoundFont. This problem was solved when I replaced the "Crystal" preset using new programming.
  - **000-024: Nylon Guitar** and **000-025: Steel String Guitar** -- Removed the weird velocity-triggered release, which would cause notes to ring out for a long time if they were played at a high velocity. I also added better velocity-to-filter cutoff mapping.
  - **008-025: 12-String Guitar** -- Manipulated the sample mapping to better simulate the sound of a real 12-string guitar.
  - **000-032: Acoustic Bass** -- Added velocity-to-filter for a mellower sound at low velocities.
  - **000-040: Violin** -- Improved tuning and used modulation envelope to counter some of the pitch bending on note attack.
  - **000-043: Contrabass** -- Fixed abrupt release at high velocities.
  - **000-044: Tremolo Strings** -- Improved realism of tremolo effect.
  - **000-045: Pizzicato** -- Countered the overly long reverb tail. Incorporated the use of the filter to create a more realistic sound across the dynamic range.
  - **000-048: Strings** (& variants) -- Eliminated the unnaturally long release, so staccato notes will now sound correct. Note tone and attack now follow velocity. I also extended the strings to the full MIDI range (previously they stopped at E7).
  - **000-057: Trombone** -- Fixed short release.
  - **000-058: Tuba** -- Release was too short for notes at low velocities.
  - **000-061: Brass Section** -- There was some strange programming in this one that caused two voices to be used up per note for no real benefit. Both voices were playing the same sample, one panned hard right and the other hard left with the only difference being slightly different filter settings. This resulted in notes that seemed to pan toward the left at lighter velocities. I fixed this to only use one voice per note and also removed the useless "Key on velocity -> initialFilterQ" modulator that had been added (it was attempting to subtract up to 47 dB from the filter Q, but nowhere was the filter Q greater than 0).
  - **000-069: English Horn** -- Tamed the crazy volume swell that occured on most notes. Unfortunately, this instrument ends up being a bit quieter than the other woodwinds. It would be necessary to edit the samples to remedy this.
  - **000-071: Clarinet** -- EQ'd the samples and added a bit of release for better realism. The original samples were incredibly nasally and cut off too abruptly on note release.
  - **000-074: Recorder** -- Duplicate samples were panned hard left and right for no discernable reason, which was causing twice the polyphony to be used. I fixed this to use single, center-panned samples instead. I also improved the tuning and countered the pitch bend at the beginning of some notes.
  - **000-089: Warm Pad** -- Reprogrammed using the strings samples to avoid needing dedicated samples for this preset.
  - **000-112: Tinker Bell** -- Fixed notes from middle C downward all playing the same pitch.
  - **128-025: TR-808** -- Fixed kit volume to match the other kits. It was being boosted way too loud.
  - **128-048: Orchestra Kit** -- Fixed snare cutting off abruptly on release.
