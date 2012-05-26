/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include "libmscore/event.h"
#include "fluid.h"
#include "sfont.h"
#include "conv.h"
#include "gen.h"
#include "chorus.h"
#include "voice.h"
#include "msynth/sparm_p.h"

namespace FluidS {

/***************************************************************
 *
 *                         GLOBAL
 */

bool Fluid::initialized = false;

/* default modulators
 * SF2.01 page 52 ff:
 *
 * There is a set of predefined default modulators. They have to be
 * explicitly overridden by the sound font in order to turn them off.
 */

static const Mod defaultMod[] = {
      { GEN_ATTENUATION, FLUID_MOD_VELOCITY, FLUID_MOD_GC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE, 0, 0, 960.0 },
      { GEN_FILTERFC,
         FLUID_MOD_VELOCITY, FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE,
         FLUID_MOD_VELOCITY, FLUID_MOD_GC | FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE,
         -2400 },
      { GEN_VIBLFOTOPITCH, FLUID_MOD_CHANNELPRESSURE, FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 50 },
      { GEN_VIBLFOTOPITCH, 1, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 50 },
      { GEN_ATTENUATION, 7, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE, 0, 0, 960.0 },
      { GEN_PAN, 10, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 500.0 },
      { GEN_ATTENUATION, 11, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE, 0, 0, 960.0 },
      { GEN_REVERBSEND, 91, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 200 },
      { GEN_CHORUSSEND, 93, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 200 },
      { GEN_PITCH,
           FLUID_MOD_PITCHWHEEL,     FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR  | FLUID_MOD_POSITIVE,
           FLUID_MOD_PITCHWHEELSENS, FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE,
        12700.0 },
      };

//
// list of fluid paramters as saved in score
//
static SyntiParameter params[] = {
      SyntiParameter(SParmId(FLUID_ID, REVERB_GROUP, REVERB_ROOMSIZE).val, "RevRoomsize", 0.0),
      SyntiParameter(SParmId(FLUID_ID, REVERB_GROUP, REVERB_DAMP).val, "RevDamp",         0.0),
      SyntiParameter(SParmId(FLUID_ID, REVERB_GROUP, REVERB_WIDTH).val, "RevWidth",       0.0),
      SyntiParameter(SParmId(FLUID_ID, REVERB_GROUP, REVERB_GAIN).val, "RevGain",         0.0),

      SyntiParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_TYPE).val, "ChoType",         0.0),
      SyntiParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_SPEED).val, "ChoSpeed",       0.0),
      SyntiParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_DEPTH).val, "ChoDepth",       0.0),
      SyntiParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_BLOCKS).val, "ChoBlocks",     0.0),
      SyntiParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_GAIN).val, "ChoGain",         0.0),
      };

//---------------------------------------------------------
//   Fluid
//---------------------------------------------------------

Fluid::Fluid()
      {
      left_buf  = new float[FLUID_MAX_BUFSIZE];
      right_buf = new float[FLUID_MAX_BUFSIZE];
      fx_buf[0] = new float[FLUID_MAX_BUFSIZE];
      fx_buf[1] = new float[FLUID_MAX_BUFSIZE];
      reverb    = 0;
      chorus    = 0;
      silentBlocks = 0;
      }

//---------------------------------------------------------
//   init
//    static initialization
//---------------------------------------------------------

void Fluid::init()
      {
      initialized = true;
      fluid_conversion_config();
      Voice::dsp_float_config();
      }

//---------------------------------------------------------
//   init
//    instance initialization
//---------------------------------------------------------

void Fluid::init(int sr)
      {
      if (!initialized) // initialize all the conversion tables and other stuff
            init();

      sample_rate        = double(sr);
      sfont_id           = 0;
      _gain              = .2;

      _state       = FLUID_SYNTH_PLAYING; // as soon as the synth is created it starts playing.
      noteid      = 0;
      for (int i = 0; i < 128; ++i)
            _tuning[i] = i * 100.0;
      _masterTuning = 440.0;

      for (int i = 0; i < 512; i++)
            freeVoices.append(new Voice(this));

      reverb = new Reverb();
      chorus = new Chorus(sample_rate);
      reverb->setPreset(0);
      }

//---------------------------------------------------------
//   ~Fluid
//---------------------------------------------------------

Fluid::~Fluid()
      {
      _state = FLUID_SYNTH_STOPPED;
      foreach(Voice* v, activeVoices)
            delete v;
      foreach(Voice* v, freeVoices)
            delete v;
      foreach(SFont* sf, sfonts)
            delete sf;
      foreach(BankOffset* bankOffset, bank_offsets)
            delete bankOffset;
      foreach(Channel* c, channel)
            delete c;

      delete[] left_buf;
      delete[] right_buf;
      delete[] fx_buf[0];
      delete[] fx_buf[1];

      delete reverb;
      delete chorus;
      }

//---------------------------------------------------------
//   freeVoice
//---------------------------------------------------------

void Fluid::freeVoice(Voice* v)
      {
      if (activeVoices.removeOne(v))
            freeVoices.append(v);
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void Fluid::play(const Event& event)
      {
      bool err = false;
      int ch   = event.channel();

      if (ch >= channel.size()) {
            for (int i = channel.size(); i < ch+1; i++)
                  channel.append(new Channel(this, i));
            }

      int type    = event.type();
      Channel* cp = channel[ch];

      if (type == ME_NOTEON) {
            int key = event.dataA();
            int vel = event.dataB();
            if (vel == 0) {
                  //
                  // process note off
                  //
                  foreach (Voice* v, activeVoices) {
                        if (v->ON() && (v->chan == ch) && (v->key == key))
                              v->noteoff();
                        }
                  return;
                  }
            if (cp->preset() == 0) {
                  log("channel has no preset");
                  err = true;
                  }
            else {
                  /*
                   * If the same note is hit twice on the same channel, then the older
                   * voice process is advanced to the release stage.  Using a mechanical
                   * MIDI controller, the only way this can happen is when the sustain
                   * pedal is held.  In this case the behaviour implemented here is
                   * natural for many instruments.  Note: One noteon event can trigger
                   * several voice processes, for example a stereo sample.  Don't
                   * release those...
                   */
                  foreach(Voice* v, activeVoices) {
                        if (v->isPlaying() && (v->chan == ch) && (v->key == key) && (v->get_id() != noteid))
                              v->noteoff();
                        }
                  err = !cp->preset()->noteon(this, noteid++, ch, key, vel, event.tuning());
                  }
            }
      else if (type == ME_CONTROLLER)  {
            switch(event.controller()) {
                  case CTRL_PROGRAM:
                        program_change(ch, event.value());
                        break;
                  case CTRL_PITCH:
                        cp->pitchBend(event.value());
                        break;
                  case CTRL_PRESS:
                        break;
                  default:
                        cp->setcc(event.controller(), event.value());
                        break;
                  }
            }
      if (err)
            qWarning("FluidSynth error: event 0x%2x channel %d: %s\n",
               type, ch, qPrintable(error()));
      }

//---------------------------------------------------------
//   damp_voices
//---------------------------------------------------------

void Fluid::damp_voices(int chan)
      {
      foreach(Voice* v, activeVoices) {
            if ((v->chan == chan) && v->SUSTAINED())
                  v->noteoff();
            }
      }

//---------------------------------------------------------
//   allNotesOff
//---------------------------------------------------------

void Fluid::allNotesOff(int chan)
      {
      foreach(Voice* v, activeVoices) {
            if (chan == -1 || v->chan == chan)
                  v->noteoff();
            }
      }

//---------------------------------------------------------
//   allSoundsOff
//    immediately stop all notes on this channel.
//    stop all channel if chan==-1
//---------------------------------------------------------

void Fluid::allSoundsOff(int chan)
      {
      foreach(Voice* v, activeVoices) {
            if (chan == -1 || v->chan == chan)
                  v->off();
            }
      }

//---------------------------------------------------------
//   system_reset
//
//    Purpose:
//    Respond to the MIDI command 'system reset' (0xFF, big red 'panic' button)
//---------------------------------------------------------

void Fluid::system_reset()
      {
      foreach(Voice* v, activeVoices)
            v->off();
      foreach(Channel* c, channel)
            c->reset();
      chorus->reset();
      reverb->reset();
      }

/*
 * fluid_synth_modulate_voices
 *
 * tell all synthesis processes on this channel to update their
 * synthesis parameters after a control change.
 */
void Fluid::modulate_voices(int chan, bool is_cc, int ctrl)
      {
      foreach(Voice* v, activeVoices) {
            if (v->chan == chan)
                  v->modulate(is_cc, ctrl);
            }
      }

/*
 * fluid_synth_modulate_voices_all
 *
 * Tell all synthesis processes on this channel to update their
 * synthesis parameters after an all control off message (i.e. all
 * controller have been reset to their default value).
 */
void Fluid::modulate_voices_all(int chan)
      {
      foreach(Voice* v, activeVoices) {
            if (v->chan == chan)
                  v->modulate_all();
            }
      }

/*
 * fluid_synth_get_pitch_bend
 */
void Fluid::get_pitch_bend(int chan, int* ppitch_bend)
      {
      *ppitch_bend = channel[chan]->getPitchBend();
      }

/*
 * Fluid_synth_pitch_wheel_sens
 */
void Fluid::pitch_wheel_sens(int chan, int val)
      {
      /* set the pitch-bend value in the channel */
      channel[chan]->pitchWheelSens(val);
      }

/*
 * fluid_synth_get_preset
 */
Preset* Fluid::get_preset(unsigned int sfontnum, unsigned banknum, unsigned prognum)
      {
      SFont* sf = get_sfont_by_id(sfontnum);
      if (sf) {
            int offset     = get_bank_offset(sfontnum);
            Preset* preset = sf->get_preset(banknum - offset, prognum);
            if (preset != 0)
                  return preset;
            }
      return 0;
      }

Preset* Fluid::get_preset(char* sfont_name, unsigned banknum, unsigned prognum)
      {
      SFont* sf = get_sfont_by_name(sfont_name);
      if (sf) {
            int offset = get_bank_offset(sf->id());
            return sf->get_preset(banknum - offset, prognum);
            }
      return 0;
      }

//---------------------------------------------------------
//   find_preset
//---------------------------------------------------------

Preset* Fluid::find_preset(unsigned banknum, unsigned prognum)
      {
      Preset* preset = 0;
      foreach(SFont* sf, sfonts) {
            int offset = get_bank_offset(sf->id());
            preset = sf->get_preset(banknum - offset, prognum);
            if (preset)
                  break;
            }
      return preset;
      }

//---------------------------------------------------------
//   program_change
//---------------------------------------------------------

void Fluid::program_change(int chan, int prognum)
      {
      Channel* c       = channel[chan];
      unsigned banknum = c->getBanknum();
      c->setPrognum(prognum);

      Preset* preset = find_preset(banknum, prognum);

      unsigned sfont_id = preset? preset->sfont->id() : 0;
      c->setSfontnum(sfont_id);
      c->setPreset(preset);
      }

/*
 * fluid_synth_get_program
 */
void Fluid::get_program(int chan, unsigned* sfont_id, unsigned* bank_num, unsigned* preset_num)
      {
      Channel* c       = channel[chan];
      *sfont_id        = c->getSfontnum();
      *bank_num        = c->getBanknum();
      *preset_num      = c->getPrognum();
      }

//---------------------------------------------------------
//   program_select
//---------------------------------------------------------

bool Fluid::program_select(int chan, unsigned sfont_id, unsigned bank_num, unsigned preset_num)
      {
      Channel* c     = channel[chan];
      Preset* preset = get_preset(sfont_id, bank_num, preset_num);
      if (preset == 0) {
            log("There is no preset with bank number %d and preset number %d in SoundFont %d", bank_num, preset_num, sfont_id);
            return false;
            }

      /* inform the channel of the new bank and program number */
      c->setSfontnum(sfont_id);
      c->setBanknum(bank_num);
      c->setPrognum(preset_num);
      c->setPreset(preset);
      return true;
      }

/*
 * fluid_synth_program_select2
 */
bool Fluid::program_select2(int chan, char* sfont_name, unsigned bank_num, unsigned preset_num)
      {
      Channel* c = channel[chan];
      SFont* sf = get_sfont_by_name(sfont_name);
      if (sf == 0)
            return log("Could not find SoundFont %s", sfont_name);
      int offset     = get_bank_offset(sf->id());
      Preset* preset = sf->get_preset(bank_num - offset, preset_num);
      if (preset == 0)
            return log("There is no preset with bank number %d and preset number %d in SoundFont %s",
               bank_num, preset_num, sfont_name);

      /* inform the channel of the new bank and program number */
      c->setSfontnum(sf->id());
      c->setBanknum(bank_num);
      c->setPrognum(preset_num);
      c->setPreset(preset);
      return true;
      }

/*
 * fluid_synth_update_presets
 */
void Fluid::update_presets()
      {
      foreach(Channel* c, channel)
            c->setPreset(get_preset(c->getSfontnum(), c->getBanknum(), c->getPrognum()));
      }

/*
 * fluid_synth_program_reset
 *
 * Resend a bank select and a program change for every channel. This
 * function is called mainly after a SoundFont has been loaded,
 * unloaded or reloaded.  */

void Fluid::program_reset()
      {
      /* try to set the correct presets */
      int n = channel.size();
      for (int i = 0; i < n; i++)
            program_change(i, channel[i]->getPrognum());
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Fluid::process(unsigned len, float* out, float gain)
      {
      const int byte_size = len * sizeof(float) * 2;

      /* clean the audio buffers */
      memset(left_buf,  0, byte_size);
      memset(right_buf, 0, byte_size);
      memset(fx_buf[0], 0, byte_size);
      memset(fx_buf[1], 0, byte_size);

      if (mutex.tryLock()) {
            if (activeVoices.isEmpty())
                  silentBlocks--;
            else {
                  silentBlocks = SILENT_BLOCKS;
                  foreach (Voice* v, activeVoices)
                        v->write(len, left_buf, right_buf, fx_buf[0], fx_buf[1]);
                  }
            if (silentBlocks > 0) {
                  reverb->process(len, fx_buf[0], left_buf, right_buf);
                  chorus->process(len, fx_buf[1], left_buf, right_buf);
                  }
            mutex.unlock();
            }
      for (unsigned i = 0; i < len; i++) {
            *out++ += gain * left_buf[i];
            *out++ += gain * right_buf[i];
            }
      }

/*
 * fluid_synth_free_voice_by_kill
 *
 * selects a voice for killing. the selection algorithm is a refinement
 * of the algorithm previously in fluid_synth_alloc_voice.
 */

void Fluid::free_voice_by_kill()
      {
      float best_prio = 999999.;
      float this_voice_prio;
      Voice* best_voice = 0;

      foreach(Voice* v, activeVoices) {
            /* Determine, how 'important' a voice is.
             * Start with an arbitrary number */
            this_voice_prio = 10000.;

            /* Is this voice on the drum channel?
             * Then it is very important.
             * Also, forget about the released-note condition:
             * Typically, drum notes are triggered only very briefly, they run most
             * of the time in release phase.
             */
            if (v->chan == 9) {
                  this_voice_prio += 4000;

                  }
            else if (v->RELEASED()) {
                  /* The key for this voice has been released. Consider it much less important
                  * than a voice, which is still held.
                  */
                  this_voice_prio -= 2000.;
                  }

            if (v->SUSTAINED()) {
              /* The sustain pedal is held down on this channel.
               * Consider it less important than non-sustained channels.
               * This decision is somehow subjective. But usually the sustain pedal
               * is used to play 'more-voices-than-fingers', so it shouldn't hurt
               * if we kill one voice.
               */
                  this_voice_prio -= 1000;
                  }

            /* We are not enthusiastic about releasing voices, which have just been started.
             * Otherwise hitting a chord may result in killing notes belonging to that very same
             * chord.
             * So subtract the age of the voice from the priority - an older voice is just a little
             * bit less important than a younger voice.
             * This is a number between roughly 0 and 100.*/

            this_voice_prio -= (noteid - v->get_id());

            /* take a rough estimate of loudness into account. Louder voices are more important. */
            if (v->volenv_section != FLUID_VOICE_ENVATTACK) {
                  this_voice_prio += v->volenv_val * 1000.;
                  }

            /* check if this voice has less priority than the previous candidate. */
            if (this_voice_prio < best_prio) {
                  best_voice = v;
                  best_prio = this_voice_prio;
                  }
            }
      if (best_voice)
            best_voice->off();
      }

//---------------------------------------------------------
//   alloc_voice
//---------------------------------------------------------

Voice* Fluid::alloc_voice(unsigned id, Sample* sample, int chan, int key, int vel, double vt)
      {
      Channel* c = 0;

      /* check if there's an available synthesis process */
      if (freeVoices.isEmpty())
            free_voice_by_kill();

      if (freeVoices.isEmpty()) {
            log("Failed to allocate a synthesis process. (chan=%d,key=%d)", chan, key);
            return 0;
            }

      Voice* v = freeVoices.takeLast();
      activeVoices.append(v);

      if (chan >= 0)
            c = channel[chan];

      v->init(sample, c, key, vel, id, vt);

      /* add the default modulators to the synthesis process. */
      for (unsigned i = 0; i < sizeof(defaultMod)/sizeof(*defaultMod); ++i)
            v->add_mod(&defaultMod[i],  FLUID_VOICE_DEFAULT);
      return v;
      }

//---------------------------------------------------------
//   start_voice
//---------------------------------------------------------

void Fluid::start_voice(Voice* voice)
      {
      /* Find the exclusive class of this voice. If set, kill all voices
      * that match the exclusive class and are younger than the first
      * voice process created by this noteon event. */

      /** Kill all voices on a given channel, which belong into
          excl_class.  This function is called by a SoundFont's preset in
          response to a noteon event.  If one noteon event results in
          several voice processes (stereo samples), ignore_ID must name
          the voice ID of the first generated voice (so that it is not
          stopped). The first voice uses ignore_ID=-1, which will
          terminate all voices on a channel belonging into the exclusive
          class excl_class.
      */

      /* Check if the voice belongs to an exclusive class. In that case,
         previous notes from the same class are released. */

      int excl_class = voice->GEN(GEN_EXCLUSIVECLASS);
      if (excl_class) {

            /* Kill all notes on the same channel with the same exclusive class */

            foreach(Voice* existing_voice, activeVoices) {
                  /* Existing voice does not play? Leave it alone. */
                  if (!existing_voice->isPlaying())
                        continue;

                  /* An exclusive class is valid for a whole channel (or preset).
                   * Is the voice on a different channel? Leave it alone. */
                  if (existing_voice->chan != voice->chan)
                        continue;

                  /* Existing voice has a different (or no) exclusive class? Leave it alone. */
                  if ((int)existing_voice->GEN(GEN_EXCLUSIVECLASS) != excl_class)
                        continue;

                  /* Existing voice is a voice process belonging to this noteon
                   * event (for example: stereo sample)?  Leave it alone. */
                  if (existing_voice->get_id() == voice->get_id())
                        continue;
                  existing_voice->kill_excl();
                  }
            }
      voice->voice_start();
      }

//---------------------------------------------------------
//   updatePatchList
//---------------------------------------------------------

void Fluid::updatePatchList()
      {
      foreach(MidiPatch* p, patches)
            delete p;
      patches.clear();

      foreach(const SFont* sf, sfonts) {
            BankOffset* bo = get_bank_offset0(sf->id());
            int bankOffset = bo ? bo->offset : 0;
            foreach (Preset* p, sf->getPresets()) {
                  MidiPatch* patch = new MidiPatch;
                  patch->name = p->get_name();
                  patch->bank = p->get_banknum() + bankOffset;
                  patch->prog = p->get_num();
                  patch->drum = (p->get_banknum() == 128);
                  patches.append(patch);
                  }
            }
      }

//---------------------------------------------------------
//   soundFonts
//---------------------------------------------------------

QStringList Fluid::soundFonts() const
      {
      QStringList sf;
      foreach (SFont* f, sfonts)
            sf.append(f->get_name());
      return sf;
      }

//---------------------------------------------------------
//   loadSoundFont
//    return false on error
//---------------------------------------------------------

bool Fluid::loadSoundFonts(const QStringList& sl)
      {
      QStringList ol = soundFonts();
      if (ol == sl) {
            // printf("Fluid:loadSoundFonts: already loaded\n");
            return true;
            }
      mutex.lock();
      foreach(Voice* v, activeVoices)
            v->off();
      foreach(Channel* c, channel)
            c->reset();
      foreach (SFont* sf, sfonts)
            sfunload(sf->id(), true);
      bool ok = true;

      for (int i = sl.size() - 1; i >= 0; --i) {
            if (sfload(sl[i], true) == -1)
                  ok = false;
            }
      mutex.unlock();
      return ok;
      }

//---------------------------------------------------------
//   addSoundFont
//    return false on error
//---------------------------------------------------------

bool Fluid::addSoundFont(const QString& s)
      {
      mutex.lock();
      bool rv = (sfload(s, true) == -1) ? false : true;
      mutex.unlock();
      return rv;
      }

//---------------------------------------------------------
//   removeSoundFont
//    return false on error
//---------------------------------------------------------

bool Fluid::removeSoundFont(const QString& s)
      {
      mutex.lock();
      foreach(Voice* v, activeVoices)
            v->off();
      SFont* sf = get_sfont_by_name(s);
      sfunload(sf->id(), true);
      mutex.unlock();
      return true;
      }

//---------------------------------------------------------
//   sfload
//---------------------------------------------------------

int Fluid::sfload(const QString& filename, bool reset_presets)
      {
      if (filename.isEmpty())
            return -1;

      SFont* sf = new SFont(this);
      if (!sf->read(filename)) {
            delete sf;
            sf = 0;
            return -1;
            }

      sf->setId(++sfont_id);

      /* insert the sfont as the first one on the list */
      sfonts.prepend(sf);

      /* reset the presets for all channels */
      if (reset_presets)
            program_reset();

      updatePatchList();
      return sf->id();
      }

//---------------------------------------------------------
//   sfunload
//---------------------------------------------------------

bool Fluid::sfunload(int id, bool reset_presets)
      {
      SFont* sf = get_sfont_by_id(id);

      if (!sf) {
            printf("No SoundFont with id = %d", id);
            return false;
            }

      sfonts.removeAll(sf);   // remove the SoundFont from the list

      /* reset the presets for all channels */
      if (reset_presets)
            program_reset();
      else
            update_presets();
      delete sf;
      updatePatchList();
      return true;
      }

//---------------------------------------------------------
//   add_sfont
//---------------------------------------------------------

int Fluid::add_sfont(SFont* sf)
      {
	sf->setId(++sfont_id);

	/* insert the sfont as the first one on the list */
      sfonts.prepend(sf);

	/* reset the presets for all channels */
	program_reset();
	return sf->id();
      }

//---------------------------------------------------------
//   remove_sfont
//---------------------------------------------------------

void Fluid::remove_sfont(SFont* sf)
      {
	int sfont_id = sf->id();
	sfonts.removeAll(sf);

	remove_bank_offset(sfont_id); /* remove a possible bank offset */
	program_reset();              /* reset the presets for all channels */
      }

//---------------------------------------------------------
//   get_sfont_by_id
//---------------------------------------------------------

SFont* Fluid::get_sfont_by_id(int id)
      {
      foreach(SFont* sf, sfonts) {
            if (sf->id() == id)
                  return sf;
            }
      return 0;
      }

//---------------------------------------------------------
//   get_sfont_by_name
//---------------------------------------------------------

SFont* Fluid::get_sfont_by_name(const QString& name)
      {
      foreach(SFont* sf, sfonts) {
            if (sf->get_name() == name)
                  return sf;
            }
      return 0;
      }

//---------------------------------------------------------
//   set_interp_method
//    Sets the interpolation method to use on channel chan.
//    If chan is < 0, then set the interpolation method on all channels.
//---------------------------------------------------------

void Fluid::set_interp_method(int chan, int interp_method)
      {
      foreach(Channel* c, channel) {
            if (chan < 0 || c->getNum() == chan)
                  c->setInterpMethod(interp_method);
            }
      }

//---------------------------------------------------------
//   set_gen
//---------------------------------------------------------

void Fluid::set_gen(int chan, int param, float value)
      {
      channel[chan]->setGen(param, value, 0);
      foreach(Voice* v, activeVoices) {
            if (v->chan == chan)
                  v->set_param(param, value, 0);
            }
      }

/** Change the value of a generator. This function allows to control
    all synthesis parameters in real-time. The changes are additive,
    i.e. they add up to the existing parameter value. This function is
    similar to sending an NRPN message to the synthesizer. The
    function accepts a float as the value of the parameter. The
    parameter numbers and ranges are described in the SoundFont 2.01
    specification, paragraph 8.1.3, page 48. See also
    'fluid_gen_type'.

    Using the fluid_synth_set_gen2() function, it is possible to set
    the absolute value of a generator. This is an extension to the
    SoundFont standard. If 'absolute' is non-zero, the value of the
    generator specified in the SoundFont is completely ignored and the
    generator is fixed to the value passed as argument. To undo this
    behavior, you must call fluid_synth_set_gen2 again, with
    'absolute' set to 0 (and possibly 'value' set to zero).

    If 'normalized' is non-zero, the value is supposed to be
    normalized between 0 and 1. Before applying the value, it will be
    scaled and shifted to the range defined in the SoundFont
    specifications.

 */
void Fluid::set_gen2(int chan, int param, float value, int absolute, int normalized)
      {
      float v = (normalized)? fluid_gen_scale(param, value) : value;
      channel[chan]->setGen(param, v, absolute);

      foreach(Voice* vo, activeVoices) {
            if (vo->chan == chan)
                  vo->set_param(param, v, absolute);
            }
      }

float Fluid::get_gen(int chan, int param)
      {
      if ((param < 0) || (param >= GEN_LAST)) {
            log("Parameter number out of range");
            return 0.0;
            }
      return channel[chan]->getGen(param);
      }

BankOffset* Fluid::get_bank_offset0(int sfont_id) const
      {
      foreach(BankOffset* offset, bank_offsets) {
		if (offset->sfont_id == sfont_id)
			return offset;
	      }
	return 0;
      }

int Fluid::set_bank_offset(int sfont_id, int offset)
      {
	BankOffset* bank_offset = get_bank_offset0(sfont_id);

	if (bank_offset == 0) {
		bank_offset = new BankOffset;
		bank_offset->sfont_id = sfont_id;
		bank_offset->offset   = offset;
		bank_offsets.prepend(bank_offset);
	      }
      else {
	      bank_offset->offset = offset;
            }
	return 0;
      }

//---------------------------------------------------------
//   get_bank_offset
//---------------------------------------------------------

int Fluid::get_bank_offset(int sfont_id)
      {
      BankOffset* bank_offset = get_bank_offset0(sfont_id);
      return (bank_offset == 0)? 0 : bank_offset->offset;
      }

//---------------------------------------------------------
//   remove_bank_offset
//---------------------------------------------------------

void Fluid::remove_bank_offset(int sfont_id)
      {
	BankOffset* bank_offset = get_bank_offset0(sfont_id);
	if (bank_offset)
		bank_offsets.removeAll(bank_offset);
      }

//---------------------------------------------------------
//   parameter
//---------------------------------------------------------

SyntiParameter Fluid::parameter(int id) const
      {
      SParmId spid(id);
      int group = spid.subsystemId;
      int no    = spid.paramId;
      for (unsigned i = 0; i < sizeof(params)/sizeof(*params); ++i) {
            SyntiParameter& p = params[i];
            if (id == p.id()) {
                  if (group == REVERB_GROUP)
                        params[i].set(reverb->parameter(no));
                  else if (group == CHORUS_GROUP)
                        params[i].set(chorus->parameter(no));
                  return params[i];
                  }
            }
      printf("Fluid::parameter: (%d,%d,%d) not found\n",
         spid.syntiId, group, no);
      return SyntiParameter();
      }

//---------------------------------------------------------
//   setParameter
//---------------------------------------------------------

void Fluid::setParameter(int id, double value)
      {
      SParmId spid(id);
      if (spid.syntiId != FLUID_ID)
            return;
      if (spid.subsystemId == REVERB_GROUP)
            reverb->setParameter(spid.paramId, value);
      else if (spid.subsystemId == CHORUS_GROUP)
            chorus->setParameter(spid.paramId, value);
      }

/**
 * Print a message to the log.
 * @param fmt Printf style format string for log message
 * @param ... Arguments for printf 'fmt' message string
 * @return Always returns -1
 */

bool Fluid::log(const char* fmt, ...)
      {
      char buf[512];
      va_list args;
      va_start (args, fmt);
      vsnprintf(buf, sizeof(buf), fmt, args);
      va_end (args);
      _error = buf;
      return false;
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SyntiState Fluid::state() const
      {
      SyntiState sp;

      QStringList sfl = soundFonts();

      foreach(QString sf, sfl)
            sp.append(SyntiParameter(SParmId(FLUID_ID, 0, 0).val, "soundfont", sf));

      //
      // fill in struct with actual values
      //
      for (unsigned i = 0; i < sizeof(params)/sizeof(*params); ++i) {
            SyntiParameter& p = params[i];
            SParmId spid(p.id());

            int group = spid.subsystemId;
            int no    = spid.paramId;

            if (group == REVERB_GROUP)
                  params[i].set(reverb->parameter(no));
            else if (group == CHORUS_GROUP)
                  params[i].set(chorus->parameter(no));
            else
                  printf("Fluid::state: unknown group %d\n", group);
            sp.append(params[i]);
            }
      return sp;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Fluid::setState(SyntiState& sp)
      {
      QStringList sfs;
      int n = sp.size();
      for (int i = 0; i < n; ++i) {
            SyntiParameter* p = &sp[i];
            int id = p->id();
            if (id == -1) {
                  //
                  // if id of parameter is invalid, name must be
                  // valid; lookup name in params table
                  //
                  for (unsigned i = 0; i < sizeof(params)/sizeof(*params); ++i) {
                        SyntiParameter& p2 = params[i];
                        if (p2.name() == p->name()) {
                              id = p2.id();
                              p->setId(id);
                              break;
                              }
                        }
                  if (id == -1 && p->name() == "soundfont")
                        id = SParmId(FLUID_ID, 0, 0).val;
                  if (id == -1)     // not for this synthesizer
                        continue;
                  }
            SParmId spid(id);
            int group = spid.subsystemId;
            if (spid.syntiId != FLUID_ID)
                  continue;
            if (group == FLUID_GROUP)
                  sfs.append(p->sval());
            }
      loadSoundFonts(sfs);
      foreach(const SyntiParameter& p, sp) {
            int id = p.id();
            if (id == -1)
                  continue;
            SParmId spid(id);
            if (spid.syntiId != FLUID_ID)
                  continue;
            int group = spid.subsystemId;
            int no    = spid.paramId;

            if (group == FLUID_GROUP)
                  ;
            else if (group == REVERB_GROUP)
                  reverb->setParameter(no, p.fval());
            else if (group == CHORUS_GROUP)
                  chorus->setParameter(no, p.fval());
            else
                  printf("Fluid::setState: unknown group %d\n", group);
            }
      }
}
