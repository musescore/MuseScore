/*
  ZynAddSubFX - a software synthesizer

  Microtonal.C - Tuning settings and microtonal capabilities
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <math.h>
#include "tuning.h"

#define MAX_LINE_SIZE 512

//---------------------------------------------------------
//   Tuning
//---------------------------------------------------------

Tuning::Tuning()
      {
      invertupdown       = false;
      invertupdowncenter = 60;
      octavesize         = 12;
      enabled            = false;
      aNote              = 69;
      aFreq              = 440.0;
      scaleshift         = 64;
      firstkey           = 0;
      lastkey            = 127;
      middlenote         = 60;
      mapsize            = 12;
      mappingenabled     = false;

      for (int i = 0; i < 128; i++)
            mapping[i] = i;

      for (int i = 0; i < MAX_OCTAVE_SIZE; i++) {
            octave[i].tuning = tmpoctave[i].tuning = pow(2,(i % octavesize + 1) / 12.0);
            octave[i].type   = tmpoctave[i].type   = 1;
            octave[i].x1     = tmpoctave[i].x1     = (i % octavesize + 1) * 100;
            octave[i].x2     = tmpoctave[i].x2     = 0;
            };
      octave[11].type  = 2;
      octave[11].x1    = 2;
      octave[11].x2    = 1;
      name             = "12tET";
      comment          = "Equal Temperament 12 notes per octave";
      globalfinedetune = 64;
      }

//---------------------------------------------------------
//   getoctavesize
//---------------------------------------------------------

int Tuning::getoctavesize()
      {
      return enabled ? octavesize : 12;
      }

//---------------------------------------------------------
//   getnotefreq
//    Get the frequency according the note number
//---------------------------------------------------------

float Tuning::getnotefreq(int note, int keyshift)
      {
      // in this function will appears many times things like this:
      // var = (a + b * 100) % b
      // I had written this way because if I use var=a%b gives unwanted results when a < 0
      // This is the same with divisions.

      if (invertupdown &&((!mappingenabled) || (!enabled)))
            note = invertupdowncenter * 2 - note;

      //compute global fine detune
      float globalfinedetunerap = pow(2.0,(globalfinedetune-64.0)/1200.0);//-64.0 .. 63.0 cents

      if (!enabled) {
            //12tET
            return pow(2.0,(note - aNote + keyshift) / 12.0) * aFreq * globalfinedetunerap;
            }

      int lscaleshift = (scaleshift - 64 + octavesize * 100) % octavesize;

      //compute the keyshift
      float rap_keyshift = 1.0;

      if (keyshift) {
            int kskey    =  (keyshift + octavesize * 100) % octavesize;
            int ksoct    =  (keyshift + octavesize * 100) /octavesize - 100;
            rap_keyshift =  (kskey==0) ? 1.0 : octave[kskey-1].tuning;
            rap_keyshift *= pow(octave[octavesize-1].tuning, ksoct);
            }

      //if the mapping is enabled
      float freq;
      if (mappingenabled) {
            if ((note < firstkey) || (note > lastkey))
                  return -1.0;

            //Compute how many mapped keys are from middle note to reference note
            //and find out the proportion between the freq. of middle note and "A" note

            int tmp    = aNote - middlenote;
            bool minus = false;
            if (tmp < 0) {
                  tmp = -tmp;
                  minus = true;
                  }
            int deltanote = 0;
            for (int i=0;i<tmp;i++)  {
                  if (mapping[i % mapsize] >= 0)
                        deltanote++;
                  }

            float rap_anote_middlenote = (deltanote == 0) ? 1.0 : octave[(deltanote-1) % octavesize].tuning;
            if (deltanote!=0)
                  rap_anote_middlenote*=pow(octave[octavesize-1].tuning,(deltanote-1)/octavesize);
            if (minus)
                  rap_anote_middlenote=1.0/rap_anote_middlenote;

            //Convert from note (midi) to degree (note from the tuning)
            int degoct = (note - middlenote + mapsize*200)/mapsize - 200;
            int degkey = (note - middlenote+(int)mapsize*100) % mapsize;
            degkey     = mapping[degkey];
            if (degkey < 0)
                  return -1.0;      //this key is not mapped

            //invert the keyboard upside-down if it is asked for
            //TODO: do the right way by using invertupdowncenter
            if (invertupdown) {
                  degkey=octavesize-degkey-1;
                  degoct=-degoct;
                  }
            //compute the frequency of the note
            degkey =  degkey+lscaleshift;
            degoct += degkey/octavesize;
            degkey %= octavesize;

            freq = (degkey==0) ? (1.0):octave[degkey-1].tuning;
            freq *= pow(octave[octavesize-1].tuning, degoct);
            freq *= aFreq/rap_anote_middlenote;
            }
      else {      //if the mapping is disabled
            int nt     = note - aNote + lscaleshift;
            int ntkey  = (nt + octavesize * 100) % octavesize;
            int ntoct  = (nt-ntkey) / octavesize;
            float oct  = octave[octavesize - 1].tuning;
            freq = octave[(ntkey + octavesize-1) % octavesize].tuning * pow(oct, ntoct) * aFreq;
            if (ntkey == 0)
                  freq /= oct;
            }
      if (lscaleshift)
            freq /= octave[scaleshift-1].tuning;
      freq *= globalfinedetunerap;
      return freq * rap_keyshift;
      }

//---------------------------------------------------------
//   linetotunings
//    Convert a line to tunings; returns true if it ok
//---------------------------------------------------------

bool Tuning::linetotunings(int nline, const char* line)
      {
      int  x1=-1, x2=-1, type=-1;
      float x=-1.0,tmp,tuning=1.0;

      if (strstr(line,"/") == 0) {
            if (strstr(line,".") == 0) {  // M case (M=M/1)
                  sscanf(line,"%d", &x1);
                  x2   = 1;
                  type = 2;   //division
                  }
            else {// float number case
                  sscanf(line,"%f",&x);
                  if (x < 0.000001)
                        return false;
                  type = 1;   //float type(cents)
                  }
            }
      else {      // M/N case
            sscanf(line,"%d/%d",&x1,&x2);
            if ((x1 < 0) || (x2 < 0))
                  return false;
            if (x2 == 0)
                  x2 = 1;
            type = 2;//division
            }

      if (x1 <= 0)
            x1 = 1;     //not allow zero frequency sounds (consider 0 as 1)

      //convert to float if the number are too big
      if ((type == 2) && ((x1 > (128*128*128-1)) || (x2 > (128*128*128-1)))) {
            type = 1;
            x = ((float) x1) / x2;
            }
      switch (type) {
            case 1:
                  x1     = (int) floor(x);
                  tmp    = fmod(x, 1.0);
                  x2     = (int) (floor (tmp*1e6));
                  tuning = pow(2.0,x/1200.0);
                  break;
            case 2:
                  tuning = ((float)x1)/x2;
                  break;
            }
      tmpoctave[nline].tuning = tuning;
      tmpoctave[nline].type   = type;
      tmpoctave[nline].x1     = x1;
      tmpoctave[nline].x2     = x2;
      return true;
      }

//---------------------------------------------------------
//   loadLine
//---------------------------------------------------------

QByteArray Tuning::loadLine(QFile* file)
      {
      QByteArray tmp;
      for (;;) {
            tmp = file->readLine();
            if (tmp.isEmpty() || tmp[0] != '!')
                  break;
            }
      return tmp;
      }

//---------------------------------------------------------
//   loadscl
//    return false on error
//---------------------------------------------------------

bool Tuning::loadscl(const QString& filename)
      {
      QFile file(filename);
      if (!file.open(QIODevice::ReadOnly))
            return false;

      //loads the short description
      QByteArray tmp = loadLine(&file).trimmed();
      if (tmp.isEmpty())
            return false;

      name    = tmp;
      comment = tmp;

      //loads the number of the notes
      tmp = loadLine(&file);
      if (tmp.isEmpty())
            return false;

      bool ok;
      int nnotes = tmp.toInt(&ok);
      if (!ok)
            nnotes = MAX_OCTAVE_SIZE;
      else if (nnotes > MAX_OCTAVE_SIZE)
            return false;

      //load the tunnings
      for (int nline = 0; nline < nnotes; nline++) {
            tmp = loadLine(&file);
            if (tmp.isEmpty())
                  return false;
            if (!linetotunings(nline, tmp.data()))
                  return false;
            }

      octavesize = nnotes;
      for (int i = 0; i < octavesize; i++) {
            octave[i].tuning = tmpoctave[i].tuning;
            octave[i].type   = tmpoctave[i].type;
            octave[i].x1     = tmpoctave[i].x1;
            octave[i].x2     = tmpoctave[i].x2;
            }
      return true;
      }

//---------------------------------------------------------
//   loadkbm
//    return false on error
//---------------------------------------------------------

bool Tuning::loadkbm(const QString& filename)
      {
      QFile file(filename);
      if (!file.open(QIODevice::ReadOnly))
            return false;

      int x;
      QByteArray tmp;
      bool ok;

      //loads the mapsize
      tmp = loadLine(&file);
      if (tmp.isEmpty())
            return false;
      x = tmp.toInt(&ok);
      if (!ok)
            return false;
      mapsize = qBound(1, x, 127);

      //loads first MIDI note to retune
      tmp = loadLine(&file);
      if (tmp.isEmpty())
            return false;
      x = tmp.toInt(&ok);
      if (!ok)
            return false;
      firstkey = qBound(1, x, 127);

      //loads last MIDI note to retune
      tmp = loadLine(&file);
      if (tmp.isEmpty())
            return false;
      x = tmp.toInt(&ok);
      if (!ok)
            return false;
      lastkey = qBound(1, x, 127);

      //loads last the middle note where scale for scale degree=0
      tmp = loadLine(&file);
      if (tmp.isEmpty())
            return false;
      x = tmp.toInt(&ok);
      if (!ok)
            return false;
      middlenote = qBound(1, x, 127);

      //loads the reference note
      tmp = loadLine(&file);
      if (tmp.isEmpty())
            return false;
      x = tmp.toInt(&ok);
      if (!ok)
            return false;
      aNote = qBound(1, x, 127);

      //loads the reference freq.
      tmp = loadLine(&file);
      if (tmp.isEmpty())
            return false;
      float tmpPAfreq = tmp.toFloat(&ok);
      if (!ok)
            return false;
      aFreq = tmpPAfreq;

      //the scale degree(which is the octave) is not loaded, it is obtained
      // by the tunnings with getoctavesize() method
      tmp = loadLine(&file);
      if (tmp.isEmpty())
            return false;

      //load the mappings
      if (mapsize) {
            for (int nline = 0; nline < mapsize; nline++) {
                  tmp = loadLine(&file);
                  if (tmp.isEmpty())
                        return false;
                  x = tmp.toInt(&ok);
                  mapping[nline] = ok ? x : -1;
                  }
            mappingenabled = true;
            }
      else {
            mappingenabled = false;
            mapping[0]     = 0;
            mapsize        = 1;
            }
      return true;
      }

