#include "AudioEngine.h"
#include <QAudioDevice>
#include <QDebug>
#include <QMediaDevices>
#include <QtMath>

AudioEngine::AudioEngine(QObject *parent) : QObject(parent) { initAudio(); }

AudioEngine::~AudioEngine() { stop(); }

void AudioEngine::initAudio() {
  m_format.setSampleRate(44100);
  m_format.setChannelCount(1);
  m_format.setSampleFormat(QAudioFormat::Int16);

  QAudioDevice device = QMediaDevices::defaultAudioOutput();
  if (!device.isFormatSupported(m_format)) {
    qWarning() << "Default audio format not supported, trying to find closest";
    m_format = device.preferredFormat();
  }

  m_audioSink = std::make_unique<QAudioSink>(device, m_format, this);
}

void AudioEngine::playNote(int midiPitch, int durationMs) {
  stop();

  int frequency = midiToFrequency(midiPitch);
  QByteArray samples = generateSineWave(frequency, durationMs, 0.5f);

  m_audioBuffer.close();
  m_audioBuffer.setData(samples);
  m_audioBuffer.open(QIODevice::ReadOnly);

  m_audioSink->start(&m_audioBuffer);
  m_isPlaying = true;
  emit isPlayingChanged();

  // Auto-stop after duration
  QTimer::singleShot(durationMs + 50, this, [this]() {
    if (m_isPlaying) {
      m_isPlaying = false;
      emit isPlayingChanged();
      emit noteFinished();
    }
  });
}

void AudioEngine::playClick() {
  stop();

  QByteArray samples = generateClickSound(30); // 30ms click

  m_audioBuffer.close();
  m_audioBuffer.setData(samples);
  m_audioBuffer.open(QIODevice::ReadOnly);

  m_audioSink->start(&m_audioBuffer);
  m_isPlaying = true;
  emit isPlayingChanged();

  QTimer::singleShot(50, this, [this]() {
    m_isPlaying = false;
    emit isPlayingChanged();
  });
}

void AudioEngine::stop() {
  if (m_audioSink) {
    m_audioSink->stop();
  }
  m_audioBuffer.close();
  m_isPlaying = false;
  emit isPlayingChanged();
}

void AudioEngine::setTempo(int bpm) {
  if (bpm >= 40 && bpm <= 240 && bpm != m_tempo) {
    m_tempo = bpm;
    emit tempoChanged();
  }
}

QByteArray AudioEngine::generateSineWave(int frequency, int durationMs,
                                         float amplitude) {
  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  QByteArray samples;
  samples.resize(numSamples * sizeof(qint16));

  qint16 *data = reinterpret_cast<qint16 *>(samples.data());
  const float maxAmplitude = 32767.0f * amplitude;

  // Attack and release envelope (avoid clicks)
  int attackSamples = qMin(numSamples / 10, sampleRate / 100);  // 10ms max
  int releaseSamples = qMin(numSamples / 10, sampleRate / 100); // 10ms max

  for (int i = 0; i < numSamples; i++) {
    float t = float(i) / sampleRate;
    float sample = qSin(2.0f * M_PI * frequency * t);

    // Apply envelope
    float envelope = 1.0f;
    if (i < attackSamples) {
      envelope = float(i) / attackSamples;
    } else if (i > numSamples - releaseSamples) {
      envelope = float(numSamples - i) / releaseSamples;
    }

    data[i] = qint16(sample * maxAmplitude * envelope);
  }

  return samples;
}

QByteArray AudioEngine::generateClickSound(int durationMs) {
  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  QByteArray samples;
  samples.resize(numSamples * sizeof(qint16));

  qint16 *data = reinterpret_cast<qint16 *>(samples.data());
  const float maxAmplitude = 32767.0f * 0.7f;

  // Click is a short burst at 1000 Hz with fast decay
  for (int i = 0; i < numSamples; i++) {
    float t = float(i) / sampleRate;
    float decay = qExp(-t * 50); // Fast exponential decay
    float sample = qSin(2.0f * M_PI * 1000.0f * t) * decay;
    data[i] = qint16(sample * maxAmplitude);
  }

  return samples;
}

int AudioEngine::midiToFrequency(int midiPitch) {
  // A4 (MIDI 69) = 440 Hz
  return qRound(440.0 * qPow(2.0, (midiPitch - 69) / 12.0));
}

int AudioEngine::keyToRootMidi(const QString &key) {
  // Return MIDI pitch for the root note (octave 4)
  QString keyLower = key.toLower();

  if (keyLower.contains("c"))
    return 60;
  if (keyLower.contains("d"))
    return 62;
  if (keyLower.contains("e"))
    return 64;
  if (keyLower.contains("f"))
    return 65;
  if (keyLower.contains("g"))
    return 67;
  if (keyLower.contains("a"))
    return 69;
  if (keyLower.contains("b"))
    return 71;

  return 60; // Default to C
}

int AudioEngine::scaleDegreeToMidi(int degree, const QString &key) {
  // Major scale intervals from root: 0, 2, 4, 5, 7, 9, 11
  static const int majorIntervals[] = {0, 2, 4, 5, 7, 9, 11};

  // Minor scale intervals from root: 0, 2, 3, 5, 7, 8, 10
  static const int minorIntervals[] = {0, 2, 3, 5, 7, 8, 10};

  int rootMidi = keyToRootMidi(key);
  bool isMinor = key.toLower().contains("minor");

  // Scale degree 1-7, map to index 0-6
  int index = qBound(0, degree - 1, 6);

  int interval = isMinor ? minorIntervals[index] : majorIntervals[index];

  return rootMidi + interval;
}

