#include "ExportEngine.h"
#include "SketchRepository.h"
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QXmlStreamWriter>
#include <cmath>

ExportEngine::ExportEngine(QObject *parent)
    : QObject(parent)
{
}

QString ExportEngine::fileExtension(int format) const
{
    switch (static_cast<Format>(format)) {
    case Format::MIDI:
        return "mid";
    case Format::MusicXML:
        return "musicxml";
    case Format::MSCZ:
        return "mscz";
    default:
        return "mid";
    }
}

QString ExportEngine::suggestedFilename(const QString &sketchName, int format) const
{
    QString sanitized = sketchName;
    sanitized.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
    return sanitized + "." + fileExtension(format);
}

bool ExportEngine::exportToMIDI(const QString &sketchId, const QUrl &filePath,
                                const QVariantList &sectionIds, const QVariantList &motifIds)
{
    emit exportStarted("MIDI");

    Sketch sketch = m_repository.loadSketch(sketchId, true);
    if (sketch.id().isEmpty()) {
        emit exportFailed("Could not load sketch");
        return false;
    }

    QString localPath = filePath.toLocalFile();
    if (localPath.isEmpty()) {
        localPath = filePath.toString();
    }

    // Convert QVariantList to QStringList
    QStringList secIds, motIds;
    for (const QVariant &v : sectionIds) {
        secIds.append(v.toString());
    }
    for (const QVariant &v : motifIds) {
        motIds.append(v.toString());
    }

    bool success = writeMIDIFile(sketch, localPath, secIds, motIds);
    
    if (success) {
        emit exportCompleted(localPath);
    } else {
        emit exportFailed("Failed to write MIDI file");
    }
    
    return success;
}

bool ExportEngine::exportToMusicXML(const QString &sketchId, const QUrl &filePath,
                                     const QVariantList &sectionIds, const QVariantList &motifIds)
{
    emit exportStarted("MusicXML");

    Sketch sketch = m_repository.loadSketch(sketchId, true);
    if (sketch.id().isEmpty()) {
        emit exportFailed("Could not load sketch");
        return false;
    }

    QString localPath = filePath.toLocalFile();
    if (localPath.isEmpty()) {
        localPath = filePath.toString();
    }

    // Convert QVariantList to QStringList
    QStringList secIds, motIds;
    for (const QVariant &v : sectionIds) {
        secIds.append(v.toString());
    }
    for (const QVariant &v : motifIds) {
        motIds.append(v.toString());
    }

    bool success = writeMusicXMLFile(sketch, localPath, secIds, motIds);
    
    if (success) {
        emit exportCompleted(localPath);
    } else {
        emit exportFailed("Failed to write MusicXML file");
    }
    
    return success;
}

bool ExportEngine::exportToMSCZ(const QString &sketchId, const QUrl &filePath)
{
    emit exportStarted("MuseScore");

    Sketch sketch = m_repository.loadSketch(sketchId, true);
    if (sketch.id().isEmpty()) {
        emit exportFailed("Could not load sketch");
        return false;
    }

    QString localPath = filePath.toLocalFile();
    if (localPath.isEmpty()) {
        localPath = filePath.toString();
    }

    bool success = writeMSCZFile(sketch, localPath);
    
    if (success) {
        emit exportCompleted(localPath);
    } else {
        emit exportFailed("Failed to write MSCZ file");
    }
    
    return success;
}

// Convert scale degree (1-7) to MIDI note number
int ExportEngine::scaleDegreeToMIDI(int scaleDegree, const QString &key, int octave) const
{
    // Major scale intervals from root: 0, 2, 4, 5, 7, 9, 11
    static const int majorIntervals[] = {0, 2, 4, 5, 7, 9, 11};
    // Minor scale intervals: 0, 2, 3, 5, 7, 8, 10
    static const int minorIntervals[] = {0, 2, 3, 5, 7, 8, 10};

    // Determine root note and mode
    int rootMidi = 60; // Default C4
    bool isMinor = key.contains("minor", Qt::CaseInsensitive);
    
    QString rootNote = key.split(" ").first().toUpper();
    
    // Map note names to MIDI (C4 = 60)
    if (rootNote == "C") rootMidi = 60;
    else if (rootNote == "C#" || rootNote == "DB") rootMidi = 61;
    else if (rootNote == "D") rootMidi = 62;
    else if (rootNote == "D#" || rootNote == "EB") rootMidi = 63;
    else if (rootNote == "E") rootMidi = 64;
    else if (rootNote == "F") rootMidi = 65;
    else if (rootNote == "F#" || rootNote == "GB") rootMidi = 66;
    else if (rootNote == "G") rootMidi = 67;
    else if (rootNote == "G#" || rootNote == "AB") rootMidi = 68;
    else if (rootNote == "A") rootMidi = 69;
    else if (rootNote == "A#" || rootNote == "BB") rootMidi = 70;
    else if (rootNote == "B") rootMidi = 71;

    // Adjust for octave (default is octave 4)
    rootMidi = rootMidi - 60 + (octave * 12);

    // Get interval for scale degree (1-indexed)
    int degreeIndex = (scaleDegree - 1) % 7;
    int interval = isMinor ? minorIntervals[degreeIndex] : majorIntervals[degreeIndex];
    
    // Handle octave shifts for high scale degrees
    int octaveShift = (scaleDegree - 1) / 7;

    return rootMidi + interval + (octaveShift * 12);
}

// Convert duration string to MIDI ticks
int ExportEngine::durationToTicks(const QString &duration, int ticksPerQuarter) const
{
    if (duration == "whole") return ticksPerQuarter * 4;
    if (duration == "dotted-half") return ticksPerQuarter * 3;
    if (duration == "half") return ticksPerQuarter * 2;
    if (duration == "dotted-quarter") return static_cast<int>(ticksPerQuarter * 1.5);
    if (duration == "quarter") return ticksPerQuarter;
    if (duration == "dotted-eighth") return static_cast<int>(ticksPerQuarter * 0.75);
    if (duration == "eighth") return ticksPerQuarter / 2;
    if (duration == "sixteenth") return ticksPerQuarter / 4;
    return ticksPerQuarter; // Default to quarter
}

void ExportEngine::writeVariableLengthQuantity(QByteArray &data, int value)
{
    QByteArray vlq;
    vlq.prepend(static_cast<char>(value & 0x7F));
    value >>= 7;
    while (value > 0) {
        vlq.prepend(static_cast<char>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    data.append(vlq);
}

void ExportEngine::writeMIDIHeader(QDataStream &stream, int numTracks, int ticksPerQuarter)
{
    // MThd chunk
    stream.writeRawData("MThd", 4);
    stream << static_cast<qint32>(6);           // Header length
    stream << static_cast<qint16>(1);           // Format type 1 (multi-track)
    stream << static_cast<qint16>(numTracks);   // Number of tracks
    stream << static_cast<qint16>(ticksPerQuarter); // Ticks per quarter note
}

void ExportEngine::writeMIDITrack(QDataStream &stream, const QByteArray &trackData)
{
    // MTrk chunk
    stream.writeRawData("MTrk", 4);
    stream << static_cast<qint32>(trackData.size());
    stream.writeRawData(trackData.constData(), trackData.size());
}

bool ExportEngine::writeMIDIFile(const Sketch &sketch, const QString &filePath,
                                  const QStringList &sectionIds, const QStringList &motifIds)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file for writing:" << filePath;
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::BigEndian);

    const int ticksPerQuarter = 480;
    
    // Filter sections and motifs based on provided IDs
    QList<Section> sectionsToExport;
    QList<Motif> motifsToExport;
    
    // If sectionIds is not empty, filter sections
    if (!sectionIds.isEmpty()) {
        for (const Section &section : sketch.sections()) {
            if (sectionIds.contains(section.id())) {
                sectionsToExport.append(section);
            }
        }
    }
    
    // If motifIds is not empty, filter motifs
    if (!motifIds.isEmpty()) {
        for (const Motif &motif : sketch.motifs()) {
            if (motifIds.contains(motif.id())) {
                motifsToExport.append(motif);
            }
        }
    }
    
    // Count tracks: 1 tempo track + 1 track per section (or 4 for SATB) + 1 per motif
    int numTracks = 1; // Tempo track
    for (const Section &section : sectionsToExport) {
        if (section.textureType() == TextureType::SATBChorale && section.hasSATBVoices()) {
            numTracks += 4; // SATB voices
        } else {
            numTracks += 1; // Melody only
        }
    }
    
    // Add motif tracks
    numTracks += motifsToExport.size();

    writeMIDIHeader(stream, numTracks, ticksPerQuarter);

    // Track 0: Tempo and time signature
    {
        QByteArray trackData;
        
        // Tempo meta event (delta=0)
        int microsecondsPerBeat = 60000000 / sketch.tempo();
        trackData.append('\x00'); // Delta time
        trackData.append('\xFF'); // Meta event
        trackData.append('\x51'); // Tempo
        trackData.append('\x03'); // Length
        trackData.append(static_cast<char>((microsecondsPerBeat >> 16) & 0xFF));
        trackData.append(static_cast<char>((microsecondsPerBeat >> 8) & 0xFF));
        trackData.append(static_cast<char>(microsecondsPerBeat & 0xFF));

        // Time signature meta event
        QString timeSig = sketch.timeSignature();
        int numerator = timeSig.split("/").first().toInt();
        int denominator = timeSig.split("/").last().toInt();
        int denomLog2 = static_cast<int>(std::log2(denominator));
        
        trackData.append('\x00'); // Delta time
        trackData.append('\xFF'); // Meta event
        trackData.append('\x58'); // Time signature
        trackData.append('\x04'); // Length
        trackData.append(static_cast<char>(numerator));
        trackData.append(static_cast<char>(denomLog2));
        trackData.append('\x18'); // Clocks per metronome click (24)
        trackData.append('\x08'); // 32nd notes per quarter (8)

        // End of track
        trackData.append('\x00');
        trackData.append('\xFF');
        trackData.append('\x2F');
        trackData.append('\x00');

        writeMIDITrack(stream, trackData);
    }

    emit exportProgress(20);

    int totalItems = sectionsToExport.size() + motifsToExport.size();
    int itemIndex = 0;

    // Export selected sections
    for (const Section &section : sectionsToExport) {
        if (section.textureType() == TextureType::SATBChorale && section.hasSATBVoices()) {
            // Export 4 SATB tracks
            for (int voice = 0; voice < 4; voice++) {
                QByteArray trackData;
                QList<NoteEvent> events = section.flattenVoiceToTimeline(voice);
                
                int currentTick = 0;
                int channel = voice; // Channels 0-3 for SATB
                
                for (const NoteEvent &event : events) {
                    if (event.isRest) continue;
                    
                    int eventTick = static_cast<int>(event.startBeat * ticksPerQuarter);
                    int deltaTicks = eventTick - currentTick;
                    int midiNote = scaleDegreeToMIDI(event.scaleDegree, sketch.key(), 4 - voice);
                    int duration = durationToTicks(event.duration, ticksPerQuarter);
                    
                    // Note on
                    writeVariableLengthQuantity(trackData, deltaTicks);
                    trackData.append(static_cast<char>(0x90 | channel)); // Note on
                    trackData.append(static_cast<char>(midiNote));
                    trackData.append('\x64'); // Velocity 100
                    
                    // Note off
                    writeVariableLengthQuantity(trackData, duration);
                    trackData.append(static_cast<char>(0x80 | channel)); // Note off
                    trackData.append(static_cast<char>(midiNote));
                    trackData.append('\x00'); // Velocity 0
                    
                    currentTick = eventTick + duration;
                }
                
                // End of track
                trackData.append('\x00');
                trackData.append('\xFF');
                trackData.append('\x2F');
                trackData.append('\x00');
                
                writeMIDITrack(stream, trackData);
            }
        } else {
            // Export melody track
            QByteArray trackData;
            QList<NoteEvent> events = section.flattenToTimeline(sketch);
            
            int currentTick = 0;
            int channel = 0;
            
            for (const NoteEvent &event : events) {
                if (event.isRest) continue;
                
                int eventTick = static_cast<int>(event.startBeat * ticksPerQuarter);
                int deltaTicks = eventTick - currentTick;
                int midiNote = scaleDegreeToMIDI(event.scaleDegree, sketch.key());
                int duration = durationToTicks(event.duration, ticksPerQuarter);
                
                // Note on
                writeVariableLengthQuantity(trackData, deltaTicks);
                trackData.append(static_cast<char>(0x90 | channel));
                trackData.append(static_cast<char>(midiNote));
                trackData.append('\x64');
                
                // Note off
                writeVariableLengthQuantity(trackData, duration);
                trackData.append(static_cast<char>(0x80 | channel));
                trackData.append(static_cast<char>(midiNote));
                trackData.append('\x00');
                
                currentTick = eventTick + duration;
            }
            
            // End of track
            trackData.append('\x00');
            trackData.append('\xFF');
            trackData.append('\x2F');
            trackData.append('\x00');
            
            writeMIDITrack(stream, trackData);
        }
        
        itemIndex++;
        if (totalItems > 0) {
            emit exportProgress(20 + (60 * itemIndex / totalItems));
        }
    }
    
    // Export selected motifs
    for (const Motif &motif : motifsToExport) {
        QByteArray trackData;
        
        int currentTick = 0;
        int channel = 0;
        const RhythmGrid &grid = motif.rhythmGrid();
        const QList<int> &pitches = motif.pitchContour();
        int pitchIndex = 0;
        
        for (int i = 0; i < grid.cellCount() && pitchIndex < pitches.size(); i++) {
            RhythmCell cell = grid.cell(i);
            int duration = durationToTicks(cell.duration, ticksPerQuarter);
            
            if (!cell.isRest && pitchIndex < pitches.size()) {
                int midiNote = scaleDegreeToMIDI(pitches[pitchIndex], sketch.key());
                
                // Note on (delta = 0 for first, calculated for subsequent)
                writeVariableLengthQuantity(trackData, 0);
                trackData.append(static_cast<char>(0x90 | channel));
                trackData.append(static_cast<char>(midiNote));
                trackData.append('\x64');
                
                // Note off
                writeVariableLengthQuantity(trackData, duration);
                trackData.append(static_cast<char>(0x80 | channel));
                trackData.append(static_cast<char>(midiNote));
                trackData.append('\x00');
                
                pitchIndex++;
            } else {
                // Rest - just advance time
                currentTick += duration;
            }
        }
        
        // End of track
        trackData.append('\x00');
        trackData.append('\xFF');
        trackData.append('\x2F');
        trackData.append('\x00');
        
        writeMIDITrack(stream, trackData);
        
        itemIndex++;
        if (totalItems > 0) {
            emit exportProgress(20 + (60 * itemIndex / totalItems));
        }
    }

    emit exportProgress(100);
    file.close();
    return true;
}

QString ExportEngine::keyToMusicXMLFifths(const QString &key) const
{
    // Map keys to circle of fifths position
    QString root = key.split(" ").first().toUpper();
    bool isMinor = key.contains("minor", Qt::CaseInsensitive);
    
    // Major keys
    static QMap<QString, int> majorFifths = {
        {"C", 0}, {"G", 1}, {"D", 2}, {"A", 3}, {"E", 4}, {"B", 5}, {"F#", 6}, {"GB", 6},
        {"F", -1}, {"BB", -2}, {"EB", -3}, {"AB", -4}, {"DB", -5}, {"C#", 7}
    };
    
    // Minor keys are 3 fifths below their relative major
    static QMap<QString, int> minorFifths = {
        {"A", 0}, {"E", 1}, {"B", 2}, {"F#", 3}, {"C#", 4}, {"G#", 5}, {"D#", 6},
        {"D", -1}, {"G", -2}, {"C", -3}, {"F", -4}, {"BB", -5}
    };
    
    int fifths = isMinor ? minorFifths.value(root, 0) : majorFifths.value(root, 0);
    return QString::number(fifths);
}

QString ExportEngine::keyToMusicXMLMode(const QString &key) const
{
    return key.contains("minor", Qt::CaseInsensitive) ? "minor" : "major";
}

bool ExportEngine::writeMusicXMLFile(const Sketch &sketch, const QString &filePath,
                                      const QStringList &sectionIds, const QStringList &motifIds)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << filePath;
        return false;
    }

    // Filter sections and motifs based on provided IDs
    QList<Section> sectionsToExport;
    QList<Motif> motifsToExport;
    
    if (!sectionIds.isEmpty()) {
        for (const Section &section : sketch.sections()) {
            if (sectionIds.contains(section.id())) {
                sectionsToExport.append(section);
            }
        }
    }
    
    if (!motifIds.isEmpty()) {
        for (const Motif &motif : sketch.motifs()) {
            if (motifIds.contains(motif.id())) {
                motifsToExport.append(motif);
            }
        }
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 3.1 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">");
    
    xml.writeStartElement("score-partwise");
    xml.writeAttribute("version", "3.1");

    // Work info
    xml.writeStartElement("work");
    xml.writeTextElement("work-title", sketch.name());
    xml.writeEndElement(); // work

    // Identification
    xml.writeStartElement("identification");
    xml.writeStartElement("encoding");
    xml.writeTextElement("software", "MuseSketch");
    xml.writeTextElement("encoding-date", QDate::currentDate().toString("yyyy-MM-dd"));
    xml.writeEndElement(); // encoding
    xml.writeEndElement(); // identification

    emit exportProgress(10);

    // Part list
    xml.writeStartElement("part-list");
    
    // Determine parts based on content
    bool hasSATB = false;
    for (const Section &section : sectionsToExport) {
        if (section.textureType() == TextureType::SATBChorale && section.hasSATBVoices()) {
            hasSATB = true;
            break;
        }
    }

    if (hasSATB) {
        QStringList voiceNames = {"Soprano", "Alto", "Tenor", "Bass"};
        for (int i = 0; i < 4; i++) {
            xml.writeStartElement("score-part");
            xml.writeAttribute("id", QString("P%1").arg(i + 1));
            xml.writeTextElement("part-name", voiceNames[i]);
            xml.writeEndElement();
        }
    } else {
        xml.writeStartElement("score-part");
        xml.writeAttribute("id", "P1");
        xml.writeTextElement("part-name", "Melody");
        xml.writeEndElement();
    }
    xml.writeEndElement(); // part-list

    emit exportProgress(20);

    // Helper lambda to write a note
    auto writeNote = [&xml, this, &sketch](const NoteEvent &event, bool isChord, int divisions) {
        xml.writeStartElement("note");
        
        if (isChord) {
            xml.writeEmptyElement("chord");
        }
        
        if (event.isRest) {
            xml.writeEmptyElement("rest");
        } else {
            xml.writeStartElement("pitch");
            
            // Convert scale degree to pitch
            int midiNote = scaleDegreeToMIDI(event.scaleDegree, sketch.key());
            int octave = midiNote / 12 - 1;
            int pitchClass = midiNote % 12;
            
            static const char* stepNames[] = {"C", "C", "D", "D", "E", "F", "F", "G", "G", "A", "A", "B"};
            static const int alters[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
            
            xml.writeTextElement("step", stepNames[pitchClass]);
            if (alters[pitchClass] != 0) {
                xml.writeTextElement("alter", QString::number(alters[pitchClass]));
            }
            xml.writeTextElement("octave", QString::number(octave));
            xml.writeEndElement(); // pitch
        }
        
        // Duration
        int durationValue = durationToTicks(event.duration, divisions);
        xml.writeTextElement("duration", QString::number(durationValue));
        
        // Type
        QString type;
        if (event.duration == "whole") type = "whole";
        else if (event.duration == "half" || event.duration == "dotted-half") type = "half";
        else if (event.duration == "quarter" || event.duration == "dotted-quarter") type = "quarter";
        else if (event.duration == "eighth" || event.duration == "dotted-eighth") type = "eighth";
        else if (event.duration == "sixteenth") type = "16th";
        else type = "quarter";
        
        xml.writeTextElement("type", type);
        
        // Dot for dotted notes
        if (event.duration.startsWith("dotted")) {
            xml.writeEmptyElement("dot");
        }
        
        xml.writeEndElement(); // note
    };

    // Write parts
    if (hasSATB) {
        for (int voice = 0; voice < 4; voice++) {
            xml.writeStartElement("part");
            xml.writeAttribute("id", QString("P%1").arg(voice + 1));
            
            int measureNumber = 1;
            for (const Section &section : sectionsToExport) {
                QList<NoteEvent> events;
                if (section.textureType() == TextureType::SATBChorale && section.hasSATBVoices()) {
                    events = section.flattenVoiceToTimeline(voice);
                } else if (voice == 0) {
                    // Only soprano gets melody in non-SATB sections
                    events = section.flattenToTimeline(sketch);
                }
                
                // Group events by measure
                double beatsPerMeasure = 4.0; // Assuming 4/4
                QString timeSig = sketch.timeSignature();
                int numerator = timeSig.split("/").first().toInt();
                beatsPerMeasure = numerator;
                
                int divisions = 1; // Divisions per quarter note
                
                for (int bar = 0; bar < section.lengthBars(); bar++) {
                    xml.writeStartElement("measure");
                    xml.writeAttribute("number", QString::number(measureNumber));
                    
                    // Attributes on first measure
                    if (measureNumber == 1) {
                        xml.writeStartElement("attributes");
                        xml.writeTextElement("divisions", QString::number(divisions));
                        
                        xml.writeStartElement("key");
                        xml.writeTextElement("fifths", keyToMusicXMLFifths(sketch.key()));
                        xml.writeTextElement("mode", keyToMusicXMLMode(sketch.key()));
                        xml.writeEndElement(); // key
                        
                        xml.writeStartElement("time");
                        xml.writeTextElement("beats", QString::number(numerator));
                        xml.writeTextElement("beat-type", timeSig.split("/").last());
                        xml.writeEndElement(); // time
                        
                        xml.writeStartElement("clef");
                        xml.writeTextElement("sign", voice < 2 ? "G" : "F");
                        xml.writeTextElement("line", voice < 2 ? "2" : "4");
                        xml.writeEndElement(); // clef
                        
                        xml.writeEndElement(); // attributes
                        
                        // Direction (tempo)
                        xml.writeStartElement("direction");
                        xml.writeAttribute("placement", "above");
                        xml.writeStartElement("direction-type");
                        xml.writeStartElement("metronome");
                        xml.writeTextElement("beat-unit", "quarter");
                        xml.writeTextElement("per-minute", QString::number(sketch.tempo()));
                        xml.writeEndElement(); // metronome
                        xml.writeEndElement(); // direction-type
                        xml.writeEndElement(); // direction
                    }
                    
                    // Write notes for this measure
                    double measureStartBeat = bar * beatsPerMeasure;
                    double measureEndBeat = (bar + 1) * beatsPerMeasure;
                    
                    bool hasNotes = false;
                    for (const NoteEvent &event : events) {
                        if (event.startBeat >= measureStartBeat && event.startBeat < measureEndBeat) {
                            writeNote(event, false, divisions);
                            hasNotes = true;
                        }
                    }
                    
                    // If no notes, write a whole rest
                    if (!hasNotes) {
                        xml.writeStartElement("note");
                        xml.writeEmptyElement("rest");
                        xml.writeTextElement("duration", QString::number(static_cast<int>(beatsPerMeasure)));
                        xml.writeTextElement("type", "whole");
                        xml.writeEndElement(); // note
                    }
                    
                    xml.writeEndElement(); // measure
                    measureNumber++;
                }
            }
            
            xml.writeEndElement(); // part
            emit exportProgress(20 + (60 * (voice + 1) / 4));
        }
    } else {
        // Single melody part
        xml.writeStartElement("part");
        xml.writeAttribute("id", "P1");
        
        int measureNumber = 1;
        int divisions = 1;
        
        // Export motifs first
        for (const Motif &motif : motifsToExport) {
                const RhythmGrid &grid = motif.rhythmGrid();
                const QList<int> &pitches = motif.pitchContour();
                
                QString timeSig = sketch.timeSignature();
                int numerator = timeSig.split("/").first().toInt();
                double beatsPerMeasure = numerator;
                
                for (int bar = 0; bar < motif.lengthBars(); bar++) {
                    xml.writeStartElement("measure");
                    xml.writeAttribute("number", QString::number(measureNumber));
                    
                    if (measureNumber == 1) {
                        xml.writeStartElement("attributes");
                        xml.writeTextElement("divisions", QString::number(divisions));
                        
                        xml.writeStartElement("key");
                        xml.writeTextElement("fifths", keyToMusicXMLFifths(sketch.key()));
                        xml.writeTextElement("mode", keyToMusicXMLMode(sketch.key()));
                        xml.writeEndElement();
                        
                        xml.writeStartElement("time");
                        xml.writeTextElement("beats", QString::number(numerator));
                        xml.writeTextElement("beat-type", timeSig.split("/").last());
                        xml.writeEndElement();
                        
                        xml.writeStartElement("clef");
                        xml.writeTextElement("sign", "G");
                        xml.writeTextElement("line", "2");
                        xml.writeEndElement();
                        
                        xml.writeEndElement(); // attributes
                    }
                    
                    // Write notes - simplified: just output all notes in sequence
                    int pitchIndex = 0;
                    for (int i = 0; i < grid.cellCount() && pitchIndex < pitches.size(); i++) {
                        RhythmCell cell = grid.cell(i);
                        
                        NoteEvent event;
                        event.duration = cell.duration;
                        event.isRest = cell.isRest;
                        event.scaleDegree = cell.isRest ? 0 : pitches[pitchIndex];
                        
                        writeNote(event, false, divisions);
                        
                        if (!cell.isRest) pitchIndex++;
                    }
                    
                    xml.writeEndElement(); // measure
                    measureNumber++;
                }
        }
        
        // Export sections
        for (const Section &section : sectionsToExport) {
                QList<NoteEvent> events = section.flattenToTimeline(sketch);
                
                QString timeSig = sketch.timeSignature();
                int numerator = timeSig.split("/").first().toInt();
                double beatsPerMeasure = numerator;
                
                for (int bar = 0; bar < section.lengthBars(); bar++) {
                    xml.writeStartElement("measure");
                    xml.writeAttribute("number", QString::number(measureNumber));
                    
                    if (measureNumber == 1) {
                        xml.writeStartElement("attributes");
                        xml.writeTextElement("divisions", QString::number(divisions));
                        
                        xml.writeStartElement("key");
                        xml.writeTextElement("fifths", keyToMusicXMLFifths(sketch.key()));
                        xml.writeTextElement("mode", keyToMusicXMLMode(sketch.key()));
                        xml.writeEndElement();
                        
                        xml.writeStartElement("time");
                        xml.writeTextElement("beats", QString::number(numerator));
                        xml.writeTextElement("beat-type", timeSig.split("/").last());
                        xml.writeEndElement();
                        
                        xml.writeStartElement("clef");
                        xml.writeTextElement("sign", "G");
                        xml.writeTextElement("line", "2");
                        xml.writeEndElement();
                        
                        xml.writeEndElement();
                        
                        xml.writeStartElement("direction");
                        xml.writeAttribute("placement", "above");
                        xml.writeStartElement("direction-type");
                        xml.writeStartElement("metronome");
                        xml.writeTextElement("beat-unit", "quarter");
                        xml.writeTextElement("per-minute", QString::number(sketch.tempo()));
                        xml.writeEndElement();
                        xml.writeEndElement();
                        xml.writeEndElement();
                    }
                    
                    double measureStartBeat = bar * beatsPerMeasure;
                    double measureEndBeat = (bar + 1) * beatsPerMeasure;
                    
                    bool hasNotes = false;
                    for (const NoteEvent &event : events) {
                        if (event.startBeat >= measureStartBeat && event.startBeat < measureEndBeat) {
                            writeNote(event, false, divisions);
                            hasNotes = true;
                        }
                    }
                    
                    if (!hasNotes) {
                        xml.writeStartElement("note");
                        xml.writeEmptyElement("rest");
                        xml.writeTextElement("duration", QString::number(static_cast<int>(beatsPerMeasure)));
                        xml.writeTextElement("type", "whole");
                        xml.writeEndElement();
                    }
                    
                    xml.writeEndElement(); // measure
                    measureNumber++;
                }
            }
        
        xml.writeEndElement(); // part
    }

    xml.writeEndElement(); // score-partwise
    xml.writeEndDocument();

    emit exportProgress(100);
    file.close();
    return true;
}

bool ExportEngine::writeMSCZFile(const Sketch &sketch, const QString &filePath)
{
    // MSCZ is a ZIP file containing MusicXML and metadata
    // For now, we'll export as MusicXML with .mscz extension
    // Full MSCZ support would require MuseScore engine integration
    
    qWarning() << "MSCZ export is experimental - exporting as MusicXML internally";
    
    // For a proper implementation, we would:
    // 1. Create a ZIP archive
    // 2. Add a META-INF/container.xml
    // 3. Add the score as .mscx (MuseScore native format)
    // 4. Add any embedded resources
    
    // For now, export as MusicXML (most DAWs and notation apps can import this)
    QString xmlPath = filePath;
    if (xmlPath.endsWith(".mscz")) {
        xmlPath = xmlPath.left(xmlPath.length() - 5) + ".musicxml";
    }
    
    // Export everything (empty lists = all content)
    QStringList allSectionIds, allMotifIds;
    for (const Section &s : sketch.sections()) {
        allSectionIds.append(s.id());
    }
    for (const Motif &m : sketch.motifs()) {
        allMotifIds.append(m.id());
    }
    bool success = writeMusicXMLFile(sketch, xmlPath, allSectionIds, allMotifIds);
    
    if (success) {
        emit exportCompleted(xmlPath);
        qInfo() << "Note: Exported as MusicXML. For native .mscz, open in MuseScore Studio and re-save.";
    }
    
    return success;
}

