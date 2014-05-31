#include "importmidi_opmodel.h"
#include "importmidi_operations.h"
#include "preferences.h"
#include "libmscore/mscore.h"


namespace Ms {

extern Preferences preferences;

struct Node {
      QString name;
      MidiOperation oper;
      QStringList values;
      bool visible = true;
      Node *parent = nullptr;
      std::vector<std::unique_ptr<Node> > children;
      };

struct Controller {
      Node *LHRHdoIt = nullptr;
      Node *LHRHMethod = nullptr;
      Node *LHRHPitchOctave = nullptr;
      Node *LHRHPitchNote = nullptr;
      Node *quantValue = nullptr;
      Node *quantHuman = nullptr;
      Node *searchTuplets = nullptr;
      Node *duplets = nullptr;
      Node *triplets = nullptr;
      Node *quadruplets = nullptr;
      Node *quintuplets = nullptr;
      Node *septuplets = nullptr;
      Node *nonuplets = nullptr;
      Node *allowedVoices = nullptr;
      Node *separateVoices = nullptr;
      Node *splitDrums = nullptr;
      Node *showStaffBracket = nullptr;
      Node *pickupMeasure = nullptr;
      Node *clef = nullptr;
      Node *removeDrumRests = nullptr;

      bool isDrumTrack = false;
      bool allTracksSelected = true;

      bool updateNodeDependencies(Node *node, bool forceUpdate);
      };

OperationsModel::OperationsModel()
            : root(std::unique_ptr<Node>(new Node()))
            , controller(std::unique_ptr<Controller>(new Controller()))
            , updateQuantTimer(new QTimer)
      {
      connect(updateQuantTimer, SIGNAL(timeout()), this, SLOT(updateQuantValue()));
      updateQuantTimer->start(100);

      beginResetModel();
                  // - initialize opeations with their default values
                  // - string lists below should match Operation enum values
      Node *quantValue = new Node;
      quantValue->name = QCoreApplication::translate("MIDI import operations", "Max quantization value");
      quantValue->oper.type = MidiOperation::Type::QUANT_VALUE;
      quantValue->oper.value = (int)TrackOperations().quantize.value;
      quantValue->values.push_back(QCoreApplication::translate("MIDI import operations", "Value from preferences"));
      quantValue->values.push_back(QCoreApplication::translate("MIDI import operations", "Quarter"));
      quantValue->values.push_back(QCoreApplication::translate("MIDI import operations", "Eighth"));
      quantValue->values.push_back(QCoreApplication::translate("MIDI import operations", "16th"));
      quantValue->values.push_back(QCoreApplication::translate("MIDI import operations", "32nd"));
      quantValue->values.push_back(QCoreApplication::translate("MIDI import operations", "64th"));
      quantValue->values.push_back(QCoreApplication::translate("MIDI import operations", "128th"));
      quantValue->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(quantValue));
      controller->quantValue = quantValue;


      Node *humanPerformance = new Node;
      humanPerformance->name = QCoreApplication::translate("MIDI import operations", "Human performance");
      humanPerformance->oper.type = MidiOperation::Type::QUANT_HUMAN;
      humanPerformance->oper.value = Quantization().humanPerformance;
      humanPerformance->parent = quantValue;
      quantValue->children.push_back(std::unique_ptr<Node>(humanPerformance));
      controller->quantHuman = humanPerformance;


      Node *useDots = new Node;
      useDots->name = QCoreApplication::translate("MIDI import operations", "Use dots");
      useDots->oper.type = MidiOperation::Type::USE_DOTS;
      useDots->oper.value = TrackOperations().useDots;
      useDots->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(useDots));


      Node *simplifyDurations = new Node;
      simplifyDurations->name = QCoreApplication::translate("MIDI import operations", "Simplify durations");
      simplifyDurations->oper.type = MidiOperation::Type::SIMPLIFY_DURATIONS;
      simplifyDurations->oper.value = TrackOperations().simplifyDurations;
      simplifyDurations->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(simplifyDurations));


      Node *allowedVoices = new Node;
      allowedVoices->name = QCoreApplication::translate("MIDI import operations", "Max allowed voices");
      allowedVoices->oper.type = MidiOperation::Type::ALLOWED_VOICES;
      allowedVoices->oper.value = (int)TrackOperations().allowedVoices;
      allowedVoices->values.push_back(QCoreApplication::translate("MIDI import operations", "1"));
      allowedVoices->values.push_back(QCoreApplication::translate("MIDI import operations", "2"));
      allowedVoices->values.push_back(QCoreApplication::translate("MIDI import operations", "3"));
      allowedVoices->values.push_back(QCoreApplication::translate("MIDI import operations", "4"));
      allowedVoices->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(allowedVoices));
      controller->allowedVoices = allowedVoices;


      Node *separateVoices = new Node;
      separateVoices->name = QCoreApplication::translate("MIDI import operations", "Separate voices");
      separateVoices->oper.type = MidiOperation::Type::SEPARATE_VOICES;
      separateVoices->oper.value = TrackOperations().separateVoices;
      separateVoices->parent = allowedVoices;
      allowedVoices->children.push_back(std::unique_ptr<Node>(separateVoices));
      controller->separateVoices = separateVoices;


      // ------------- tuplets --------------

      Node *searchTuplets = new Node;
      searchTuplets->name = QCoreApplication::translate("MIDI import operations", "Search tuplets");
      searchTuplets->oper.type = MidiOperation::Type::TUPLET_SEARCH;
      searchTuplets->oper.value = TrackOperations().tuplets.doSearch;
      searchTuplets->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(searchTuplets));
      controller->searchTuplets = searchTuplets;


      Node *duplets = new Node;
      duplets->name = QCoreApplication::translate("MIDI import operations", "Duplets (2)");
      duplets->oper.type = MidiOperation::Type::TUPLET_2;
      duplets->oper.value = TrackOperations().tuplets.duplets;
      duplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(duplets));
      controller->duplets = duplets;


      Node *triplets = new Node;
      triplets->name = QCoreApplication::translate("MIDI import operations", "Triplets (3)");
      triplets->oper.type = MidiOperation::Type::TUPLET_3;
      triplets->oper.value = TrackOperations().tuplets.triplets;
      triplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(triplets));
      controller->triplets = triplets;


      Node *quadruplets = new Node;
      quadruplets->name = QCoreApplication::translate("MIDI import operations", "Quadruplets (4)");
      quadruplets->oper.type = MidiOperation::Type::TUPLET_4;
      quadruplets->oper.value = TrackOperations().tuplets.quadruplets;
      quadruplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(quadruplets));
      controller->quadruplets = quadruplets;


      Node *quintuplets = new Node;
      quintuplets->name = QCoreApplication::translate("MIDI import operations", "Quintuplets (5)");
      quintuplets->oper.type = MidiOperation::Type::TUPLET_5;
      quintuplets->oper.value = TrackOperations().tuplets.quintuplets;
      quintuplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(quintuplets));
      controller->quintuplets = quintuplets;


      Node *septuplets = new Node;
      septuplets->name = QCoreApplication::translate("MIDI import operations", "Septuplets (7)");
      septuplets->oper.type = MidiOperation::Type::TUPLET_7;
      septuplets->oper.value = TrackOperations().tuplets.septuplets;
      septuplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(septuplets));
      controller->septuplets = septuplets;


      Node *nonuplets = new Node;
      nonuplets->name = QCoreApplication::translate("MIDI import operations", "Nonuplets (9)");
      nonuplets->oper.type = MidiOperation::Type::TUPLET_9;
      nonuplets->oper.value = TrackOperations().tuplets.nonuplets;
      nonuplets->parent = searchTuplets;
      searchTuplets->children.push_back(std::unique_ptr<Node>(nonuplets));
      controller->nonuplets = nonuplets;

      // ------------------------------------

      Node *pickupMeasure = new Node;
      pickupMeasure->name = QCoreApplication::translate("MIDI import operations", "Recognize pickup measure");
      pickupMeasure->oper.type = MidiOperation::Type::PICKUP_MEASURE;
      pickupMeasure->oper.value = TrackOperations().pickupMeasure;
      pickupMeasure->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(pickupMeasure));
      controller->pickupMeasure = pickupMeasure;


      Node *swing = new Node;
      swing->name = QCoreApplication::translate("MIDI import operations", "Detect swing");
      swing->oper.type = MidiOperation::Type::SWING;
      swing->oper.value = (int)TrackOperations().swing;
      swing->values.push_back(QCoreApplication::translate("MIDI import operations", "None (1:1)"));
      swing->values.push_back(QCoreApplication::translate("MIDI import operations", "Swing (2:1)"));
      swing->values.push_back(QCoreApplication::translate("MIDI import operations", "Shuffle (3:1)"));
      swing->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(swing));


      Node *changeClef = new Node;
      changeClef->name = QCoreApplication::translate("MIDI import operations", "Allow clef changes within a staff");
      changeClef->oper.type = MidiOperation::Type::CHANGE_CLEF;
      changeClef->oper.value = TrackOperations().changeClef;
      changeClef->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(changeClef));
      controller->clef = changeClef;


      Node *removeDrumRests = new Node;
      removeDrumRests->name = QCoreApplication::translate("MIDI import operations", "Remove rests and ties between notes");
      removeDrumRests->oper.type = MidiOperation::Type::REMOVE_DRUM_RESTS;
      removeDrumRests->oper.value = TrackOperations().removeDrumRests;
      removeDrumRests->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(removeDrumRests));
      controller->removeDrumRests = removeDrumRests;


      Node *splitDrums = new Node;
      splitDrums->name = QCoreApplication::translate("MIDI import operations", "Split drum set");
      splitDrums->oper.type = MidiOperation::Type::SPLIT_DRUMS;
      splitDrums->oper.value = TrackOperations().splitDrums.doSplit;;
      splitDrums->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(splitDrums));
      controller->splitDrums = splitDrums;


      Node *showStaffBracket = new Node;
      showStaffBracket->name = QCoreApplication::translate("MIDI import operations", "Show staff bracket");
      showStaffBracket->oper.type = MidiOperation::Type::SHOW_STAFF_BRACKET;
      showStaffBracket->oper.value = TrackOperations().splitDrums.showStaffBracket;
      showStaffBracket->parent = splitDrums;
      splitDrums->children.push_back(std::unique_ptr<Node>(showStaffBracket));
      controller->showStaffBracket = showStaffBracket;


      Node *doLHRH = new Node;
      doLHRH->name = QCoreApplication::translate("MIDI import operations", "Left/right hand separation");
      doLHRH->oper.type = MidiOperation::Type::DO_LHRH_SEPARATION;
      doLHRH->oper.value = LHRHSeparation().doIt;
      doLHRH->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(doLHRH));
      controller->LHRHdoIt = doLHRH;


      Node *LHRHMethod = new Node;
      LHRHMethod->name = QCoreApplication::translate("MIDI import operations", "Separation method");
      LHRHMethod->oper.type = MidiOperation::Type::LHRH_METHOD;
      LHRHMethod->oper.value = (int)LHRHSeparation().method;
      LHRHMethod->values.push_back(QCoreApplication::translate("MIDI import operations", "Hand width"));
      LHRHMethod->values.push_back(QCoreApplication::translate("MIDI import operations", "Fixed pitch"));
      LHRHMethod->parent = doLHRH;
      doLHRH->children.push_back(std::unique_ptr<Node>(LHRHMethod));
      controller->LHRHMethod = LHRHMethod;


      Node *LHRHPitchOctave = new Node;
      LHRHPitchOctave->name = QCoreApplication::translate("MIDI import operations", "Split pitch octave");
      LHRHPitchOctave->oper.type = MidiOperation::Type::LHRH_SPLIT_OCTAVE;
      LHRHPitchOctave->oper.value = (int)LHRHSeparation().splitPitchOctave;
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C-1"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C0"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C1"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C2"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C3"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C4"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C5"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C6"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C7"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C8"));
      LHRHPitchOctave->values.push_back(QCoreApplication::translate("MIDI import operations", "C9"));
      LHRHPitchOctave->parent = LHRHMethod;
      LHRHMethod->children.push_back(std::unique_ptr<Node>(LHRHPitchOctave));
      controller->LHRHPitchOctave = LHRHPitchOctave;


      Node *LHRHPitchNote = new Node;
      LHRHPitchNote->name = QCoreApplication::translate("MIDI import operations", "Split pitch note");
      LHRHPitchNote->oper.type = MidiOperation::Type::LHRH_SPLIT_NOTE;
      LHRHPitchNote->oper.value = (int)LHRHSeparation().splitPitchNote;
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "C"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "C#"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "D"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "D#"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "E"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "F"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "F#"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "G"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "G#"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "A"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "A#"));
      LHRHPitchNote->values.push_back(QCoreApplication::translate("MIDI import operations", "B"));
      LHRHPitchNote->parent = LHRHMethod;
      LHRHMethod->children.push_back(std::unique_ptr<Node>(LHRHPitchNote));
      controller->LHRHPitchNote = LHRHPitchNote;

      //--------------------------------------------------------------------
      connect(this,
              SIGNAL(dataChanged(QModelIndex,QModelIndex)),
              SLOT(onDataChanged(QModelIndex)));
      controller->updateNodeDependencies(nullptr, true);
      endResetModel();
      }

OperationsModel::~OperationsModel()
      {
      }

QModelIndex OperationsModel::index(int row, int column, const QModelIndex &parent) const
      {
      if (!root || row < 0 || column < 0 || column >= OperationCol::OCOL_COUNT)
            return QModelIndex();
      const Node *parentNode = nodeFromIndex(parent);
      if (!parentNode)
            return QModelIndex();
      if (parentNode->children.empty() || row >= (int)parentNode->children.size())
            return QModelIndex();
                  // find new row in connection with invisible items
      int shift = 0;
      for (int i = 0; i <= row + shift; ++i) {
            if (i >= (int)parentNode->children.size())
                  return QModelIndex();
            if (!parentNode->children.at(i)->visible)
                  ++shift;
            }
      Node *childNode = parentNode->children.at(row + shift).get();
      if (!childNode || !childNode->visible)
            return QModelIndex();
      return createIndex(row, column, childNode);
      }

QModelIndex OperationsModel::parent(const QModelIndex &child) const
      {
      const Node *node = nodeFromIndex(child);
      if (!node)
            return QModelIndex();
      Node *parentNode = node->parent;
      if (!parentNode)
            return QModelIndex();
      const Node *grandparentNode = parentNode->parent;
      if (!grandparentNode)
            return QModelIndex();
      const auto &children = grandparentNode->children;
      const auto iter = std::find_if(children.begin(), children.end(),
               [parentNode](const std::unique_ptr<Node> &el){ return el.get() == parentNode; });
      const int row = (iter == children.end()) ? -1 : iter - children.begin();
      return createIndex(row, 0, parentNode);
      }

int OperationsModel::rowCount(const QModelIndex &parent) const
      {
      if (parent.column() >= OperationCol::OCOL_COUNT)
            return 0;
      const Node *parentNode = nodeFromIndex(parent);
      if (!parentNode)
            return 0;
                  // take only visible nodes into account
      size_t counter = 0;
      for (const auto &p: parentNode->children)
            if (p->visible)
                  ++counter;
      return counter;
      }

int OperationsModel::columnCount(const QModelIndex &/*parent*/) const
      {
      return OperationCol::OCOL_COUNT;
      }

// All nodes can have either bool value or list of possible values
// also node value can be undefined (QVariant()), for example grayed checkbox

QVariant OperationsModel::data(const QModelIndex &index, int role) const
      {
      const Node *node = nodeFromIndex(index);
      if (!node)
            return QVariant();
      switch (role) {
            case DataRole:
                  if (node->values.empty())  // checkbox
                        return node->oper.value.toBool();
                  else
                        return node->oper.value.toInt();
                  break;
            case Qt::DisplayRole:
                  switch (index.column()) {
                        case OperationCol::OPER_NAME:
                              return node->name;
                        case OperationCol::VALUE:
                              if (!node->values.empty()) {
                                    if (!node->oper.value.isValid()) // undefined operation value
                                          return " . . . ";
                                                // list contains names of possible string values
                                                // like {"1/4", "1/8"}
                                                // valid node value is one of enum items
                                                // -> use enum item as index
                                    int indexOfValue = node->oper.value.toInt();
                                    if (indexOfValue < node->values.size() && indexOfValue >= 0)
                                          return node->values.at(indexOfValue);
                                    }
                                          // otherwise return nothing because it's a checkbox
                              break;
                        default:
                              break;
                        }
                  break;
            case Qt::EditRole:
                  if (index.column() == OperationCol::VALUE && !node->values.empty())
                        return node->values;
                  break;
            case Qt::CheckStateRole:
                  if (index.column() == OperationCol::VALUE && node->values.empty()) {
                        if (!node->oper.value.isValid())
                              return Qt::PartiallyChecked;
                        return (node->oper.value.toBool())
                               ? Qt::Checked : Qt::Unchecked;
                        }
                  break;
            case Qt::SizeHintRole:
                  {
                  QSize sz;
                  sz.setHeight(22);
                  return sz;
                  }
            case OperationTypeRole:
                  return (int)node->oper.type;
            default:
                  break;
            }
      return QVariant();
      }

QVariant OperationsModel::headerData(int section, Qt::Orientation orientation, int role) const
      {
      if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            {
            switch (section) {
                  case OperationCol::OPER_NAME:
                        return QCoreApplication::translate("MIDI import operations",
                                         "Selected track [%1] operations").arg(trackLabel);
                  case OperationCol::VALUE:
                        return QCoreApplication::translate("MIDI import operations", "Value");
                  default:
                        break;
                  }
            }
      return QVariant();
      }

Qt::ItemFlags OperationsModel::flags(const QModelIndex &index) const
      {
      const Node *node = nodeFromIndex(index);
      if (!node)
            return Qt::ItemFlags();
      Qt::ItemFlags flags = Qt::ItemFlags(Qt::ItemIsEnabled);
      if (index.column() == OperationCol::VALUE) {
            if (node->values.empty())           // node value is bool - a checkbox
                  flags |= Qt::ItemIsUserCheckable;
            else        // node has list of values
                  flags |= Qt::ItemIsEditable;
            }
      return flags;
      }

bool OperationsModel::setData(const QModelIndex &index, const QVariant &value, int role)
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return false;
      bool result = false;
      if (index.column() == OperationCol::VALUE) {
            switch (role) {
                  case Qt::CheckStateRole:
                        node->oper.value = value.toBool();
                        result = true;
                        break;
                  case Qt::EditRole:
                                    // set enum value from value == list index
                        node->oper.value = value.toInt();
                        result = true;
                        break;
                  default:
                        break;
                  }
            }
      if (result)
            emit dataChanged(index, index);
      return result;
      }

void setNodeOperations(Node *node, const DefinedTrackOperations &opers)
      {
      if (opers.undefinedOpers.contains((int)node->oper.type))
            node->oper.value = QVariant();
      else {
            switch (node->oper.type) {
                  case MidiOperation::Type::DO_IMPORT:
                  case MidiOperation::Type::LYRIC_TRACK_INDEX:
                        break;

                  case MidiOperation::Type::QUANT_VALUE:
                        node->oper.value = (int)opers.opers.quantize.value; break;
                  case MidiOperation::Type::QUANT_HUMAN:
                        node->oper.value = opers.opers.quantize.humanPerformance; break;

                  case MidiOperation::Type::DO_LHRH_SEPARATION:
                        node->oper.value = opers.opers.LHRH.doIt; break;
                  case MidiOperation::Type::LHRH_METHOD:
                        node->oper.value = (int)opers.opers.LHRH.method; break;
                  case MidiOperation::Type::LHRH_SPLIT_OCTAVE:
                        node->oper.value = (int)opers.opers.LHRH.splitPitchOctave; break;
                  case MidiOperation::Type::LHRH_SPLIT_NOTE:
                        node->oper.value = (int)opers.opers.LHRH.splitPitchNote; break;

                  case MidiOperation::Type::USE_DOTS:
                        node->oper.value = opers.opers.useDots; break;

                  case MidiOperation::Type::SIMPLIFY_DURATIONS:
                        node->oper.value = opers.opers.simplifyDurations; break;

                  case MidiOperation::Type::SEPARATE_VOICES:
                        node->oper.value = opers.opers.separateVoices; break;

                  case MidiOperation::Type::SWING:
                        node->oper.value = (int)opers.opers.swing; break;

                  case MidiOperation::Type::ALLOWED_VOICES:
                        node->oper.value = (int)opers.opers.allowedVoices; break;

                  case MidiOperation::Type::TUPLET_SEARCH:
                        node->oper.value = opers.opers.tuplets.doSearch; break;
                  case MidiOperation::Type::TUPLET_2:
                        node->oper.value = opers.opers.tuplets.duplets; break;
                  case MidiOperation::Type::TUPLET_3:
                        node->oper.value = opers.opers.tuplets.triplets; break;
                  case MidiOperation::Type::TUPLET_4:
                        node->oper.value = opers.opers.tuplets.quadruplets; break;
                  case MidiOperation::Type::TUPLET_5:
                        node->oper.value = opers.opers.tuplets.quintuplets; break;
                  case MidiOperation::Type::TUPLET_7:
                        node->oper.value = opers.opers.tuplets.septuplets; break;
                  case MidiOperation::Type::TUPLET_9:
                        node->oper.value = opers.opers.tuplets.nonuplets; break;

                  case MidiOperation::Type::CHANGE_CLEF:
                        node->oper.value = opers.opers.changeClef; break;

                  case MidiOperation::Type::PICKUP_MEASURE:
                        node->oper.value = opers.opers.pickupMeasure; break;

                  case MidiOperation::Type::SPLIT_DRUMS:
                        node->oper.value = opers.opers.splitDrums.doSplit; break;
                  case MidiOperation::Type::SHOW_STAFF_BRACKET:
                        node->oper.value = opers.opers.splitDrums.showStaffBracket; break;
                  case MidiOperation::Type::REMOVE_DRUM_RESTS:
                        node->oper.value = opers.opers.removeDrumRests; break;
                  }
            }
      for (const auto &nodePtr: node->children)
            setNodeOperations(nodePtr.get(), opers);
      }

void OperationsModel::setTrackData(const QString &trackLabel,
                                   const DefinedTrackOperations &opers)
      {
      this->trackLabel = trackLabel;
      controller->isDrumTrack = opers.isDrumTrack;
      controller->allTracksSelected = opers.allTracksSelected;
                  // set new operations values
      beginResetModel();
      for (const auto &nodePtr: root->children)
            setNodeOperations(nodePtr.get(), opers);
      controller->updateNodeDependencies(nullptr, true);
      endResetModel();
      }

void OperationsModel::onDataChanged(const QModelIndex &index)
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return;
      if (controller->updateNodeDependencies(node, false))
            layoutChanged();
      }

void OperationsModel::updateQuantValue()
      {
      const auto newPrefQuantValue = ReducedFraction::fromTicks(preferences.shortestNote);
      if (newPrefQuantValue == prefQuantValue)
            return;

      prefQuantValue = newPrefQuantValue;
      const auto division = ReducedFraction::fromTicks(MScore::division);
      QString value;

      if (prefQuantValue == division)
            value += "Quarter";
      else if (prefQuantValue == division / 2)
            value += "Eighth";
      else if (prefQuantValue == division / 4)
            value += "16th";
      else if (prefQuantValue == division / 8)
            value += "32nd";
      else if (prefQuantValue == division / 16)
            value += "64th";
      else if (prefQuantValue == division / 32)
            value += "128th";
      else
            Q_ASSERT_X(false, "OperationsModel::updateQuantValue", "Unknown quantization value");

      controller->quantValue->values[0] = QCoreApplication::translate(
                        "MIDI import operations", "Value from preferences (%1)").arg(value);
      emit dataChanged(QModelIndex(), QModelIndex());
      }

Node* OperationsModel::nodeFromIndex(const QModelIndex &index) const
      {
      if (index.isValid())
            return static_cast<Node *>(index.internalPointer());
      else
            return root.get();
      }

// Different controller actions, i.e. conditional visibility of node

bool Controller::updateNodeDependencies(Node *node, bool forceUpdate)
      {
      bool result = false;
      if (!node && !forceUpdate)
            return result;
      if (LHRHMethod && (forceUpdate || node == LHRHMethod)) {
            const auto value = (MidiOperation::LHRHMethod)LHRHMethod->oper.value.toInt();
            switch (value) {
                  case MidiOperation::LHRHMethod::HAND_WIDTH:
                        if (LHRHPitchOctave)
                              LHRHPitchOctave->visible = false;
                        if (LHRHPitchNote)
                              LHRHPitchNote->visible = false;
                        result = true;
                        break;
                  case MidiOperation::LHRHMethod::SPECIFIED_PITCH:
                        if (LHRHPitchOctave)
                              LHRHPitchOctave->visible = true;
                        if (LHRHPitchNote)
                              LHRHPitchNote->visible = true;
                        result = true;
                        break;
                  }
            }
      if (LHRHdoIt && (forceUpdate || node == LHRHdoIt)) {
            const auto value = LHRHdoIt->oper.value.toBool();
            if (LHRHMethod)
                  LHRHMethod->visible = value;
            result = true;
            }
      if (allowedVoices && (forceUpdate || node == allowedVoices)) {
            const auto value = (MidiOperation::AllowedVoices)allowedVoices->oper.value.toInt();
            switch (value) {
                  case MidiOperation::AllowedVoices::V_1:
                        if (separateVoices)
                              separateVoices->visible = false;
                        result = true;
                        break;
                  case MidiOperation::AllowedVoices::V_2:
                  case MidiOperation::AllowedVoices::V_3:
                  case MidiOperation::AllowedVoices::V_4:
                        if (separateVoices)
                              separateVoices->visible = true;
                        result = true;
                        break;
                  }
            }
      if (searchTuplets && (forceUpdate || node == searchTuplets)) {
            const auto value = searchTuplets->oper.value.toBool();
            if (duplets)
                  duplets->visible = value;
            if (triplets)
                  triplets->visible = value;
            if (quadruplets)
                  quadruplets->visible = value;
            if (quintuplets)
                  quintuplets->visible = value;
            if (septuplets)
                  septuplets->visible = value;
            if (nonuplets)
                  nonuplets->visible = value;
            result = true;
            }
      if (splitDrums && (forceUpdate || node == splitDrums)) {
            const auto value = splitDrums->oper.value.toBool();
            if (showStaffBracket)
                  showStaffBracket->visible = value;
            result = true;
            }
      if (forceUpdate) {
            if (LHRHdoIt)
                  LHRHdoIt->visible = !isDrumTrack;
            if (splitDrums)
                  splitDrums->visible = isDrumTrack;
            if (removeDrumRests)
                  removeDrumRests->visible = isDrumTrack;
            if (allowedVoices)
                  allowedVoices->visible = !isDrumTrack;
            if (clef)
                  clef->visible = !isDrumTrack;
            if (pickupMeasure)
                  pickupMeasure->visible = allTracksSelected;
            if (quantHuman)
                  quantHuman->visible = allTracksSelected;
            result = true;
            }

      return result;
      }

} // namespace Ms
