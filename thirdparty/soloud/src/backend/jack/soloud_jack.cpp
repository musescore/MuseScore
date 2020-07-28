/*
Copyright (c) 2019 Samson Close

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "soloud.h"

#if !defined(WITH_JACK)

namespace SoLoud
{
    result jack_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
        return NOT_IMPLEMENTED;
    }
};

#else

#include <jack/jack.h>
#include <string>

namespace SoLoud
{
    static jack_client_t *client;
    static jack_port_t **ports;
    static unsigned int portCount;

    static void jack_cleanup(Soloud * /*aSoloud*/)
    {
        jack_client_close(client);
    }

    int jack_callback(jack_nframes_t nframes, void* arg) {
        Soloud* soloud = (Soloud*) arg;
        jack_nframes_t dataLength = nframes * portCount;
        jack_nframes_t samples = nframes;

        jack_default_audio_sample_t data[dataLength];
        soloud->mix(data, samples);

        for (int i = 0; i < portCount; i++) {
            jack_default_audio_sample_t* buf = (jack_default_audio_sample_t*) jack_port_get_buffer(ports[i], samples);
            // de-interlaces the samples into each port buffer 121212 -> {111} {222}
            int c = 0;
            for (int j = i; j < dataLength; j += portCount) {
                buf[c] = data[j];
                c++;
            }
        }

        return 0;
    }

    result jack_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
    {
        aSoloud->mBackendCleanupFunc = jack_cleanup;

        // Starting Jack client
        if ((client = jack_client_open("Solound_Audio", JackNullOption, NULL)) == 0) return UNKNOWN_ERROR;
        aChannels = aChannels == 0 ? 1 : aChannels; // default to 1 channel if none are provided
        portCount = aChannels;
        ports = new jack_port_t*[portCount];
        // Registerring JACK Ports
        for (int i = 0; i < portCount; i++)
        {
            ports[i] = jack_port_register(client, ("channel_" + std::to_string(i + 1)).c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        }

        // Activating Jack client
        jack_set_process_callback(client, jack_callback, (void*) aSoloud);
        if (jack_activate(client)) return UNKNOWN_ERROR;

        // Connecting to audio ports
        const char** audioPorts = jack_get_ports(client, NULL, JACK_DEFAULT_AUDIO_TYPE, JackPortIsPhysical | JackPortIsInput);
        if (audioPorts == NULL)
        {
            return UNKNOWN_ERROR; // Cannot find any physical audio playback ports
        }
        else
        {
            for (int n = 0; audioPorts[n] != NULL; n++)
            {
                int m = jack_connect(client, jack_port_name(ports[n % portCount]), audioPorts[n]);
                // if (m) // Warning, cannot connect to audio output
            }
        }

        aSoloud->postinit_internal(aSamplerate, aBuffer, aFlags, aChannels);
        aSoloud->mBackendString = "JACK driver";
        return SO_NO_ERROR;
    }
};
#endif
