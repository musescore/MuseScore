#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vorbis/codec.h>
#include "sfont.h"

namespace FluidS {

//---------------------------------------------------------
//   decompressOggVorbis
//---------------------------------------------------------

bool Sample::decompressOggVorbis(char* src, int size)
      {
#define MAX_OUT   1024*500
      short odata[MAX_OUT];
      short* oPtr = odata;

      ogg_sync_state   oy; // sync and verify incoming physical bitstream
      ogg_stream_state os; // take physical pages, weld into a logical stream of packets
      ogg_page         og; // one Ogg bitstream page. Vorbis packets are inside
      ogg_packet       op; // one raw packet of data for decode
      vorbis_info      vi; // struct that stores all the static vorbis bitstream settings
      vorbis_comment   vc; // struct that stores all the bitstream user comments

      ogg_sync_init(&oy); // Now we can read pages

      bool eos = false;

      /* grab some data at the head of the stream. We want the first page
         (which is guaranteed to be small and only contain the Vorbis
         stream initial header) We need the first page to get the stream
         serialno. */

      char* buffer = ogg_sync_buffer(&oy, size);
      memcpy(buffer, src, size);
      ogg_sync_wrote(&oy, size);

      /* Get the first page. */
      if (ogg_sync_pageout(&oy, &og) != 1) {
            /* error case.  Must not be Vorbis data */
            fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
            return false;
            }

      /* Get the serial number and set up the rest of decode. */
      /* serialno first; use it to set up a logical stream */
      ogg_stream_init(&os, ogg_page_serialno(&og));

      /* extract the initial header from the first page and verify that the
         Ogg bitstream is in fact Vorbis data */

      /* I handle the initial header first instead of just having the code
         read all three Vorbis headers at once because reading the initial
         header is an easy way to identify a Vorbis bitstream and it's
         useful to see that functionality seperated out. */

      vorbis_info_init(&vi);
      vorbis_comment_init(&vc);
      if (ogg_stream_pagein(&os,&og) < 0) {
            /* error; stream version mismatch perhaps */
            fprintf(stderr,"Error reading first page of Ogg bitstream data.\n");
            return false;
            }

      if (ogg_stream_packetout(&os,&op) != 1) {
            /* no page? must not be vorbis */
            fprintf(stderr,"Error reading initial header packet.\n");
            return false;
            }

      if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
            /* error case; not a vorbis header */
            fprintf(stderr,"This Ogg bitstream does not contain Vorbis "
               "audio data.\n");
            return false;
            }

      /* At this point, we're sure we're Vorbis. We've set up the logical
         (Ogg) bitstream decoder. Get the comment and codebook headers and
         set up the Vorbis decoder */

      /* The next two packets in order are the comment and codebook headers.
         They're likely large and may span multiple pages. Thus we read
         and submit data until we get our two packets, watching that no
         pages are missing. If a page is missing, error out; losing a
         header page is the only place where missing data is fatal. */

      int i = 0;
      while (i < 2) {
            while (i < 2) {
                  int result = ogg_sync_pageout(&oy, &og);
                  if (result == 0)
                        break; /* Need more data */
                  /* Don't complain about missing or corrupt data yet. We'll
                    catch it at the packet output phase */
                  if (result == 1) {
                        ogg_stream_pagein(&os, &og);
                        while (i < 2) {
                              result = ogg_stream_packetout(&os, &op);
                              if (result == 0)
                                    break;
                              if (result < 0) {
                                    /* Uh oh; data at some point was corrupted or missing!
                                       We can't tolerate that in a header.  Die. */
                                    fprintf(stderr,"Corrupt secondary header.  Exiting.\n");
                                    return false;
                                    }
                              if (vorbis_synthesis_headerin(&vi,&vc,&op) < 0) {
                                    fprintf(stderr,"Corrupt secondary header.  Exiting.\n");
                                    return false;
                                    }
                              i++;
                              }
                        }
                  }
            }

      /* OK, got and parsed all three headers. Initialize the Vorbis
         packet->PCM decoder. */
      vorbis_dsp_state vd;    // central working state for the packet->PCM decoder

      if (vorbis_synthesis_init(&vd, &vi) == 0) { /* central decode state */
            vorbis_block vb; // local working space for packet->PCM decode
            vorbis_block_init(&vd, &vb);

            /* The rest is just a straight decode loop until end of stream */
            while(!eos) {
                  while(!eos) {
                        int result = ogg_sync_pageout(&oy, &og);
                        if (result == 0)
                              break; /* need more data */
                        if (result<0) { /* missing or corrupt data at this page position */
                              fprintf(stderr,"Corrupt or missing data in bitstream; "
                                 "continuing...\n");
                              }
                        else {
                              ogg_stream_pagein(&os, &og);
                              while (1) {
                                    result = ogg_stream_packetout(&os, &op);
                                    if (result == 0)
                                          break; /* need more data */
                                    if (result<0) { /* missing or corrupt data at this page position */
                                                    /* no reason to complain; already complained above */
                                          }
                                    else {
                                          /* we have a packet.  Decode it */
                                          float **pcm;
                                          int samples;

                                          if (vorbis_synthesis(&vb, &op) == 0) /* test for success! */
                                                vorbis_synthesis_blockin(&vd, &vb);

                                          while ((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0) {
                                                for (int j = 0; j < samples; j++) {
                                                      int val = floor(pcm[0][j] * 32767.f + .5f);
                                                      /* might as well guard against clipping */
                                                      if (val > 32767)
                                                            val = 32767;
                                                      if (val < -32768)
                                                            val = -32768;
                                                      *oPtr++ = val;
                                                      if ((oPtr - odata) >= MAX_OUT)
                                                            abort();
                                                      }
                                                vorbis_synthesis_read(&vd, samples);
                                                }
                                          }
                                    }
                              if (ogg_page_eos(&og))
                                    eos = true;
                              }
                        }
                  }
            /* ogg_page and ogg_packet structs always point to storage in
               libvorbis.  They're never freed or manipulated directly */

            vorbis_block_clear(&vb);
            vorbis_dsp_clear(&vd);
            }
      else{
            fprintf(stderr,"Error: Corrupt header during playback initialization.\n");
            }

      ogg_stream_clear(&os);
      vorbis_comment_clear(&vc);
      vorbis_info_clear(&vi);
      ogg_sync_clear(&oy);

      start = 0;
      end   = oPtr - odata;

      if (loopend > end ||loopstart >= loopend || loopstart <= start) {
            /* can pad loop by 8 samples and ensure at least 4 for loop (2*8+4) */
            if ((end - start) >= 20) {
                  loopstart = start + 8;
                  loopend   = end - 8;
                  }
            else {      // loop is fowled, sample is tiny (can't pad 8 samples)
                  loopstart = start + 1;
                  loopend   = end - 1;
                  }
            }
      if ((end - start) < 8) {
            printf("invalid sample\n");
            setValid(false);
            }

      data = new short[end];
      memcpy(data, odata, end * sizeof(short));
      end -= 1;

// printf("  vorbis sample 0-%d %d %d\n", end, loopstart, loopend);
      return true;
      }
} // namespace
