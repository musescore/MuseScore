#pragma once

#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <QObject>
#include <QTimer>
#include <memory>

// Simple audio engine for playing tones
class AudioEngine : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
  Q_PROPERTY(int tempo READ tempo WRITE setTempo NOTIFY tempoChanged)

public:
  explicit AudioEngine(QObject *parent = nullptr);
  ~AudioEngine();

  // Play a single note
  Q_INVOKABLE void playNote(int midiPitch, int durationMs);

  // Play a click sound (for metronome)
  Q_INVOKABLE void playClick();

  // Stop all sound
  Q_INVOKABLE void stop();

  // Convert scale degree to MIDI pitch
  Q_INVOKABLE static int scaleDegreeToMidi(int degree, const QString &key);

  bool isPlaying() const { return m_isPlaying; }
  int tempo() const { return m_tempo; }
  void setTempo(int bpm);

signals:
  void isPlayingChanged();
  void tempoChanged();
  void noteFinished();

private:
  void initAudio();
  QByteArray generateSineWave(int frequency, int durationMs, float amplitude);
  QByteArray generateClickSound(int durationMs);
  int midiToFrequency(int midiPitch);

  std::unique_ptr<QAudioSink> m_audioSink;
  QBuffer m_audioBuffer;
  QAudioFormat m_format;
  bool m_isPlaying = false;
  int m_tempo = 120; // BPM

  // Key signatures: maps key name to root MIDI note
  static int keyToRootMidi(const QString &key);
};

