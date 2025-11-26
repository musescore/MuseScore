#pragma once

#include "RhythmGrid.h"
#include <QJsonObject>
#include <QList>
#include <QString>

class Motif;
class Sketch;

// Texture types for section rendering
enum class TextureType {
  MelodyOnly = 0,
  SATBChorale = 1
};

// Represents a single note event in the flattened timeline
struct NoteEvent {
  int scaleDegree;    // 1-7
  QString duration;   // "quarter", "eighth", etc.
  double startBeat;   // Beat position in the section
  bool isRest;
  bool tie;
};

// Voice data for SATB texture
struct VoiceData {
  QList<int> pitches;    // Scale degrees
  RhythmGrid rhythm;     // Rhythm pattern
  
  bool isEmpty() const { return pitches.isEmpty(); }
  
  QJsonObject toJson() const;
  static VoiceData fromJson(const QJsonObject &json);
};

struct MotifPlacement {
  QString motifId;
  int startBar;
  int repetitions = 1;  // Number of times to repeat the motif
  int voice; // 0=soprano, 1=alto, 2=tenor, 3=bass

  // Calculate the end bar based on motif length and repetitions
  int endBar(int motifLengthBars) const { return startBar + (motifLengthBars * repetitions); }

  QJsonObject toJson() const;
  static MotifPlacement fromJson(const QJsonObject &json);
};

class Section {
public:
  Section() = default;
  Section(const QString &id, const QString &name, int lengthBars);

  QString id() const { return m_id; }
  QString name() const { return m_name; }
  int lengthBars() const { return m_lengthBars; }
  QList<MotifPlacement> placements() const { return m_placements; }
  
  // Texture type
  TextureType textureType() const { return m_textureType; }
  void setTextureType(TextureType type) { m_textureType = type; }
  QString textureTypeString() const;
  
  // SATB voice data (populated when texture is SATBChorale)
  VoiceData sopranoVoice() const { return m_soprano; }
  VoiceData altoVoice() const { return m_alto; }
  VoiceData tenorVoice() const { return m_tenor; }
  VoiceData bassVoice() const { return m_bass; }
  
  void setSopranoVoice(const VoiceData &voice) { m_soprano = voice; }
  void setAltoVoice(const VoiceData &voice) { m_alto = voice; }
  void setTenorVoice(const VoiceData &voice) { m_tenor = voice; }
  void setBassVoice(const VoiceData &voice) { m_bass = voice; }
  
  // Check if SATB voices are populated
  bool hasSATBVoices() const;

  void setName(const QString &name) { m_name = name; }
  void setLengthBars(int length) { m_lengthBars = length; }
  
  // Placement management
  void addPlacement(const MotifPlacement &placement);
  void removePlacement(int index);
  void removePlacementByMotifId(const QString &motifId);
  void updatePlacement(int index, const MotifPlacement &placement);
  MotifPlacement* findPlacement(int index);
  
  // Move placement to a new start bar
  void movePlacement(int index, int newStartBar);
  
  // Set repetitions for a placement
  void setPlacementRepetitions(int index, int repetitions);
  
  // Check if a bar range is available (no overlapping placements)
  bool isBarRangeAvailable(int startBar, int endBar, int excludeIndex = -1) const;
  
  // Flatten all placements into a single timeline of note events
  // Uses the sketch to look up motif data
  QList<NoteEvent> flattenToTimeline(const Sketch &sketch) const;
  
  // Flatten a specific voice to timeline (for SATB)
  QList<NoteEvent> flattenVoiceToTimeline(int voiceIndex) const;
  
  // Get placements sorted by start bar
  QList<MotifPlacement> sortedPlacements() const;

  QJsonObject toJson() const;
  static Section fromJson(const QJsonObject &json);

private:
  QString m_id;
  QString m_name;
  int m_lengthBars = 8;
  QList<MotifPlacement> m_placements;
  
  // Texture settings
  TextureType m_textureType = TextureType::MelodyOnly;
  
  // SATB voice data
  VoiceData m_soprano;
  VoiceData m_alto;
  VoiceData m_tenor;
  VoiceData m_bass;
};
