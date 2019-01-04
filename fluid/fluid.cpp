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

#include "synthesizer/event.h"
#include "synthesizer/msynthesizer.h"
#include "mscore/preferences.h"
#include "mscore/extension.h"

#include "fluid.h"
#include "sfont.h"
#include "conv.h"
#include "gen.h"
#include "voice.h"

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

static const Mod forcePanMod = { GEN_PAN, 10, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 1000.0 };

//---------------------------------------------------------
//   Fluid
//---------------------------------------------------------

Fluid::Fluid()
   : Synthesizer()
      {
      }

//---------------------------------------------------------
//   init
//    instance initialization
//---------------------------------------------------------

void Fluid::init(float sampleRate)
      {
      if (!initialized) {     // initialize all the conversion tables and other stuff
            initialized = true;
            fluid_conversion_config();
            Voice::dsp_float_config();
            }
      Synthesizer::init(sampleRate);
      sample_rate        = sampleRate;
      sfont_id           = 0;

      _state       = FLUID_SYNTH_PLAYING; // as soon as the synth is created it starts playing.
      noteid      = 0;
      for (int i = 0; i < 128; ++i)
            _tuning[i] = i * 100.0;
      _masterTuning = 440.0;

      for (int i = 0; i < 512; i++)
            freeVoices.append(new Voice(this));
      }

//---------------------------------------------------------
//   ~Fluid
//---------------------------------------------------------

Fluid::~Fluid()
      {
      _state = FLUID_SYNTH_STOPPED;
      _globalTerminate = true;
      while (!mutex.tryLock()) {}
      qDeleteAll(activeVoices);
      qDeleteAll(freeVoices);
      qDeleteAll(sfonts);
      qDeleteAll(channel);
      qDeleteAll(patches);
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

void Fluid::play(const PlayEvent& event)
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
                  for (Voice* v : activeVoices) {
                        if (v->ON() && (v->chan == ch) && (v->key == key))
                              v->noteoff();
                        }
                  return;
                  }
            if (cp->preset() == 0) {
                  qDebug("channel has no preset");
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
                  for(Voice* v : activeVoices) {
                        if (v->isPlaying() && (v->chan == ch) && (v->key == key) && (v->get_id() != noteid))
                              v->noteoff();
                        }
                  err = !cp->preset()->noteon(this, noteid++, ch, key, vel, event.tuning());
                  }
            }
      else if (type == ME_CONTROLLER) {
            switch(event.dataA()) {
                  case CTRL_PROGRAM:
                        program_change(ch, event.dataB());
                        break;
                  case CTRL_PRESS:
                        break;
                  default:
                        cp->setcc(event.dataA(), event.dataB());
                        break;
                  }
            }
      else if (type == ME_PITCHBEND){
            int midiPitch = event.dataB() * 128 + event.dataA();  // msb * 128 + lsb
            cp->pitchBend(midiPitch);
            }
      if (err) {
            // TODO: distinguish between types of error code.
            // Lack of a soundfont should not produce qDebug messages, because user could deliberately be using MIDI out only.
            //qWarning("FluidSynth error: event 0x%2x channel %d: %s", type, ch, qPrintable(error()));
            }
      }

//---------------------------------------------------------
//   damp_voices
//---------------------------------------------------------

void Fluid::damp_voices(int chan)
      {
      for(Voice* v : activeVoices) {
            if ((v->chan == chan) && v->SUSTAINED())
                  v->noteoff();
            }
      }

//---------------------------------------------------------
//   allNotesOff
//---------------------------------------------------------

void Fluid::allNotesOff(int chan)
      {
      for(Voice* v : activeVoices) {
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
      for(Voice* v : activeVoices) {
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
      for(Voice* v : activeVoices)
            v->off();
      for(Channel* c : channel)
            c->reset();
      }

/*
 * fluid_synth_modulate_voices
 *
 * tell all synthesis processes on this channel to update their
 * synthesis parameters after a control change.
 */
void Fluid::modulate_voices(int chan, bool is_cc, int ctrl)
      {
      for(Voice* v : activeVoices) {
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
      for(Voice* v : activeVoices) {
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
            Preset* preset = sf->get_preset(banknum, prognum);
            if (preset != 0)
                  return preset;
            }
      return 0;
      }

//---------------------------------------------------------
//   find_preset
//---------------------------------------------------------

Preset* Fluid::find_preset(unsigned banknum, unsigned prognum)
      {
      for (SFont* sf : sfonts) {
            Preset* preset = sf->get_preset(banknum, prognum);
            if (preset)
                  return preset;
            }
      return 0;
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
      if (!preset) {
            //Suppressing qDebug because might not have soundfont if using MIDI out only.
            //qDebug("Fluid::program_change: preset %d %d not found", banknum, prognum);
            preset = find_preset(0, prognum);
            if (!preset)
                  preset = find_preset(0, 0);
            }

      unsigned sfont_idl = preset? preset->sfont->id() : 0;
      c->setSfontnum(sfont_idl);
      c->setPreset(preset);
      }

/*
 * fluid_synth_get_program
 */
void Fluid::get_program(int chan, unsigned* sfont_idl, unsigned* bank_num, unsigned* preset_num)
      {
      Channel* c       = channel[chan];
      *sfont_idl       = c->getSfontnum();
      *bank_num        = c->getBanknum();
      *preset_num      = c->getPrognum();
      }

//---------------------------------------------------------
//   program_select
//---------------------------------------------------------

bool Fluid::program_select(int chan, unsigned sfont_idl, unsigned bank_num, unsigned preset_num)
      {
      Channel* c     = channel[chan];
      Preset* preset = get_preset(sfont_idl, bank_num, preset_num);
      if (preset == 0) {
            qDebug("There is no preset with bank number %d and preset number %d in SoundFont %d", bank_num, preset_num, sfont_idl);
            return false;
            }

      /* inform the channel of the new bank and program number */
      c->setSfontnum(sfont_idl);
      c->setBanknum(bank_num);
      c->setPrognum(preset_num);
      c->setPreset(preset);
      return true;
      }

//---------------------------------------------------------
//   update_presets
//---------------------------------------------------------

void Fluid::update_presets()
      {
      for (Channel* c : channel)
            c->setPreset(get_preset(c->getSfontnum(), c->getBanknum(), c->getPrognum()));
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Fluid::process(unsigned len, float* out, float* effect1, float* effect2)
      {
      if (mutex.tryLock()) {
            //we have to copy voices array for proper output sound processing in for loop
            auto tempVoices = activeVoices;
            for (Voice* v : tempVoices)
                  v->write(len, out, effect1, effect2);
            mutex.unlock();
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

      for(Voice* v : activeVoices) {
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
            qDebug("Failed to allocate a synthesis process. (chan=%d,key=%d)", chan, key);
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
      v->add_mod(&forcePanMod, FLUID_VOICE_OVERWRITE);
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

            for(Voice* existing_voice : activeVoices) {
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
      qDeleteAll(patches);
      patches.clear();

      QMap<int, QList<MidiPatch*>> patchNums;

      int bankOffset = 0;
      for (SFont* sf : sfonts) {
            sf->setBankOffset(bankOffset);
            int banks = 0;
            for (Preset* p : sf->getPresets()) {
                  MidiPatch* patch = new MidiPatch;
                  patch->drum = (p->get_banknum() == 128);
                  patch->synti = name();
                  if (p->get_banknum() > banks)
                        banks = p->get_banknum();
                  patch->bank = p->get_banknum() + bankOffset;
                  patch->prog = p->get_num();
                  patch->name = p->get_name();
                  patchNums[patch->prog].append(patch);
                  }
            bankOffset += (banks + 1);
            }

      // Order by patch number first instead of by bank first
      for (QList<MidiPatch*> num : patchNums) {
            for (MidiPatch* p : num)
                  patches.append(p);
            }

      /* try to set the correct presets */
      int n = channel.size();
      for (int i = 0; i < n; i++)
            program_change(i, channel[i]->getPrognum());
      }

//---------------------------------------------------------
//   soundFonts
//---------------------------------------------------------

QStringList Fluid::soundFonts() const
      {
      QStringList sf;
      for (SFont* f : sfonts)
            sf.append(QFileInfo(f->get_name()).fileName());
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
            qDebug("Fluid:loadSoundFonts: already loaded");
            return true;
            }
      QMutexLocker locker(&mutex);
      for(Voice* v : activeVoices)
            v->off();
      for(Channel* c : channel)
            c->reset();
      for (SFont* sf : sfonts)
            sfunload(sf->id());
      locker.unlock();
      bool ok = true;

      QFileInfoList l = sfFiles();
      for (int i = sl.size() - 1; i >= 0; --i) {
            QString s = sl[i];
            if (s.isEmpty())
                  continue;
            QString path;
            QFileInfo fis(s);
            QString fileName = fis.fileName();
            for (const QFileInfo& fi : l) {
                  if (fi.fileName() == fileName) {
                        path = fi.absoluteFilePath();
                        break;
                        }
                  }
            if (path.isEmpty()) {
                  qDebug("Fluid: sf <%s> not found", qPrintable(s));
                  ok = false;
                  }
            else {
                  locker.relock();
                  if (sfload(path) == -1) {
                        qDebug("loading sf failed: <%s>", qPrintable(path));
                        ok = false;
                        }
                  locker.unlock();
                  }
            }
      return ok;
      }

//---------------------------------------------------------
//   addSoundFont
//    return false on error
//---------------------------------------------------------

bool Fluid::addSoundFont(const QString& s)
      {
      QMutexLocker locker(&mutex);
      bool rv = (sfload(s) == -1) ? false : true;
      return rv;
      }

//---------------------------------------------------------
//   removeSoundFont
//    return false on error
//---------------------------------------------------------

bool Fluid::removeSoundFont(const QString& s)
      {
      QMutexLocker locker(&mutex);
      for(Voice* v : activeVoices)
            v->off();
      SFont* sf = get_sfont_by_name(s);
      sfunload(sf->id());
      return true;
      }

//---------------------------------------------------------
//   sfload
//---------------------------------------------------------

int Fluid::sfload(const QString& filename)
      {
      if (filename.isEmpty())
            return -1;

      SFont* sf = new SFont(this);
      try {
            if (!sf->read(filename)) {
                  delete sf;
                  sf = 0;
                  return -1;
                  }
            }
      catch(...) {
            delete sf;
            sf = 0;
            return -1;
            }

      sf->setId(++sfont_id);

      /* insert the sfont as the first one on the list */
      sfonts.prepend(sf);

      /* reset the presets for all channels */

      updatePatchList();
      return sf->id();
      }

//---------------------------------------------------------
//   sfunload
//---------------------------------------------------------

bool Fluid::sfunload(int id)
      {
      SFont* sf = get_sfont_by_id(id);

      if (!sf) {
            qDebug("No SoundFont with id = %d", id);
            return false;
            }

      sfonts.removeAll(sf);   // remove the SoundFont from the list
      updatePatchList();

      delete sf;
      return true;
      }

//---------------------------------------------------------
//   get_sfont_by_id
//---------------------------------------------------------

SFont* Fluid::get_sfont_by_id(int id)
      {
      for(SFont* sf : sfonts) {
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
      for(SFont* sf : sfonts) {
            if (QFileInfo(sf->get_name()).fileName() == name)
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
      for(Channel* c : channel) {
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
      for(Voice* v : activeVoices) {
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

      for(Voice* vo : activeVoices) {
            if (vo->chan == chan)
                  vo->set_param(param, v, absolute);
            }
      }

float Fluid::get_gen(int chan, int param)
      {
      if ((param < 0) || (param >= GEN_LAST)) {
            qDebug("Parameter number out of range");
            return 0.0;
            }
      return channel[chan]->getGen(param);
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SynthesizerGroup Fluid::state() const
      {
      SynthesizerGroup g;
      g.setName(name());

      QStringList sfl = soundFonts();
      for (QString sf : sfl)
            g.push_back(IdValue(0, sf));

      return g;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

bool Fluid::setState(const SynthesizerGroup& sp)
      {
      QStringList sfl;
      for (const IdValue& v : sp) {
            if (v.id == 0)
                  sfl.append(v.data);
            else
                  qDebug("Fluid::setState: unknown id %d", v.id);
            }
      return loadSoundFonts(sfl);
      }

//---------------------------------------------------------
//   collectFiles
//---------------------------------------------------------

static void collectFiles(QFileInfoList* l, const QString& path)
      {
      QDir dir(path);
      for (const QFileInfo& s : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (path == s.absoluteFilePath())
                  return;

            if (s.isDir() && !s.isHidden())
                  collectFiles(l, s.absoluteFilePath());
            else {
                  QString suffix = s.suffix().toLower();
                  if (suffix == "sf" || suffix == "sf2" || suffix == "sf3")
                        l->append(s);
                  }
            }
      }

//---------------------------------------------------------
//   sfFiles
//---------------------------------------------------------

QFileInfoList Fluid::sfFiles()
      {
      QFileInfoList l;

      QStringList pl = preferences.getString(PREF_APP_PATHS_MYSOUNDFONTS).split(";");
      pl.prepend(QFileInfo(QString("%1%2").arg(mscoreGlobalShare).arg("sound")).absoluteFilePath());

      // append extensions directory
      QStringList extensionsDir = Ms::Extension::getDirectoriesByType(Ms::Extension::soundfontsDir);
      pl.append(extensionsDir);

      foreach (const QString& s, pl) {
            QString ss(s);
            if (!s.isEmpty() && s[0] == '~')
                  ss = QDir::homePath() + s.mid(1);
            collectFiles(&l, ss);
            }
      return l;
      }
}
