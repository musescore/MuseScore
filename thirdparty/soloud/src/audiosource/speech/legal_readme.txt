The speech synth is based on rsynth by the late 
Nick Ing-Simmons (et al).

He described the legal status as:

    This is a text to speech system produced by 
    integrating various pieces of code and tables 
    of data, which are all (I believe) in the 
    public domain.
    
Since then, the rsynth source code has passed legal
checks by several open source organizations, so it
"should" be pretty safe.

The primary copyright claims seem to have to do
with text-to-speech dictionary use, which I've
removed completely.

I've done some serious refactoring, clean-up and 
feature removal on the source, as all I need is 
"a" free, simple speech synth, not a "good" 
speech synth. Since I've removed a bunch of stuff,
this is probably safer public domain release
than the original.

(I'm rather surprised there's no good public domain
speech synths out there; after all, it's 2013..)

I'm placing my changes in public domain as well,
or if that's not acceptable for you, then CC0:
http://creativecommons.org/publicdomain/zero/1.0/

The SoLoud interface files (soloud_speech.*) are
under ZLib/LibPNG license.

-- Jari Komppa
   2013
   