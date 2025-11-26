#pragma once

#include "AudioEngine.h"
#include "RhythmGrid.h"
#include <QObject>
#include <QTimer>
#include <QVariantList>

// Plays a motif with correct timing
class MotifPlayer : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
  Q_PROPERTY(bool isLooping READ isLooping WRITE setLooping NOTIFY loopingChanged)
  Q_PROPERTY(int tempo READ tempo WRITE setTempo NOTIFY tempoChanged)
  Q_PROPERTY(bool metronomeEnabled READ metronomeEnabled WRITE setMetronomeEnabled 
             NOTIFY metronomeEnabledChanged)
  Q_PROPERTY(int currentNoteIndex READ currentNoteIndex NOTIFY currentNoteIndexChanged)
  Q_PROPERTY(int voiceType READ voiceType WRITE setVoiceType NOTIFY voiceTypeChanged)

public:
  explicit MotifPlayer(QObject *parent = nullptr);

  // Set the motif data to play
  Q_INVOKABLE void setMotif(const QVariantList &pitchContour,
                            const QVariantList &rhythmCells,
                            const QString &key);

  // Playback controls
  Q_INVOKABLE void play();           // Note-by-note playback (legacy)
  Q_INVOKABLE void playPrerendered(); // Pre-render entire motif for smooth playback
  Q_INVOKABLE void stop();
  Q_INVOKABLE void pause();

  // Play a single note immediately (for pad taps)
  Q_INVOKABLE void playPreviewNote(int scaleDegree, const QString &key);
  
  // Play a note with a specific voice type (0=Piano, 1=S, 2=A, 3=T, 4=B)
  Q_INVOKABLE void playPreviewNoteWithVoice(int scaleDegree, const QString &key, int voiceType);
  
  // Play a SATB chord (all 4 voices simultaneously)
  // notes: array of { scaleDegree, voiceType, octaveOffset }
  Q_INVOKABLE void playSATBChord(const QVariantList &notes, const QString &key, int durationMs);
  
  // Pre-render and play entire SATB timeline
  // events: array of { startBeat, duration, notes: [{scaleDegree, voiceType, octaveOffset}] }
  Q_INVOKABLE void playSATBTimeline(const QVariantList &events, const QString &key);

  bool isPlaying() const { return m_isPlaying; }
  bool isLooping() const { return m_isLooping; }
  int tempo() const { return m_tempo; }
  bool metronomeEnabled() const { return m_metronomeEnabled; }
  int currentNoteIndex() const { return m_currentNoteIndex; }
  int voiceType() const { return m_voiceType; }

  void setLooping(bool loop);
  void setTempo(int bpm);
  void setMetronomeEnabled(bool enabled);
  void setVoiceType(int type);

signals:
  void isPlayingChanged();
  void loopingChanged();
  void tempoChanged();
  void metronomeEnabledChanged();
  void currentNoteIndexChanged();
  void voiceTypeChanged();
  void playbackFinished();
  void beatTick(int beatNumber);

private slots:
  void playNextNote();
  void onMetronomeTick();

private:
  int durationToMs(const QString &duration);
  void scheduleNextNote();
  void startMetronome();
  void stopMetronome();

  AudioEngine m_audioEngine;
  QTimer m_playbackTimer;
  QTimer m_metronomeTimer;
  QTimer m_prerenderedEndTimer;  // Timer for prerendered playback end

  // Current motif data
  QList<int> m_pitchContour;
  QList<RhythmCell> m_rhythmCells;
  QString m_key;

  // Playback state
  bool m_isPlaying = false;
  bool m_isPaused = false;
  bool m_isLooping = false;
  bool m_metronomeEnabled = true;
  int m_tempo = 120;
  int m_currentNoteIndex = 0;
  int m_currentPitchIndex = 0;
  int m_currentBeat = 0;
  int m_voiceType = 0; // 0=SketchPiano (default)
};

