#include "AudioEngine.h"
#include <QAudioDevice>
#include <QDebug>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QtMath>
#include <cmath>

AudioEngine::AudioEngine(QObject *parent) : QObject(parent) { 
  initAudio(); 
}

AudioEngine::~AudioEngine() { stop(); }

void AudioEngine::initAudio() {
  m_format.setSampleRate(44100);
  m_format.setChannelCount(1);
  m_format.setSampleFormat(QAudioFormat::Int16);

  // Create media devices monitor to track output device changes
  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged,
          this, &AudioEngine::onAudioOutputsChanged);

  recreateAudioSink();
}

void AudioEngine::recreateAudioSink() {
  if (m_audioSink) {
    m_audioSink->stop();
    m_audioSink.reset();
  }
  m_audioBuffer.close();

  QAudioDevice device = QMediaDevices::defaultAudioOutput();
  
  if (!device.isNull()) {
    m_currentDeviceId = device.id();
    
    if (!device.isFormatSupported(m_format)) {
      m_format = device.preferredFormat();
    }

    m_audioSink = std::make_unique<QAudioSink>(device, m_format, this);
  } else {
    m_currentDeviceId.clear();
  }
}

void AudioEngine::onAudioOutputsChanged() {
  QAudioDevice device = QMediaDevices::defaultAudioOutput();
  QString newDeviceId = device.isNull() ? QString() : device.id();
  
  if (newDeviceId != m_currentDeviceId) {
    bool wasPlaying = m_isPlaying;
    recreateAudioSink();
    
    if (wasPlaying) {
      m_isPlaying = false;
      emit isPlayingChanged();
    }
  }
}

// ============================================================================
// Waveform Helpers
// ============================================================================

float AudioEngine::sineWave(float phase) {
  return qSin(phase);
}

float AudioEngine::triangleWave(float phase) {
  // Normalize phase to 0-2π
  float normalized = fmod(phase, 2.0f * M_PI);
  if (normalized < 0) normalized += 2.0f * M_PI;
  
  // Triangle: goes from 0 to 1 to -1 to 0
  if (normalized < M_PI / 2) {
    return normalized / (M_PI / 2);
  } else if (normalized < 3 * M_PI / 2) {
    return 1.0f - 2.0f * (normalized - M_PI / 2) / M_PI;
  } else {
    return -1.0f + (normalized - 3 * M_PI / 2) / (M_PI / 2);
  }
}

float AudioEngine::sawtoothWave(float phase) {
  float normalized = fmod(phase, 2.0f * M_PI);
  if (normalized < 0) normalized += 2.0f * M_PI;
  return (normalized / M_PI) - 1.0f;
}

float AudioEngine::softSquareWave(float phase) {
  // Soft square using sine harmonics (odd only, limited)
  float result = qSin(phase);
  result += 0.33f * qSin(3 * phase);
  result += 0.2f * qSin(5 * phase);
  return result * 0.6f; // Normalize
}

// ============================================================================
// Voice Generators
// ============================================================================

QByteArray AudioEngine::generateSketchPiano(int frequency, int durationMs) {
  // Sketch Piano: pluck transient + warm piano body + light pad
  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  QByteArray samples;
  samples.resize(numSamples * sizeof(qint16));
  qint16 *data = reinterpret_cast<qint16 *>(samples.data());
  
  const float maxAmplitude = 32767.0f * 0.45f;  // Reduced for headroom
  
  // ADSR envelope (per SOUND_DESIGN.md)
  int attackSamples = sampleRate * 8 / 1000;     // 8ms attack
  int decaySamples = sampleRate * 40 / 1000;     // 40ms decay
  int releaseSamples = sampleRate * 300 / 1000;  // 300ms release
  int releaseStart = numSamples - releaseSamples;
  
  for (int i = 0; i < numSamples; i++) {
    float t = float(i) / sampleRate;
    float phase = 2.0f * M_PI * frequency * t;
    
    // === Transient (pluck) ===
    // Short noise burst + high-freq click for attack clarity
    float transient = 0.0f;
    if (i < attackSamples) {
      float transientEnv = 1.0f - float(i) / attackSamples;
      transientEnv = transientEnv * transientEnv; // Quadratic decay
      // High-freq click (2-3x fundamental)
      transient = 0.4f * qSin(phase * 2.5f) * transientEnv;
      transient += 0.2f * qSin(phase * 4.0f) * transientEnv;
    }
    
    // === Piano body (warm harmonics) ===
    float body = 0.0f;
    body += 1.0f * sineWave(phase);           // Fundamental
    body += 0.5f * sineWave(phase * 2);       // 2nd harmonic
    body += 0.25f * sineWave(phase * 3);      // 3rd harmonic
    body += 0.12f * sineWave(phase * 4);      // 4th harmonic
    body += 0.06f * sineWave(phase * 5);      // 5th harmonic
    body *= 0.45f; // Normalize
    
    // === Pad underlay (sine + triangle, subtle) ===
    float pad = 0.0f;
    float padPhase = phase * 0.5f; // Sub-octave for warmth
    pad += 0.6f * sineWave(padPhase);
    pad += 0.4f * triangleWave(padPhase);
    pad *= 0.12f; // 10-15% mix
    
    // === Envelope ===
    float envelope = 1.0f;
    if (i < attackSamples) {
      envelope = float(i) / attackSamples;
    } else if (i < attackSamples + decaySamples) {
      float decayProgress = float(i - attackSamples) / decaySamples;
      envelope = 1.0f - 0.3f * decayProgress; // Decay to 70%
    } else if (i >= releaseStart && releaseStart > 0) {
      float releaseProgress = float(i - releaseStart) / releaseSamples;
      envelope = 0.7f * (1.0f - releaseProgress);
    } else {
      envelope = 0.7f; // Sustain level
    }
    
    float sample = (transient + body + pad) * envelope;
    data[i] = qint16(qBound(-1.0f, sample, 1.0f) * maxAmplitude);
  }
  
  return samples;
}

QByteArray AudioEngine::generateSoprano(int frequency, int durationMs) {
  // Soprano: warm flute + light choir "ah" vowel
  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  QByteArray samples;
  samples.resize(numSamples * sizeof(qint16));
  qint16 *data = reinterpret_cast<qint16 *>(samples.data());
  
  const float maxAmplitude = 32767.0f * 0.38f; // S: reduced for mixing headroom
  
  int attackSamples = sampleRate * 18 / 1000;   // 18ms attack
  int releaseSamples = sampleRate * 200 / 1000; // 200ms release
  int releaseStart = numSamples - releaseSamples;
  
  for (int i = 0; i < numSamples; i++) {
    float t = float(i) / sampleRate;
    float phase = 2.0f * M_PI * frequency * t;
    
    // Flute-like: mostly fundamental with slight breath noise
    float flute = sineWave(phase);
    flute += 0.15f * sineWave(phase * 2);
    flute += 0.05f * sineWave(phase * 3);
    
    // Choir "ah" vowel formant simulation
    float choir = 0.0f;
    choir += 0.8f * sineWave(phase);
    choir += 0.3f * sineWave(phase * 2);
    choir += 0.15f * sineWave(phase * 3);
    choir += 0.08f * triangleWave(phase * 4);
    
    // Blend flute and choir
    float sample = 0.6f * flute + 0.4f * choir;
    sample *= 0.5f; // Normalize
    
    // Envelope
    float envelope = 1.0f;
    if (i < attackSamples) {
      envelope = float(i) / attackSamples;
      envelope = envelope * envelope; // Smooth attack
    } else if (i >= releaseStart && releaseStart > 0) {
      float releaseProgress = float(i - releaseStart) / releaseSamples;
      envelope = 1.0f - releaseProgress;
    }
    
    data[i] = qint16(qBound(-1.0f, sample * envelope, 1.0f) * maxAmplitude);
  }
  
  return samples;
}

QByteArray AudioEngine::generateAlto(int frequency, int durationMs) {
  // Alto: warm string synth, rich midrange pad
  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  QByteArray samples;
  samples.resize(numSamples * sizeof(qint16));
  qint16 *data = reinterpret_cast<qint16 *>(samples.data());
  
  const float maxAmplitude = 32767.0f * 0.32f; // A: reduced for mixing headroom
  
  int attackSamples = sampleRate * 22 / 1000;   // 22ms attack
  int releaseSamples = sampleRate * 300 / 1000; // 300ms release
  int releaseStart = numSamples - releaseSamples;
  
  for (int i = 0; i < numSamples; i++) {
    float t = float(i) / sampleRate;
    float phase = 2.0f * M_PI * frequency * t;
    
    // String synth: sawtooth-ish with filtering
    float sample = 0.0f;
    sample += 1.0f * sineWave(phase);
    sample += 0.4f * sineWave(phase * 2);
    sample += 0.25f * sineWave(phase * 3);
    sample += 0.15f * triangleWave(phase * 4);
    sample += 0.08f * sineWave(phase * 5);
    
    // Add slight detuning for richness
    float detune = 1.003f;
    sample += 0.3f * sineWave(phase * detune);
    sample += 0.15f * sineWave(phase * 2 * detune);
    
    sample *= 0.35f; // Normalize
    
    // Envelope
    float envelope = 1.0f;
    if (i < attackSamples) {
      envelope = float(i) / attackSamples;
    } else if (i >= releaseStart && releaseStart > 0) {
      float releaseProgress = float(i - releaseStart) / releaseSamples;
      envelope = 1.0f - releaseProgress;
    }
    
    data[i] = qint16(qBound(-1.0f, sample * envelope, 1.0f) * maxAmplitude);
  }
  
  return samples;
}

QByteArray AudioEngine::generateTenor(int frequency, int durationMs) {
  // Tenor: soft cello-like synth, warm and rounded
  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  QByteArray samples;
  samples.resize(numSamples * sizeof(qint16));
  qint16 *data = reinterpret_cast<qint16 *>(samples.data());
  
  const float maxAmplitude = 32767.0f * 0.35f; // T: reduced for mixing headroom
  
  int attackSamples = sampleRate * 28 / 1000;   // 28ms attack
  int releaseSamples = sampleRate * 330 / 1000; // 330ms release
  int releaseStart = numSamples - releaseSamples;
  
  for (int i = 0; i < numSamples; i++) {
    float t = float(i) / sampleRate;
    float phase = 2.0f * M_PI * frequency * t;
    
    // Cello-like: rich harmonics, warm character
    float sample = 0.0f;
    sample += 1.0f * sineWave(phase);
    sample += 0.6f * sineWave(phase * 2);
    sample += 0.4f * sineWave(phase * 3);
    sample += 0.2f * sineWave(phase * 4);
    sample += 0.1f * triangleWave(phase * 5);
    
    // Slight saturation for presence (soft clipping)
    sample *= 0.4f;
    sample = sample * (1.0f + 0.02f * qAbs(sample)); // 2% saturation
    
    // Envelope with slower attack
    float envelope = 1.0f;
    if (i < attackSamples) {
      float attackProgress = float(i) / attackSamples;
      envelope = attackProgress * attackProgress; // Smooth bow attack
    } else if (i >= releaseStart && releaseStart > 0) {
      float releaseProgress = float(i - releaseStart) / releaseSamples;
      envelope = 1.0f - releaseProgress;
    }
    
    data[i] = qint16(qBound(-1.0f, sample * envelope, 1.0f) * maxAmplitude);
  }
  
  return samples;
}

QByteArray AudioEngine::generateBass(int frequency, int durationMs) {
  // Bass: upright bass pluck + pad, strong transient
  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  QByteArray samples;
  samples.resize(numSamples * sizeof(qint16));
  qint16 *data = reinterpret_cast<qint16 *>(samples.data());
  
  const float maxAmplitude = 32767.0f * 0.30f; // B: reduced for mixing headroom
  
  int attackSamples = sampleRate * 10 / 1000;   // 10ms attack (pluck)
  int releaseSamples = sampleRate * 200 / 1000; // 200ms release
  int releaseStart = numSamples - releaseSamples;
  
  for (int i = 0; i < numSamples; i++) {
    float t = float(i) / sampleRate;
    float phase = 2.0f * M_PI * frequency * t;
    
    // === Pluck transient ===
    float transient = 0.0f;
    if (i < attackSamples * 2) {
      float transientEnv = 1.0f - float(i) / (attackSamples * 2);
      transientEnv = transientEnv * transientEnv;
      transient = 0.5f * sineWave(phase * 3) * transientEnv;
      transient += 0.3f * triangleWave(phase * 2) * transientEnv;
    }
    
    // === Bass body (sine + sub) ===
    float body = 0.0f;
    body += 1.0f * sineWave(phase);           // Fundamental
    body += 0.4f * sineWave(phase * 2);       // 2nd harmonic
    body += 0.15f * triangleWave(phase * 3);  // Slight warmth
    
    // Sub bass (triangle for warmth)
    float sub = 0.3f * triangleWave(phase * 0.5f);
    
    // Combine
    float sample = transient + 0.5f * body + sub;
    sample *= 0.5f; // Normalize
    
    // Envelope
    float envelope = 1.0f;
    if (i < attackSamples) {
      envelope = float(i) / attackSamples;
    } else if (i >= releaseStart && releaseStart > 0) {
      float releaseProgress = float(i - releaseStart) / releaseSamples;
      envelope = 1.0f - releaseProgress;
    }
    
    data[i] = qint16(qBound(-1.0f, sample * envelope, 1.0f) * maxAmplitude);
  }
  
  return samples;
}

QByteArray AudioEngine::generateMetronomeClick(int durationMs) {
  // Muted woodblock-like tick
  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  QByteArray samples;
  samples.resize(numSamples * sizeof(qint16));
  qint16 *data = reinterpret_cast<qint16 *>(samples.data());
  
  const float maxAmplitude = 32767.0f * 0.6f;
  
  // Short, dry "tok" at 1-2 kHz peak
  for (int i = 0; i < numSamples; i++) {
    float t = float(i) / sampleRate;
    float decay = qExp(-t * 80); // Very fast decay
    
    // Two frequencies for woodblock character
    float sample = 0.7f * qSin(2.0f * M_PI * 1200.0f * t);
    sample += 0.3f * qSin(2.0f * M_PI * 1800.0f * t);
    sample *= decay;
    
    data[i] = qint16(sample * maxAmplitude);
  }
  
  return samples;
}

// ============================================================================
// Public API
// ============================================================================

void AudioEngine::playNote(int midiPitch, int durationMs, int voiceType) {
  // Gently stop previous note to avoid clicks
  if (m_audioSink) {
    m_audioSink->stop();
  }
  m_audioBuffer.close();

  QAudioDevice currentDefault = QMediaDevices::defaultAudioOutput();
  if (!currentDefault.isNull() && currentDefault.id() != m_currentDeviceId) {
    recreateAudioSink();
  }

  if (!m_audioSink) {
    qWarning() << "No audio sink available";
    return;
  }

  int frequency = midiToFrequency(midiPitch);
  QByteArray samples;
  
  VoiceType voice = static_cast<VoiceType>(voiceType);
  switch (voice) {
    case VoiceType::SketchPiano:
      samples = generateSketchPiano(frequency, durationMs);
      break;
    case VoiceType::Soprano:
      samples = generateSoprano(frequency, durationMs);
      break;
    case VoiceType::Alto:
      samples = generateAlto(frequency, durationMs);
      break;
    case VoiceType::Tenor:
      samples = generateTenor(frequency, durationMs);
      break;
    case VoiceType::Bass:
      samples = generateBass(frequency, durationMs);
      break;
    case VoiceType::Metronome:
      samples = generateMetronomeClick(durationMs);
      break;
    default:
      samples = generateSketchPiano(frequency, durationMs);
      break;
  }

  m_audioBuffer.setData(samples);
  m_audioBuffer.open(QIODevice::ReadOnly);

  m_audioSink->start(&m_audioBuffer);
  m_isPlaying = true;
  emit isPlayingChanged();

  QTimer::singleShot(durationMs + 50, this, [this]() {
    if (m_isPlaying) {
      m_isPlaying = false;
      emit isPlayingChanged();
      emit noteFinished();
    }
  });
}

void AudioEngine::playChord(const QVariantList &notes, int durationMs) {
  // Don't hard-stop previous audio - let it fade naturally
  if (m_audioSink) {
    m_audioSink->stop();
  }
  m_audioBuffer.close();

  QAudioDevice currentDefault = QMediaDevices::defaultAudioOutput();
  if (!currentDefault.isNull() && currentDefault.id() != m_currentDeviceId) {
    recreateAudioSink();
  }

  if (!m_audioSink || notes.isEmpty()) {
    return;
  }

  int sampleRate = m_format.sampleRate();
  int numSamples = (sampleRate * durationMs) / 1000;
  
  // Generate each voice and mix them together
  QVector<float> mixBuffer(numSamples, 0.0f);
  int voiceCount = 0;
  
  for (int idx = 0; idx < notes.size(); idx++) {
    QVariantMap noteMap = notes[idx].toMap();
    int midiPitch = noteMap["midiPitch"].toInt();
    int voiceType = noteMap["voiceType"].toInt();
    
    if (midiPitch <= 0) continue;
    
    int frequency = midiToFrequency(midiPitch);
    QByteArray voiceSamples;
    
    VoiceType voice = static_cast<VoiceType>(voiceType);
    switch (voice) {
      case VoiceType::Soprano:
        voiceSamples = generateSoprano(frequency, durationMs);
        break;
      case VoiceType::Alto:
        voiceSamples = generateAlto(frequency, durationMs);
        break;
      case VoiceType::Tenor:
        voiceSamples = generateTenor(frequency, durationMs);
        break;
      case VoiceType::Bass:
        voiceSamples = generateBass(frequency, durationMs);
        break;
      default:
        voiceSamples = generateSketchPiano(frequency, durationMs);
        break;
    }
    
    // Mix this voice into the buffer (already normalized to -1..1 range)
    const qint16 *voiceData = reinterpret_cast<const qint16*>(voiceSamples.constData());
    int voiceSampleCount = voiceSamples.size() / sizeof(qint16);
    
    for (int i = 0; i < qMin(numSamples, voiceSampleCount); i++) {
      mixBuffer[i] += voiceData[i] / 32767.0f;
    }
    voiceCount++;
  }
  
  if (voiceCount == 0) return;
  
  // Use a gentler normalization - divide by voice count for proper mixing
  // Also apply soft limiting to prevent harsh clipping
  float normFactor = 0.7f / float(voiceCount);  // Leave headroom
  
  // Convert back to int16 with soft limiting
  QByteArray mixedSamples;
  mixedSamples.resize(numSamples * sizeof(qint16));
  qint16 *data = reinterpret_cast<qint16*>(mixedSamples.data());
  
  for (int i = 0; i < numSamples; i++) {
    float sample = mixBuffer[i] * normFactor;
    // Soft clipping (tanh-style limiting)
    if (sample > 0.8f) {
      sample = 0.8f + 0.2f * std::tanh((sample - 0.8f) * 5.0f);
    } else if (sample < -0.8f) {
      sample = -0.8f + 0.2f * std::tanh((sample + 0.8f) * 5.0f);
    }
    data[i] = qint16(qBound(-1.0f, sample, 1.0f) * 32767.0f);
  }
  
  m_audioBuffer.setData(mixedSamples);
  m_audioBuffer.open(QIODevice::ReadOnly);

  m_audioSink->start(&m_audioBuffer);
  m_isPlaying = true;
  emit isPlayingChanged();

  QTimer::singleShot(durationMs + 50, this, [this]() {
    if (m_isPlaying) {
      m_isPlaying = false;
      emit isPlayingChanged();
      emit noteFinished();
    }
  });
}

void AudioEngine::playRenderedTimeline(const QVariantList &events, int totalDurationMs) {
  // Pre-render entire timeline into one continuous audio buffer
  if (m_audioSink) {
    m_audioSink->stop();
  }
  m_audioBuffer.close();

  QAudioDevice currentDefault = QMediaDevices::defaultAudioOutput();
  if (!currentDefault.isNull() && currentDefault.id() != m_currentDeviceId) {
    recreateAudioSink();
  }

  if (!m_audioSink || events.isEmpty()) {
    qWarning() << "AudioEngine::playRenderedTimeline - no audio sink or empty events";
    return;
  }

  int sampleRate = m_format.sampleRate();
  int totalSamples = (sampleRate * totalDurationMs) / 1000;
  
  qDebug() << "AudioEngine::playRenderedTimeline -" << events.size() << "events,"
           << "totalDurationMs=" << totalDurationMs << "sampleRate=" << sampleRate
           << "totalSamples=" << totalSamples;
  
  // Create a float mixing buffer for the entire duration
  QVector<float> mixBuffer(totalSamples, 0.0f);
  int maxVoicesAtOnce = 0;
  
  // Process each event and add its audio to the mix buffer
  for (int eventIdx = 0; eventIdx < events.size(); eventIdx++) {
    QVariantMap event = events[eventIdx].toMap();
    int startMs = event["startMs"].toInt();
    int durationMs = event["durationMs"].toInt();
    QVariantList notes = event["notes"].toList();
    
    qDebug() << "  Event" << eventIdx << ": startMs=" << startMs 
             << "durationMs=" << durationMs << "notes=" << notes.size();
    
    int startSample = (sampleRate * startMs) / 1000;
    int numSamples = (sampleRate * durationMs) / 1000;
    
    // Track max simultaneous voices for normalization
    if (notes.size() > maxVoicesAtOnce) {
      maxVoicesAtOnce = notes.size();
    }
    
    // Generate and mix each note in this event
    for (int noteIdx = 0; noteIdx < notes.size(); noteIdx++) {
      QVariantMap noteMap = notes[noteIdx].toMap();
      int midiPitch = noteMap["midiPitch"].toInt();
      int voiceType = noteMap["voiceType"].toInt();
      
      QByteArray voiceSamples;
      VoiceType voice = static_cast<VoiceType>(voiceType);
      
      // Handle metronome separately (doesn't need midiPitch)
      if (voice == VoiceType::Metronome) {
        voiceSamples = generateMetronomeClick(durationMs);
      } else {
        if (midiPitch <= 0) continue;
        
        int frequency = midiToFrequency(midiPitch);
        switch (voice) {
          case VoiceType::Soprano:
            voiceSamples = generateSoprano(frequency, durationMs);
            break;
          case VoiceType::Alto:
            voiceSamples = generateAlto(frequency, durationMs);
            break;
          case VoiceType::Tenor:
            voiceSamples = generateTenor(frequency, durationMs);
            break;
          case VoiceType::Bass:
            voiceSamples = generateBass(frequency, durationMs);
            break;
          default:
            voiceSamples = generateSketchPiano(frequency, durationMs);
            break;
        }
      }
      
      // Add this voice to the mix buffer at the correct position
      const qint16 *voiceData = reinterpret_cast<const qint16*>(voiceSamples.constData());
      int voiceSampleCount = voiceSamples.size() / sizeof(qint16);
      
      for (int i = 0; i < voiceSampleCount && (startSample + i) < totalSamples; i++) {
        mixBuffer[startSample + i] += voiceData[i] / 32767.0f;
      }
    }
  }
  
  if (maxVoicesAtOnce == 0) maxVoicesAtOnce = 1;
  
  // Normalize based on max simultaneous voices
  float normFactor = 0.6f / float(maxVoicesAtOnce);
  
  // Convert to int16 with soft limiting
  QByteArray renderedAudio;
  renderedAudio.resize(totalSamples * sizeof(qint16));
  qint16 *data = reinterpret_cast<qint16*>(renderedAudio.data());
  
  for (int i = 0; i < totalSamples; i++) {
    float sample = mixBuffer[i] * normFactor;
    // Soft clipping
    if (sample > 0.8f) {
      sample = 0.8f + 0.2f * std::tanh((sample - 0.8f) * 5.0f);
    } else if (sample < -0.8f) {
      sample = -0.8f + 0.2f * std::tanh((sample + 0.8f) * 5.0f);
    }
    data[i] = qint16(qBound(-1.0f, sample, 1.0f) * 32767.0f);
  }
  
  m_audioBuffer.setData(renderedAudio);
  m_audioBuffer.open(QIODevice::ReadOnly);

  m_audioSink->start(&m_audioBuffer);
  m_isPlaying = true;
  emit isPlayingChanged();

  QTimer::singleShot(totalDurationMs + 100, this, [this]() {
    if (m_isPlaying) {
      m_isPlaying = false;
      emit isPlayingChanged();
      emit noteFinished();
    }
  });
}

void AudioEngine::playClick() {
  stop();

  QAudioDevice currentDefault = QMediaDevices::defaultAudioOutput();
  if (!currentDefault.isNull() && currentDefault.id() != m_currentDeviceId) {
    recreateAudioSink();
  }

  if (!m_audioSink) {
    return;
  }

  QByteArray samples = generateMetronomeClick(30);

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

int AudioEngine::midiToFrequency(int midiPitch) {
  return qRound(440.0 * qPow(2.0, (midiPitch - 69) / 12.0));
}

int AudioEngine::keyToRootMidi(const QString &key) {
  QString keyLower = key.toLower();

  if (keyLower.contains("c")) return 60;
  if (keyLower.contains("d")) return 62;
  if (keyLower.contains("e")) return 64;
  if (keyLower.contains("f")) return 65;
  if (keyLower.contains("g")) return 67;
  if (keyLower.contains("a")) return 69;
  if (keyLower.contains("b")) return 71;

  return 60;
}

int AudioEngine::scaleDegreeToMidi(int degree, const QString &key, int octaveOffset) {
  static const int majorIntervals[] = {0, 2, 4, 5, 7, 9, 11};
  static const int minorIntervals[] = {0, 2, 3, 5, 7, 8, 10};

  int rootMidi = keyToRootMidi(key);
  bool isMinor = key.toLower().contains("minor");

  int index = qBound(0, degree - 1, 6);
  int interval = isMinor ? minorIntervals[index] : majorIntervals[index];

  return rootMidi + interval + (octaveOffset * 12);
}
