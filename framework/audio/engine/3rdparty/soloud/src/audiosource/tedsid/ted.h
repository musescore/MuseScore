
class TED 
{
public: 
    unsigned int    masterVolume;
    int             Volume;
    int             Snd1Status;
    int             Snd2Status;
    int             SndNoiseStatus;
    int             DAStatus;
    unsigned short  Freq1;
    unsigned short  Freq2;
    int             NoiseCounter;
    int             FlipFlop[2];
    int             dcOutput[2];
    int             oscCount[2];
    int             OscReload[2];
    int             waveForm[2];
    int             oscStep;
    int             sampleRate;
    unsigned char   noise[256]; // 0-8
    unsigned int channelMask[3];
    int             vol;

    TED();
    void enableChannel(unsigned int channel, bool enable);
    void setFreq(unsigned int channel, int freq);
    void oscillatorReset();
    void oscillatorInit();
    void writeSoundReg(unsigned int reg, unsigned char value);
    void storeToBuffer(short *buffer, unsigned int count);
    unsigned int waveSquare(unsigned int channel);
    unsigned int waveSawTooth(unsigned int channel);
    unsigned int waveTriangle(unsigned int channel);
    unsigned int getWaveSample(unsigned int channel, unsigned int wave);
    void renderSound(unsigned int nrsamples, short *buffer);
    void setMasterVolume(unsigned int shift);
    void selectWaveForm(unsigned int channel, unsigned int wave);
    void setplaybackSpeed(unsigned int speed);
    unsigned int getTimeSinceLastReset();
    void setSampleRate(unsigned int value);
    void setFilterOrder(unsigned int value);
    void initFilter(unsigned int sampleRate_, unsigned int filterOrder_);
};