#include "Section.h"
#include "Motif.h"
#include "Sketch.h"
#include <QJsonArray>
#include <QUuid>
#include <algorithm>

// VoiceData serialization
QJsonObject VoiceData::toJson() const {
  QJsonObject json;
  QJsonArray pitchArray;
  for (int p : pitches) {
    pitchArray.append(p);
  }
  json["pitches"] = pitchArray;
  json["rhythm"] = rhythm.toJson(); // toJson returns QJsonArray
  return json;
}

VoiceData VoiceData::fromJson(const QJsonObject &json) {
  VoiceData voice;
  QJsonArray pitchArray = json["pitches"].toArray();
  for (const auto &p : pitchArray) {
    voice.pitches.append(p.toInt());
  }
  // RhythmGrid::fromJson expects QJsonArray
  voice.rhythm = RhythmGrid::fromJson(json["rhythm"].toArray());
  return voice;
}

Section::Section(const QString &id, const QString &name, int lengthBars)
    : m_id(id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                        : id),
      m_name(name), m_lengthBars(lengthBars) {}

QString Section::textureTypeString() const {
  switch (m_textureType) {
    case TextureType::MelodyOnly: return "MelodyOnly";
    case TextureType::SATBChorale: return "SATBChorale";
    default: return "MelodyOnly";
  }
}

bool Section::hasSATBVoices() const {
  return !m_soprano.isEmpty() && !m_alto.isEmpty() && 
         !m_tenor.isEmpty() && !m_bass.isEmpty();
}

void Section::addPlacement(const MotifPlacement &placement) {
  m_placements.append(placement);
}

void Section::removePlacement(int index) {
  if (index >= 0 && index < m_placements.size()) {
    m_placements.removeAt(index);
  }
}

void Section::removePlacementByMotifId(const QString &motifId) {
  m_placements.erase(std::remove_if(m_placements.begin(), m_placements.end(),
                                    [&motifId](const MotifPlacement &p) {
                                      return p.motifId == motifId;
                                    }),
                     m_placements.end());
}

void Section::updatePlacement(int index, const MotifPlacement &placement) {
  if (index >= 0 && index < m_placements.size()) {
    m_placements[index] = placement;
  }
}

MotifPlacement* Section::findPlacement(int index) {
  if (index >= 0 && index < m_placements.size()) {
    return &m_placements[index];
  }
  return nullptr;
}

void Section::movePlacement(int index, int newStartBar) {
  if (index >= 0 && index < m_placements.size()) {
    m_placements[index].startBar = newStartBar;
  }
}

void Section::setPlacementRepetitions(int index, int repetitions) {
  if (index >= 0 && index < m_placements.size() && repetitions > 0) {
    m_placements[index].repetitions = repetitions;
  }
}

bool Section::isBarRangeAvailable(int startBar, int endBar, int excludeIndex) const {
  // For now, we allow overlapping - the flatten logic will handle it
  // by layering. Return true to always allow placement.
  // In a stricter mode, you could check for overlaps here.
  Q_UNUSED(startBar)
  Q_UNUSED(endBar)
  Q_UNUSED(excludeIndex)
  return true;
}

QList<MotifPlacement> Section::sortedPlacements() const {
  QList<MotifPlacement> sorted = m_placements;
  std::sort(sorted.begin(), sorted.end(),
            [](const MotifPlacement &a, const MotifPlacement &b) {
              return a.startBar < b.startBar;
            });
  return sorted;
}

// Helper to convert duration string to beats
static double durationToBeats(const QString &duration) {
  if (duration == "whole") return 4.0;
  if (duration == "dotted-half") return 3.0;
  if (duration == "half") return 2.0;
  if (duration == "dotted-quarter") return 1.5;
  if (duration == "quarter") return 1.0;
  if (duration == "dotted-eighth") return 0.75;
  if (duration == "eighth") return 0.5;
  if (duration == "sixteenth") return 0.25;
  return 1.0; // Default to quarter
}

QList<NoteEvent> Section::flattenToTimeline(const Sketch &sketch) const {
  QList<NoteEvent> timeline;
  
  // Get time signature info (assuming 4/4 for now)
  int beatsPerBar = 4;
  QString timeSig = sketch.timeSignature();
  if (timeSig == "3/4") beatsPerBar = 3;
  else if (timeSig == "6/8") beatsPerBar = 6;
  else if (timeSig == "2/4") beatsPerBar = 2;
  
  // Cache motifs list to avoid pointer-to-temporary issues
  QList<Motif> motifsList = sketch.motifs();
  
  // Process each placement
  for (const MotifPlacement &placement : sortedPlacements()) {
    // Find the motif in the sketch
    const Motif *motif = nullptr;
    for (int i = 0; i < motifsList.size(); ++i) {
      if (motifsList[i].id() == placement.motifId) {
        motif = &motifsList[i];
        break;
      }
    }
    
    if (!motif) continue;
    
    // Get motif data
    QList<int> pitches = motif->pitchContour();
    const RhythmGrid &rhythm = motif->rhythmGrid();
    
    // Calculate the starting beat for this placement
    double placementStartBeat = placement.startBar * beatsPerBar;
    
    // Repeat the motif as many times as specified
    for (int rep = 0; rep < placement.repetitions; rep++) {
      double currentBeat = placementStartBeat + (rep * motif->lengthBars() * beatsPerBar);
      int pitchIndex = 0;
      
      // If no rhythm cells, create default quarter notes
      if (rhythm.isEmpty()) {
        for (int pitch : pitches) {
          NoteEvent event;
          event.scaleDegree = pitch;
          event.duration = "quarter";
          event.startBeat = currentBeat;
          event.isRest = false;
          event.tie = false;
          timeline.append(event);
          currentBeat += 1.0;
        }
      } else {
        // Process each rhythm cell
        for (int i = 0; i < rhythm.cellCount(); i++) {
          const RhythmCell &cell = rhythm.cell(i);
          
          NoteEvent event;
          event.duration = cell.duration;
          event.startBeat = currentBeat;
          event.isRest = cell.isRest;
          event.tie = cell.tie;
          
          if (cell.isRest) {
            event.scaleDegree = 0; // Rests have no pitch
          } else if (pitchIndex < pitches.size()) {
            event.scaleDegree = pitches[pitchIndex];
            pitchIndex++;
          } else {
            event.scaleDegree = 1; // Fallback
          }
          
          timeline.append(event);
          currentBeat += durationToBeats(cell.duration);
        }
      }
    }
  }
  
  // Sort by start beat
  std::sort(timeline.begin(), timeline.end(),
            [](const NoteEvent &a, const NoteEvent &b) {
              return a.startBeat < b.startBeat;
            });
  
  return timeline;
}

QList<NoteEvent> Section::flattenVoiceToTimeline(int voiceIndex) const {
  QList<NoteEvent> timeline;
  
  const VoiceData *voice = nullptr;
  switch (voiceIndex) {
    case 0: voice = &m_soprano; break;
    case 1: voice = &m_alto; break;
    case 2: voice = &m_tenor; break;
    case 3: voice = &m_bass; break;
    default: return timeline;
  }
  
  if (voice->isEmpty()) return timeline;
  
  double currentBeat = 0.0;
  int pitchIndex = 0;
  
  if (voice->rhythm.isEmpty()) {
    // Default to quarter notes
    for (int pitch : voice->pitches) {
      NoteEvent event;
      event.scaleDegree = pitch;
      event.duration = "quarter";
      event.startBeat = currentBeat;
      event.isRest = false;
      event.tie = false;
      timeline.append(event);
      currentBeat += 1.0;
    }
  } else {
    for (int i = 0; i < voice->rhythm.cellCount(); i++) {
      const RhythmCell &cell = voice->rhythm.cell(i);
      
      NoteEvent event;
      event.duration = cell.duration;
      event.startBeat = currentBeat;
      event.isRest = cell.isRest;
      event.tie = cell.tie;
      
      if (cell.isRest) {
        event.scaleDegree = 0;
      } else if (pitchIndex < voice->pitches.size()) {
        event.scaleDegree = voice->pitches[pitchIndex];
        pitchIndex++;
      } else {
        event.scaleDegree = 1;
      }
      
      timeline.append(event);
      currentBeat += durationToBeats(cell.duration);
    }
  }
  
  return timeline;
}

QJsonObject MotifPlacement::toJson() const {
  QJsonObject json;
  json["motifId"] = motifId;
  json["startBar"] = startBar;
  json["repetitions"] = repetitions;
  json["voice"] = voice;
  return json;
}

MotifPlacement MotifPlacement::fromJson(const QJsonObject &json) {
  MotifPlacement placement;
  placement.motifId = json["motifId"].toString();
  placement.startBar = json["startBar"].toInt();
  placement.repetitions = json["repetitions"].toInt(1);
  placement.voice = json["voice"].toInt();
  return placement;
}

QJsonObject Section::toJson() const {
  QJsonObject json;
  json["id"] = m_id;
  json["name"] = m_name;
  json["lengthBars"] = m_lengthBars;
  json["textureType"] = static_cast<int>(m_textureType);

  QJsonArray placementsArray;
  for (const auto &placement : m_placements) {
    placementsArray.append(placement.toJson());
  }
  json["placements"] = placementsArray;
  
  // Save SATB voice data if present
  if (m_textureType == TextureType::SATBChorale) {
    json["soprano"] = m_soprano.toJson();
    json["alto"] = m_alto.toJson();
    json["tenor"] = m_tenor.toJson();
    json["bass"] = m_bass.toJson();
  }

  return json;
}

Section Section::fromJson(const QJsonObject &json) {
  Section section(json["id"].toString(), json["name"].toString(),
                  json["lengthBars"].toInt(8));

  section.m_textureType = static_cast<TextureType>(json["textureType"].toInt(0));
  
  QJsonArray placementsArray = json["placements"].toArray();
  for (const auto &placementValue : placementsArray) {
    section.addPlacement(MotifPlacement::fromJson(placementValue.toObject()));
  }
  
  // Load SATB voice data if present
  if (json.contains("soprano")) {
    section.m_soprano = VoiceData::fromJson(json["soprano"].toObject());
  }
  if (json.contains("alto")) {
    section.m_alto = VoiceData::fromJson(json["alto"].toObject());
  }
  if (json.contains("tenor")) {
    section.m_tenor = VoiceData::fromJson(json["tenor"].toObject());
  }
  if (json.contains("bass")) {
    section.m_bass = VoiceData::fromJson(json["bass"].toObject());
  }

  return section;
}
