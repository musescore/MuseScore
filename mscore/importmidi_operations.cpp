#include "importmidi_operations.h"

namespace Ms {


void MidiImportOperations::addTrackOperations(const MidiTrackOperations &operations)
      {
      operations_.push_back(operations);
      }

void MidiImportOperations::clear()
      {
      operations_.clear();
      }
}

