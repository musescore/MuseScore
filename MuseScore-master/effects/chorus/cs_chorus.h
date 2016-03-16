/*
  Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>
   
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __CS_CHORUS_H
#define __CS_CHORUS_H

#include "ladspaplugin.h"


class Ladspa_CS_chorus1 : public LadspaPlugin
{
public:

    enum { INPUT, OUTPUT, DELAY, FREQ1, TMOD1, FREQ2, TMOD2, NPORT };

    Ladspa_CS_chorus1 (unsigned long fsam);
    virtual void setport (unsigned long port, LADSPA_Data *data);  
    virtual void active  (bool act);  
    virtual void runproc (unsigned long len, bool add);
    virtual ~Ladspa_CS_chorus1 (void);  

private:

    float    *_port [NPORT];
    unsigned long  _size;
    unsigned long  _wi;
    unsigned long  _gi;
    float     _ri [3];
    float     _dr [3];  
    float     _x1, _y1;
    float     _x2, _y2;
    float    *_line;
};


class Ladspa_CS_chorus2 : public LadspaPlugin
{
public:

    enum { INPUT, OUTPUT, DELAY, FREQ1, TMOD1, FREQ2, TMOD2, NPORT };

    Ladspa_CS_chorus2 (unsigned long fsam);
    virtual void setport (unsigned long port, LADSPA_Data *data);  
    virtual void active  (bool act);  
    virtual void runproc (unsigned long len, bool add);
    virtual ~Ladspa_CS_chorus2 (void);  

private:

    float    *_port [NPORT];
    unsigned long  _size;
    unsigned long  _wi;
    unsigned long  _gi;
    float     _ri [3];
    float     _dr [3];  
    float     _x1, _y1;
    float     _x2, _y2;
    float     _a, _b;
    float    *_line;
};


class Ladspa_CS_chorus3 : public LadspaPlugin
{
public:

    enum { INPUT, OUTPUT1, OUTPUT2, OUTPUT3, DELAY, FREQ1, TMOD1, FREQ2, TMOD2, NPORT };

    Ladspa_CS_chorus3 (unsigned long fsam);
    virtual void setport (unsigned long port, LADSPA_Data *data);  
    virtual void active  (bool act);  
    virtual void runproc (unsigned long len, bool add);
    virtual ~Ladspa_CS_chorus3 (void);  

private:

    float    *_port [NPORT];
    unsigned long  _size;
    unsigned long  _wi;
    unsigned long  _gi;
    float     _ri [3];
    float     _dr [3];  
    float     _x1, _y1;
    float     _x2, _y2;
    float     _a, _b;
    float    *_line;
};


#endif
