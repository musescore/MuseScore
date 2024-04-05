#ifndef CONFIG_H
#define CONFIG_H

/* Define to enable ALSA driver */
#cmakedefine ALSA_SUPPORT @ALSA_SUPPORT@

/* Define to activate sound output to files */
#cmakedefine AUFILE_SUPPORT @AUFILE_SUPPORT@

/* whether or not we are supporting CoreAudio */
#cmakedefine COREAUDIO_SUPPORT @COREAUDIO_SUPPORT@

/* whether or not we are supporting CoreMIDI */
#cmakedefine COREMIDI_SUPPORT @COREMIDI_SUPPORT@

/* whether or not we are supporting DART */
#cmakedefine DART_SUPPORT @DART_SUPPORT@

/* Define if building for Mac OS X Darwin */
#cmakedefine DARWIN @DARWIN@

/* Define if D-Bus support is enabled */
#cmakedefine DBUS_SUPPORT  @DBUS_SUPPORT@

/* Soundfont to load automatically in some use cases */
#cmakedefine DEFAULT_SOUNDFONT "@DEFAULT_SOUNDFONT@"

/* Define to enable FPE checks */
#cmakedefine FPE_CHECK @FPE_CHECK@

/* Define to 1 if you have the <arpa/inet.h> header file. */
#cmakedefine HAVE_ARPA_INET_H @HAVE_ARPA_INET_H@

/* Define to 1 if you have the <errno.h> header file. */
#cmakedefine HAVE_ERRNO_H @HAVE_ERRNO_H@

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H @HAVE_FCNTL_H@

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H @HAVE_INTTYPES_H@

/* Define to 1 if you have the <io.h> header file. */
#cmakedefine HAVE_IO_H @HAVE_IO_H@

/* whether or not we are supporting lash */
#cmakedefine HAVE_LASH @HAVE_LASH@

/* Define if systemd support is enabled */
#cmakedefine SYSTEMD_SUPPORT @SYSTEMD_SUPPORT@

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H @HAVE_LIMITS_H@

/* Define to 1 if you have the <linux/soundcard.h> header file. */
#cmakedefine HAVE_LINUX_SOUNDCARD_H @HAVE_LINUX_SOUNDCARD_H@

/* Define to 1 if you have the <machine/soundcard.h> header file. */
#cmakedefine HAVE_MACHINE_SOUNDCARD_H @HAVE_MACHINE_SOUNDCARD_H@

/* Define to 1 if you have the <math.h> header file. */
#cmakedefine HAVE_MATH_H @HAVE_MATH_H@

/* Define to 1 if you have the <netinet/in.h> header file. */
#cmakedefine HAVE_NETINET_IN_H @HAVE_NETINET_IN_H@

/* Define to 1 if you have the <netinet/tcp.h> header file. */
#cmakedefine HAVE_NETINET_TCP_H @HAVE_NETINET_TCP_H@

/* Define if compiling the mixer with multi-thread support */
#cmakedefine ENABLE_MIXER_THREADS @ENABLE_MIXER_THREADS@

/* Define if compiling with openMP to enable parallel audio rendering */
#cmakedefine HAVE_OPENMP @HAVE_OPENMP@

/* Define to 1 if you have the <pthread.h> header file. */
#cmakedefine HAVE_PTHREAD_H @HAVE_PTHREAD_H@

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H @HAVE_SIGNAL_H@

/* Define to 1 if you have the <stdarg.h> header file. */
#cmakedefine HAVE_STDARG_H @HAVE_STDARG_H@

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H @HAVE_STDINT_H@

/* Define to 1 if you have the <stdio.h> header file. */
#cmakedefine HAVE_STDIO_H @HAVE_STDIO_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H @HAVE_STDLIB_H@

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H @HAVE_STRINGS_H@

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H @HAVE_STRING_H@

/* Define to 1 if you have the <sys/mman.h> header file. */
#cmakedefine HAVE_SYS_MMAN_H @HAVE_SYS_MMAN_H@

/* Define to 1 if you have the <sys/socket.h> header file. */
#cmakedefine HAVE_SYS_SOCKET_H @HAVE_SYS_SOCKET_H@

/* Define to 1 if you have the <sys/soundcard.h> header file. */
#cmakedefine HAVE_SYS_SOUNDCARD_H @HAVE_SYS_SOUNDCARD_H@

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H @HAVE_SYS_STAT_H@

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H @HAVE_SYS_TIME_H@

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H @HAVE_SYS_TYPES_H@

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H @HAVE_UNISTD_H@

/* Define to 1 if you have the <windows.h> header file. */
#cmakedefine HAVE_WINDOWS_H @HAVE_WINDOWS_H@

/* Define to 1 if you have the <getopt.h> header file. */
#cmakedefine HAVE_GETOPT_H @HAVE_GETOPT_H@

/* Define to 1 if you have the inet_ntop() function. */
#cmakedefine HAVE_INETNTOP @HAVE_INETNTOP@

/* Define to enable JACK driver */
#cmakedefine JACK_SUPPORT @JACK_SUPPORT@

/* Define to enable PipeWire driver */
#cmakedefine PIPEWIRE_SUPPORT @PIPEWIRE_SUPPORT@

/* Include the LADSPA Fx unit */
#cmakedefine LADSPA @LADSPA_SUPPORT@

/* Define to enable IPV6 support */
#cmakedefine IPV6_SUPPORT @IPV6_SUPPORT@

/* Define to enable network support */
#cmakedefine NETWORK_SUPPORT @NETWORK_SUPPORT@

/* Defined when fluidsynth is build in an automated environment, where no MSVC++ Runtime Debug Assertion dialogs should pop up */
#cmakedefine NO_GUI @NO_GUI@

/* libinstpatch for DLS and GIG */
#cmakedefine LIBINSTPATCH_SUPPORT @LIBINSTPATCH_SUPPORT@

/* libsndfile has ogg vorbis support */
#cmakedefine LIBSNDFILE_HASVORBIS @LIBSNDFILE_HASVORBIS@

/* Define to enable libsndfile support */
#cmakedefine LIBSNDFILE_SUPPORT @LIBSNDFILE_SUPPORT@

/* Define to enable MidiShare driver */
#cmakedefine MIDISHARE_SUPPORT @MIDISHARE_SUPPORT@

/* Define if using the MinGW32 environment */
#cmakedefine MINGW32 @MINGW32@

/* Define to enable OSS driver */
#cmakedefine OSS_SUPPORT @OSS_SUPPORT@

/* Define to enable OPENSLES driver */
#cmakedefine OPENSLES_SUPPORT @OPENSLES_SUPPORT@

/* Define to enable Oboe driver */
#cmakedefine OBOE_SUPPORT @OBOE_SUPPORT@

/* Name of package */
#cmakedefine PACKAGE "@PACKAGE@"

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT @PACKAGE_BUGREPORT@

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME @PACKAGE_NAME@

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING @PACKAGE_STRING@

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME @PACKAGE_TARNAME@

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION @PACKAGE_VERSION@

/* Define to enable PortAudio driver */
#cmakedefine PORTAUDIO_SUPPORT @PORTAUDIO_SUPPORT@

/* Define to enable PulseAudio driver */
#cmakedefine PULSE_SUPPORT @PULSE_SUPPORT@

/* Define to enable DirectSound driver */
#cmakedefine DSOUND_SUPPORT @DSOUND_SUPPORT@

/* Define to enable Windows WASAPI driver */
#cmakedefine WASAPI_SUPPORT @WASAPI_SUPPORT@

/* Define to enable Windows WaveOut driver */
#cmakedefine WAVEOUT_SUPPORT @WAVEOUT_SUPPORT@

/* Define to enable Windows MIDI driver */
#cmakedefine WINMIDI_SUPPORT @WINMIDI_SUPPORT@

/* Define to enable SDL2 audio driver */
#cmakedefine SDL2_SUPPORT @SDL2_SUPPORT@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS @STDC_HEADERS@

/* Soundfont to load for unit testing */
#cmakedefine TEST_SOUNDFONT "@TEST_SOUNDFONT@"

/* Soundfont to load for UTF-8 unit testing */
#cmakedefine TEST_SOUNDFONT_UTF8_1 "@TEST_SOUNDFONT_UTF8_1@"
#cmakedefine TEST_SOUNDFONT_UTF8_2 "@TEST_SOUNDFONT_UTF8_2@"
#cmakedefine TEST_MIDI_UTF8 "@TEST_MIDI_UTF8@"

/* SF3 Soundfont to load for unit testing */
#cmakedefine TEST_SOUNDFONT_SF3 "@TEST_SOUNDFONT_SF3@"

/* Define to enable SIGFPE assertions */
#cmakedefine TRAP_ON_FPE @TRAP_ON_FPE@

/* Define to do all DSP in single floating point precision */
#cmakedefine WITH_FLOAT @WITH_FLOAT@

/* Define to profile the DSP code */
#cmakedefine WITH_PROFILING @WITH_PROFILING@

/* Define to use the readline library for line editing */
#cmakedefine READLINE_SUPPORT @READLINE_SUPPORT@

/* Define if the compiler supports VLA */ 
#cmakedefine SUPPORTS_VLA @SUPPORTS_VLA@ 

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#cmakedefine WORDS_BIGENDIAN @WORDS_BIGENDIAN@

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#cmakedefine inline @INLINE_KEYWORD@
#endif

/* Define to 1 if you have the sinf() function. */
#cmakedefine HAVE_SINF @HAVE_SINF@

/* Define to 1 if you have the cosf() function. */
#cmakedefine HAVE_COSF @HAVE_COSF@

/* Define to 1 if you have the fabsf() function. */
#cmakedefine HAVE_FABSF @HAVE_FABSF@

/* Define to 1 if you have the powf() function. */
#cmakedefine HAVE_POWF @HAVE_POWF@

/* Define to 1 if you have the sqrtf() function. */
#cmakedefine HAVE_SQRTF @HAVE_SQRTF@

/* Define to 1 if you have the logf() function. */
#cmakedefine HAVE_LOGF @HAVE_LOGF@

/* Define to 1 if you have the socklen_t type. */
#cmakedefine HAVE_SOCKLEN_T @HAVE_SOCKLEN_T@

#endif /* CONFIG_H */
