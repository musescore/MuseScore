#include "fretdiagramsettingsmodel.h"

FretDiagramSettingsModel::FretDiagramSettingsModel(QObject* parent, IElementRepositoryService* repository)
  : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_FRET_DIAGRAM);
    setTitle(tr("Fretboard Diagram"));
    createProperties();

    connect(m_repository->getQObject(), SIGNAL(elementsUpdated()), this, SLOT(onElementsUpdated()));
}

void FretDiagramSettingsModel::createProperties()
{
    m_strings = buildPropertyItem(Ms::Pid::FRET_STRINGS);
    m_frets = buildPropertyItem(Ms::Pid::FRET_FRETS);
    m_showNut = buildPropertyItem(Ms::Pid::FRET_NUT);
    m_offset = buildPropertyItem(Ms::Pid::FRET_OFFSET);
    m_numPos = buildPropertyItem(Ms::Pid::FRET_NUM_POS);
}

void FretDiagramSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::FRET_DIAGRAM);
}

void FretDiagramSettingsModel::loadProperties()
{
    loadPropertyItem(m_strings);
    loadPropertyItem(m_frets);
    loadPropertyItem(m_showNut);
    loadPropertyItem(m_offset);
    loadPropertyItem(m_numPos);
}

void FretDiagramSettingsModel::resetProperties()
{
    m_strings->resetToDefault();
    m_frets->resetToDefault();
    m_showNut->resetToDefault();
    m_offset->resetToDefault();
    m_numPos->resetToDefault();
}

bool FretDiagramSettingsModel::canvasVisible() const
{
    // show canvas only if a single fret diagram is selected
    return m_elementList.size() == 1;
}

Ms::FretDiagram* FretDiagramSettingsModel::fretDiagram() const
{
    return Ms::toFretDiagram(m_elementList[0]);
}

// FretDiagramTypes::FretDot FretDiagramSettingsModel::dot(int string, int fret) const
int FretDiagramSettingsModel::dot(int string, int fret) const
{
    if (m_elementList.size() != 1)
        // return FretDiagramTypes::FretDot::DOT_NONE;
        return -1;
    Ms::FretItem::Dot dot = fretDiagram()->dot(string, fret)[0];
    // return dot.fret ? static_cast<FretDiagramTypes::FretDot>(dot.dtype) : FretDiagramTypes::FretDot::DOT_NONE;
    return dot.fret ? static_cast<int>(dot.dtype) : -1;
}

// FretDiagramTypes::FretMarker FretDiagramSettingsModel::marker(int string) const
int FretDiagramSettingsModel::marker(int string) const
{
    if (m_elementList.size() != 1)
        // return FretDiagramTypes::FretMarker::MARKER_NONE;
        return 0;
    // return static_cast<FretDiagramTypes::FretMarker>(fretDiagram()->marker(string).mtype);
    return static_cast<int>(fretDiagram()->marker(string).mtype);
}

bool FretDiagramSettingsModel::barreExists(int fret) const
{
    if (!m_elementList.size())
        return false;
    return fretDiagram()->barre(fret).exists();
}

int FretDiagramSettingsModel::barreStartString(int fret) const
{
    if (!m_elementList.size())
        return -1;
    return fretDiagram()->barre(fret).startString;
}

int FretDiagramSettingsModel::barreEndString(int fret) const
{
    if (!m_elementList.size())
        return -1;
    return fretDiagram()->barre(fret).endString;
}

qreal FretDiagramSettingsModel::barreLineWidth() const
{
    if (!m_elementList.size())
        return 1.0;
    return fretDiagram()->score()->styleD(Ms::Sid::barreLineWidth);
}

void FretDiagramSettingsModel::setDot(int string, int fret, bool add, FretDiagramTypes::FretDot dType)
{
    if (!m_elementList.size())
        return;
    fretDiagram()->score()->startCmd();
    fretDiagram()->setDot(string, fret, add, static_cast<Ms::FretDotType>(dType));
    fretDiagram()->score()->endCmd();
}

void FretDiagramSettingsModel::setMarker(int string, FretDiagramTypes::FretMarker mType)
{
    if (!m_elementList.size())
        return;
    fretDiagram()->score()->startCmd();
    fretDiagram()->setMarker(string, static_cast<Ms::FretMarkerType>(mType));
    fretDiagram()->score()->endCmd();
}

void FretDiagramSettingsModel::onElementsUpdated()
{
    emit selectionChanged();
}

PropertyItem* FretDiagramSettingsModel::strings() const
{
    return m_strings;
}

PropertyItem* FretDiagramSettingsModel::frets() const
{
    return m_frets;
}

PropertyItem* FretDiagramSettingsModel::showNut() const
{
    return m_showNut;
}

PropertyItem* FretDiagramSettingsModel::offset() const
{
    return m_offset;
}

PropertyItem* FretDiagramSettingsModel::numPos() const
{
    return m_numPos;
}
