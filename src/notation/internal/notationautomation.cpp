/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "notationautomation.h"

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "engraving/automation/iautomation.h"
#include "engraving/dom/masterscore.h"
#include "engraving/editing/undo.h"

using namespace mu::notation;

static constexpr mu::engraving::AutomationType AUTOMATION_VIEW_TYPE = mu::engraving::AutomationType::Expression;
static constexpr double AUTOMATION_VIEW_DEFAULT_VALUE = 0.5;

static double automationValueToLineY(double value)
{
    return 1.0 - std::clamp(value, 0.0, 1.0);
}

static double lineYToAutomationValue(double y)
{
    return 1.0 - std::clamp(static_cast<double>(y), 0.0, 1.0);
}

static mu::engraving::AutomationCurveKey automationKey(const mu::engraving::Staff* staff)
{
    mu::engraving::AutomationCurveKey key;
    key.type = AUTOMATION_VIEW_TYPE;
    key.staffId = staff ? staff->id() : muse::ID();
    return key;
}

static bool hasAutomationPointAtTick(const mu::engraving::IAutomation* automation,
                                     const mu::engraving::AutomationCurveKey& key, int tick)
{
    IF_ASSERT_FAILED(automation) {
        return false;
    }

    const mu::engraving::AutomationCurve& curve = automation->curve(key);
    return curve.find(tick) != curve.cend();
}

static mu::engraving::AutomationPoint automationPoint(double value)
{
    mu::engraving::AutomationPoint point;
    point.inValue = value;
    point.outValue = value;
    return point;
}

static bool automationPointsEqual(const mu::engraving::AutomationPoint& left,
                                  const mu::engraving::AutomationPoint& right)
{
    return muse::RealIsEqual(left.inValue, right.inValue)
           && muse::RealIsEqual(left.outValue, right.outValue)
           && left.interpolation == right.interpolation
           && left.itemId == right.itemId;
}

static bool automationCurvesEqual(const mu::engraving::AutomationCurve& left,
                                  const mu::engraving::AutomationCurve& right)
{
    if (left.size() != right.size()) {
        return false;
    }

    auto leftIt = left.cbegin();
    auto rightIt = right.cbegin();
    while (leftIt != left.cend()) {
        if (leftIt->first != rightIt->first || !automationPointsEqual(leftIt->second, rightIt->second)) {
            return false;
        }

        ++leftIt;
        ++rightIt;
    }

    return true;
}

static void replaceAutomationCurve(mu::engraving::IAutomation* automation,
                                   const mu::engraving::AutomationCurveKey& key,
                                   const mu::engraving::AutomationCurve& curve)
{
    IF_ASSERT_FAILED(automation) {
        return;
    }

    const mu::engraving::AutomationCurve currentCurve = automation->curve(key);
    for (const auto& point : currentCurve) {
        automation->removePoint(key, point.first);
    }

    for (const auto& [tick, point] : curve) {
        automation->addPoint(key, tick, point);
    }
}

class ChangeAutomationCurve : public mu::engraving::UndoCommand
{
public:
    ChangeAutomationCurve(mu::engraving::Score* score, mu::engraving::AutomationCurveKey key,
                          mu::engraving::AutomationCurve before, mu::engraving::AutomationCurve after)
        : m_score(score), m_key(std::move(key)), m_before(std::move(before)), m_after(std::move(after))
    {
    }

    const char* name() const override
    {
        return "ChangeAutomationCurve";
    }

protected:
    void flip(mu::engraving::EditData*) override
    {
        IF_ASSERT_FAILED(m_score && m_score->automation()) {
            return;
        }

        replaceAutomationCurve(m_score->automation(), m_key, m_atAfter ? m_before : m_after);
        m_atAfter = !m_atAfter;
    }

private:
    mu::engraving::Score* m_score = nullptr;
    mu::engraving::AutomationCurveKey m_key;
    mu::engraving::AutomationCurve m_before;
    mu::engraving::AutomationCurve m_after;
    bool m_atAfter = true;
};

static const Segment* firstAutomationSegmentForSystem(const System* system)
{
    IF_ASSERT_FAILED(system) {
        return nullptr;
    }

    const mu::engraving::SegmentType type = Segment::CHORD_REST_OR_TIME_TICK_TYPE;
    const Segment* seg = system->firstMeasure() ? system->firstMeasure()->first(type) : nullptr;
    while (seg && seg->system() != system) {
        seg = seg->next1(type);
    }
    return seg && seg->system() == system ? seg : nullptr;
}

struct TickCanvasPoint {
    int tick = 0;
    double canvasX = 0.0;
};

using TickCanvasMap = std::vector<TickCanvasPoint>;

static void appendTickCanvasPoint(TickCanvasMap& points, int tick, double canvasX)
{
    if (!points.empty() && tick == points.back().tick) {
        points.back().canvasX = canvasX;
        return;
    }

    if (points.empty() || tick > points.back().tick) {
        points.push_back({ tick, canvasX });
    }
}

static TickCanvasMap tickCanvasMapForSystem(const System* system, double staffStartCanvasX, double staffEndCanvasX,
                                            int startTick, int endTick)
{
    TickCanvasMap points;
    points.reserve(16);
    appendTickCanvasPoint(points, startTick, staffStartCanvasX);

    const mu::engraving::SegmentType type = Segment::CHORD_REST_OR_TIME_TICK_TYPE;
    const Segment* seg = firstAutomationSegmentForSystem(system);
    while (seg && seg->system() == system) {
        const double segStartCanvasX = seg->canvasX();
        const double segEndCanvasX = segStartCanvasX + seg->width();
        const int segStartTick = seg->tick().ticks();
        const int segEndTick = segStartTick + seg->ticks().ticks();

        appendTickCanvasPoint(points, segStartTick, segStartCanvasX);
        appendTickCanvasPoint(points, segEndTick, segEndCanvasX);

        seg = seg->next1(type);
    }

    appendTickCanvasPoint(points, endTick, staffEndCanvasX);
    return points;
}

static int interpolatedTick(double x, double leftX, double rightX, int leftTick, int rightTick)
{
    if (rightTick <= leftTick || muse::RealIsEqual(leftX, rightX)) {
        return leftTick;
    }

    const double ratio = std::clamp((x - leftX) / (rightX - leftX), 0.0, 1.0);
    return leftTick + static_cast<int>(ratio * (rightTick - leftTick));
}

static double interpolatedCanvasX(int tick, int leftTick, int rightTick, double leftX, double rightX)
{
    if (rightTick <= leftTick || muse::RealIsEqual(leftX, rightX)) {
        return leftX;
    }

    const double ratio = std::clamp(static_cast<double>(tick - leftTick) / static_cast<double>(rightTick - leftTick),
                                    0.0, 1.0);
    return leftX + (ratio * (rightX - leftX));
}

static int tickForCanvasX(const TickCanvasMap& tickCanvasMap, double canvasX, bool* ok)
{
    if (ok) {
        *ok = false;
    }

    IF_ASSERT_FAILED(!tickCanvasMap.empty()) {
        return 0;
    }

    auto right = std::lower_bound(tickCanvasMap.cbegin(), tickCanvasMap.cend(), canvasX,
                                  [](const TickCanvasPoint& point, double x) {
        return point.canvasX < x;
    });

    if (ok) {
        *ok = true;
    }

    if (right == tickCanvasMap.cbegin()) {
        return right->tick;
    }

    if (right == tickCanvasMap.cend()) {
        return tickCanvasMap.back().tick;
    }

    if (muse::RealIsEqual(right->canvasX, canvasX)) {
        return right->tick;
    }

    auto left = std::prev(right);
    return interpolatedTick(canvasX, left->canvasX, right->canvasX, left->tick, right->tick);
}

static double canvasXForTick(const TickCanvasMap& tickCanvasMap, int tick)
{
    IF_ASSERT_FAILED(!tickCanvasMap.empty()) {
        return 0.0;
    }

    auto right = std::lower_bound(tickCanvasMap.cbegin(), tickCanvasMap.cend(), tick,
                                  [](const TickCanvasPoint& point, int value) {
        return point.tick < value;
    });

    if (right == tickCanvasMap.cbegin()) {
        return right->canvasX;
    }

    if (right == tickCanvasMap.cend()) {
        return tickCanvasMap.back().canvasX;
    }

    if (right->tick == tick) {
        return right->canvasX;
    }

    auto left = std::prev(right);
    return interpolatedCanvasX(tick, left->tick, right->tick, left->canvasX, right->canvasX);
}

static bool hasAutomationViewCurve(const mu::engraving::Score* score)
{
    IF_ASSERT_FAILED(score && score->automation()) {
        return false;
    }

    for (const mu::engraving::Staff* staff : score->staves()) {
        if (!staff || !staff->isPrimaryStaff()) {
            continue;
        }

        if (!score->automation()->curve(automationKey(staff)).empty()) {
            return true;
        }
    }

    return false;
}

static void seedAutomationViewCurves(mu::engraving::Score* score)
{
    IF_ASSERT_FAILED(score && score->automation()) {
        return;
    }

    const int startTick = 0;
    const int endTick = score->lastMeasure() ? score->lastMeasure()->endTick().ticks() : startTick;

    for (const mu::engraving::Staff* staff : score->staves()) {
        if (!staff || !staff->isPrimaryStaff()) {
            continue;
        }

        const mu::engraving::AutomationCurveKey expressionKey = automationKey(staff);
        if (!score->automation()->curve(expressionKey).empty()) {
            continue;
        }

        score->automation()->addPoint(expressionKey, startTick, automationPoint(AUTOMATION_VIEW_DEFAULT_VALUE));
        if (endTick > startTick) {
            score->automation()->addPoint(expressionKey, endTick, automationPoint(AUTOMATION_VIEW_DEFAULT_VALUE));
        }
    }
}

NotationAutomation::NotationAutomation(IGetScore* getScore,
                                       muse::async::Channel<muse::RectF> notationChanged)
    : m_getScore(getScore), m_notationChanged(notationChanged)
{
}

bool NotationAutomation::isAutomationModeEnabled() const
{
    return m_isAutomationModeEnabled;
}

void NotationAutomation::setAutomationModeEnabled(bool enabled)
{
    if (m_isAutomationModeEnabled == enabled) {
        return;
    }

    if (enabled) {
        initAutomationLinesData();
    }

    m_isAutomationModeEnabled = enabled;
    m_automationModeEnabledChanged.notify();
}

muse::async::Notification NotationAutomation::automationModeEnabledChanged() const
{
    return m_automationModeEnabledChanged;
}

QVariant NotationAutomation::automationLinesData() const
{
    return m_automationLinesData;
}

void NotationAutomation::initAutomationLinesData()
{
    mu::engraving::MasterScore* masterScore = score() ? score()->masterScore() : nullptr;
    IF_ASSERT_FAILED(masterScore) {
        return;
    }

    if (m_automationLinesData.isEmpty() && !hasAutomationViewCurve(masterScore)) {
        masterScore->initAutomation();
        seedAutomationViewCurves(masterScore);
    }

    QVariantList automationLinesData;
    const std::vector<System*>& systems = score()->systems();
    for (size_t systemIdx = 0; systemIdx < systems.size(); ++systemIdx) {
        const QVariantList linesData = linesDataForSystem(systems.at(systemIdx), systemIdx);
        if (!linesData.empty()) {
            automationLinesData << linesData;
        }
    }
    m_automationLinesData = automationLinesData;
    m_automationLinesDataChanged.notify();
}

QVariantList NotationAutomation::linesDataForSystem(const System* system, size_t systemIdx) const
{
    QVariantList lines;

    staff_idx_t staffIdx = system->firstVisibleStaff();
    while (staffIdx != muse::nidx) {
        const QVariantMap lineData = lineDataForSysStaff(staffIdx, systemIdx, system);
        if (!lineData.isEmpty()) {
            lines << lineData;
        }

        staffIdx = system->nextVisibleStaff(staffIdx);
    }

    return lines;
}

QVariantMap NotationAutomation::lineDataForSysStaff(staff_idx_t staffIdx, size_t systemIdx, const System* system) const
{
    IF_ASSERT_FAILED(system && system->first() && system->last()) {
        return {};
    }

    const Staff* staff = score()->staff(staffIdx);
    const SysStaff* sysStaff = system->staff(staffIdx);
    IF_ASSERT_FAILED(staff && sysStaff) {
        return {};
    }

    if (!staff->isPrimaryStaff()) {
        return {};
    }

    const int systemStartTick = system->first()->tick().ticks();
    const int systemEndTick = system->last()->endTick().ticks();
    const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());
    const QVariantList staffLinesData = pointsDataForSysStaff(staff, system, staffCanvasRect,
                                                              systemStartTick, systemEndTick);
    if (staffLinesData.isEmpty()) {
        return {};
    }

    QVariantMap lineData;
    lineData["staffIdx"] = static_cast<int>(staffIdx);
    lineData["systemIdx"] = static_cast<qulonglong>(systemIdx);
    lineData["x"] = staffCanvasRect.x();
    lineData["y"] = staffCanvasRect.y();
    lineData["width"] = staffCanvasRect.width();
    lineData["height"] = staffCanvasRect.height();
    lineData["startTick"] = systemStartTick;
    lineData["endTick"] = systemEndTick;
    lineData["points"] = staffLinesData;

    return lineData;
}

QVariantList NotationAutomation::pointsDataForSysStaff(const Staff* staff, const System* system,
                                                       const muse::RectF& sysStaffCanvasRect, int startTick,
                                                       int endTick) const
{
    QVariantList points;
    const auto addPoint = [&points](double x, double y, int tick, PointType pointType) {
        QVariantMap pointData;
        pointData["x"] = x;
        pointData["y"] = y;
        pointData["tick"] = tick;
        pointData["pointType"] = static_cast<int>(pointType);
        points << pointData;
    };

    IF_ASSERT_FAILED(automation()) {
        return points;
    }

    const mu::engraving::AutomationCurveKey key = automationKey(staff);
    const mu::engraving::AutomationCurve& curve = automation()->curve(key);
    const TickCanvasMap tickCanvasMap = tickCanvasMapForSystem(system, sysStaffCanvasRect.x(),
                                                               sysStaffCanvasRect.x() + sysStaffCanvasRect.width(),
                                                               startTick, endTick);

    const mu::engraving::AutomationPoint& startPoint = automation()->activePoint(key, startTick);
    addPoint(0.0, automationValueToLineY(startPoint.outValue), startTick, PointType::BOTH);

    for (const auto& point : curve) {
        const int tick = point.first;
        if (tick <= startTick || tick >= endTick) {
            continue;
        }

        const double pointCanvasX = canvasXForTick(tickCanvasMap, tick);
        const double pointXInStaff = (pointCanvasX - sysStaffCanvasRect.x()) / sysStaffCanvasRect.width();

        // Point in/out values are always between 0 and 1 - higher value == lower Y...
        const mu::engraving::AutomationPoint& autoPoint = point.second;
        if (muse::RealIsEqual(autoPoint.inValue, autoPoint.outValue)) {
            addPoint(pointXInStaff, automationValueToLineY(autoPoint.inValue), tick, PointType::BOTH);
            continue;
        }
        addPoint(pointXInStaff, automationValueToLineY(autoPoint.inValue), tick, PointType::IN);
        addPoint(pointXInStaff, automationValueToLineY(autoPoint.outValue), tick, PointType::OUT);
    }

    if (endTick > startTick) {
        const mu::engraving::AutomationPoint& endPoint = automation()->activePoint(key, endTick);
        addPoint(1.0, automationValueToLineY(endPoint.outValue), endTick, PointType::BOTH);
    }

    return points;
}

muse::async::Notification NotationAutomation::automationLinesDataChanged() const
{
    return m_automationLinesDataChanged;
}

mu::engraving::Score* NotationAutomation::score() const
{
    return m_getScore ? m_getScore->score() : nullptr;
}

mu::engraving::IAutomation* NotationAutomation::automation() const
{
    return score() ? score()->automation() : nullptr;
}

void NotationAutomation::refreshAutomationView()
{
    m_automationLinesData.clear();
    initAutomationLinesData();
    m_notationChanged.send(muse::RectF());
}

bool NotationAutomation::refreshAutomationLinesForStaff(staff_idx_t staffIdx)
{
    const std::vector<System*>& systems = score()->systems();
    bool refreshed = false;

    for (qsizetype lineIdx = 0; lineIdx < m_automationLinesData.size(); ++lineIdx) {
        const QVariantMap lineData = m_automationLinesData.at(lineIdx).toMap();
        bool ok = false;
        const staff_idx_t lineStaffIdx = static_cast<staff_idx_t>(lineData.value("staffIdx").toInt(&ok));
        if (!ok || lineStaffIdx != staffIdx) {
            continue;
        }

        const qulonglong systemIdxRaw = lineData.value("systemIdx").toULongLong(&ok);
        if (!ok || systemIdxRaw >= systems.size()) {
            return false;
        }

        const QVariantMap refreshedLineData = lineDataForSysStaff(staffIdx, static_cast<size_t>(systemIdxRaw),
                                                                  systems.at(static_cast<size_t>(systemIdxRaw)));
        if (refreshedLineData.isEmpty()) {
            return false;
        }

        m_automationLinesData.replace(lineIdx, refreshedLineData);
        refreshed = true;
    }

    if (!refreshed) {
        return false;
    }

    m_automationLinesDataChanged.notify();
    m_notationChanged.send(muse::RectF());
    return true;
}

void NotationAutomation::notifyAutomationChanged(staff_idx_t staffIdx)
{
    mu::engraving::Score* s = score();
    IF_ASSERT_FAILED(s) {
        return;
    }

    if (!refreshAutomationLinesForStaff(staffIdx)) {
        refreshAutomationView();
    }

    mu::engraving::ScoreChanges changes;
    changes.tickFrom = 0;
    changes.tickTo = s->lastMeasure() ? s->lastMeasure()->endTick().ticks() : 0;
    changes.staffIdxFrom = staffIdx;
    changes.staffIdxTo = staffIdx + 1;
    s->changesChannel().send(changes);
}

int NotationAutomation::tickForLineX(const QVariantMap& lineData, staff_idx_t staffIdx, qreal x, bool* ok)
{
    if (ok) {
        *ok = false;
    }

    bool dataOk = false;
    const int startTick = lineData.value("startTick").toInt(&dataOk);
    IF_ASSERT_FAILED(dataOk) {
        return 0;
    }

    const int endTick = lineData.value("endTick").toInt(&dataOk);
    IF_ASSERT_FAILED(dataOk) {
        return 0;
    }

    const qulonglong systemIdxRaw = lineData.value("systemIdx").toULongLong(&dataOk);
    IF_ASSERT_FAILED(dataOk) {
        return 0;
    }

    const std::vector<System*>& systems = score()->systems();
    IF_ASSERT_FAILED(systemIdxRaw < systems.size()) {
        refreshAutomationView();
        return 0;
    }

    const System* system = systems.at(static_cast<size_t>(systemIdxRaw));
    const SysStaff* sysStaff = system ? system->staff(staffIdx) : nullptr;
    IF_ASSERT_FAILED(sysStaff) {
        refreshAutomationView();
        return 0;
    }

    const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());
    const double staffStartCanvasX = staffCanvasRect.x();
    const double staffEndCanvasX = staffStartCanvasX + staffCanvasRect.width();
    const double pointCanvasX = staffStartCanvasX
                                + std::clamp(static_cast<double>(x), 0.0, 1.0) * (staffEndCanvasX - staffStartCanvasX);
    const TickCanvasMap tickCanvasMap = tickCanvasMapForSystem(system, staffStartCanvasX, staffEndCanvasX,
                                                               startTick, endTick);

    bool tickOk = false;
    const int tick = tickForCanvasX(tickCanvasMap, pointCanvasX, &tickOk);
    if (ok) {
        *ok = tickOk;
    }
    return tick;
}

void NotationAutomation::requestChangeAutomationPoint(qsizetype lineIdx, qsizetype pointIdx, qreal x, qreal y)
{
    IF_ASSERT_FAILED(automation() && lineIdx < m_automationLinesData.size()) {
        return;
    }

    const QVariantMap lineData = m_automationLinesData.at(lineIdx).toMap();
    IF_ASSERT_FAILED(!lineData.isEmpty()) {
        return;
    }

    bool ok = false;
    const staff_idx_t staffIdx = static_cast<staff_idx_t>(lineData.value("staffIdx").toInt(&ok));
    IF_ASSERT_FAILED(ok && staffIdx != muse::nidx) {
        return;
    }

    const Staff* staff = score()->staff(staffIdx);
    const QVariantList pointsList = lineData.value("points").toList();
    IF_ASSERT_FAILED(staff && !pointsList.isEmpty() && pointIdx < pointsList.size()) {
        return;
    }

    const QVariantMap point = pointsList.at(pointIdx).toMap();
    IF_ASSERT_FAILED(!point.isEmpty()) {
        return;
    }

    const int oldTick = point.value("tick").toInt(&ok); // "Old" because we might change it given a new x value...
    const int pointTypeRaw = ok ? point.value("pointType").toInt(&ok) : 0;
    IF_ASSERT_FAILED(ok) {
        return;
    }

    const PointType pointType = static_cast<PointType>(pointTypeRaw);
    const mu::engraving::AutomationCurveKey key = automationKey(staff);
    const mu::engraving::AutomationCurve beforeChange = automation()->curve(key);
    score()->startCmd(muse::TranslatableString("undoableAction", "Edit automation"));

    auto finishChange = [this, staffIdx, key, beforeChange]() {
        const mu::engraving::AutomationCurve afterChange = automation()->curve(key);
        if (!automationCurvesEqual(afterChange, beforeChange)) {
            score()->undoStack()->pushWithoutPerforming(new ChangeAutomationCurve(score(), key, beforeChange,
                                                                                  afterChange));
        }

        score()->endCmd();
        notifyAutomationChanged(staffIdx);
    };

    const double newValue = lineYToAutomationValue(y);

    bool tickOk = false;
    const int newTick = tickForLineX(lineData, staffIdx, x, &tickOk);
    if (!tickOk) {
        score()->endCmd(true);
        return;
    }
    const bool pointExistsAtOldTick = hasAutomationPointAtTick(automation(), key, oldTick);

    if (!pointExistsAtOldTick) {
        const mu::engraving::AutomationPoint& activePoint = automation()->activePoint(key, oldTick);
        automation()->addPoint(key, oldTick, automationPoint(activePoint.outValue));
    }

    if (pointType == PointType::IN || pointType == PointType::BOTH) {
        automation()->setPointInValue(key, oldTick, newValue);
    }
    if (pointType == PointType::OUT || pointType == PointType::BOTH) {
        automation()->setPointOutValue(key, oldTick, newValue);
    }

    if (newTick == oldTick) {
        finishChange();
        return;
    }

    if (hasAutomationPointAtTick(automation(), key, newTick)) {
        finishChange();
        return;
    }

    //! NOTE: Moving a BOTH point is the simplest case - we can simply change the tick. Moving IN/OUT points is slightly
    //! more complex. In this case we need to set the in/out values to be equal at oldTick (effectively converting the
    //! original point to a BOTH point) and create a new point at newTick...

    if (pointType == PointType::BOTH) {
        automation()->movePoint(key, oldTick, newTick);
        finishChange();
        return;
    }

    const mu::engraving::AutomationPoint& oldPoint = automation()->activePoint(key, oldTick);
    const mu::engraving::AutomationPoint newPoint = automationPoint(newValue);
    if (pointType == PointType::IN) {
        automation()->setPointInValue(key, oldTick, oldPoint.outValue);
    }
    if (pointType == PointType::OUT) {
        automation()->setPointOutValue(key, oldTick, oldPoint.inValue);
    }
    automation()->addPoint(key, newTick, newPoint);
    finishChange();
}

void NotationAutomation::requestAddAutomationPoint(qsizetype lineIdx, qreal x, qreal y)
{
    IF_ASSERT_FAILED(automation() && lineIdx < m_automationLinesData.size()) {
        return;
    }

    const QVariantMap lineData = m_automationLinesData.at(lineIdx).toMap();
    IF_ASSERT_FAILED(!lineData.isEmpty()) {
        return;
    }

    bool ok = false;
    const staff_idx_t staffIdx = static_cast<staff_idx_t>(lineData.value("staffIdx").toInt(&ok));
    IF_ASSERT_FAILED(ok && staffIdx != muse::nidx) {
        return;
    }

    const Staff* staff = score()->staff(staffIdx);
    IF_ASSERT_FAILED(staff) {
        return;
    }

    bool tickOk = false;
    const int tick = tickForLineX(lineData, staffIdx, x, &tickOk);
    if (!tickOk) {
        return;
    }

    const mu::engraving::AutomationCurveKey key = automationKey(staff);
    const double newValue = lineYToAutomationValue(y);
    const mu::engraving::AutomationCurve beforeChange = automation()->curve(key);
    score()->startCmd(muse::TranslatableString("undoableAction", "Edit automation"));

    if (!hasAutomationPointAtTick(automation(), key, tick)) {
        automation()->addPoint(key, tick, automationPoint(newValue));
    } else {
        automation()->setPointInValue(key, tick, newValue);
        automation()->setPointOutValue(key, tick, newValue);
    }

    const mu::engraving::AutomationCurve afterChange = automation()->curve(key);
    if (!automationCurvesEqual(afterChange, beforeChange)) {
        score()->undoStack()->pushWithoutPerforming(new ChangeAutomationCurve(score(), key, beforeChange,
                                                                              afterChange));
    }

    score()->endCmd();
    notifyAutomationChanged(staffIdx);
}

void NotationAutomation::requestRemoveAutomationPoint(qsizetype lineIdx, qsizetype pointIdx)
{
    IF_ASSERT_FAILED(automation() && lineIdx < m_automationLinesData.size()) {
        return;
    }

    const QVariantMap lineData = m_automationLinesData.at(lineIdx).toMap();
    IF_ASSERT_FAILED(!lineData.isEmpty()) {
        return;
    }

    bool ok = false;
    const staff_idx_t staffIdx = static_cast<staff_idx_t>(lineData.value("staffIdx").toInt(&ok));
    IF_ASSERT_FAILED(ok && staffIdx != muse::nidx) {
        return;
    }

    const Staff* staff = score()->staff(staffIdx);
    const QVariantList pointsList = lineData.value("points").toList();
    IF_ASSERT_FAILED(staff && !pointsList.isEmpty() && pointIdx < pointsList.size()) {
        return;
    }

    const QVariantMap point = pointsList.at(pointIdx).toMap();
    IF_ASSERT_FAILED(!point.isEmpty()) {
        return;
    }

    const int tick = point.value("tick").toInt(&ok);
    IF_ASSERT_FAILED(ok) {
        return;
    }

    const mu::engraving::AutomationCurveKey key = automationKey(staff);
    if (!hasAutomationPointAtTick(automation(), key, tick)) {
        refreshAutomationView();
        return;
    }

    const mu::engraving::AutomationCurve beforeChange = automation()->curve(key);
    score()->startCmd(muse::TranslatableString("undoableAction", "Edit automation"));
    automation()->removePoint(key, tick);

    const mu::engraving::AutomationCurve afterChange = automation()->curve(key);
    if (!automationCurvesEqual(afterChange, beforeChange)) {
        score()->undoStack()->pushWithoutPerforming(new ChangeAutomationCurve(score(), key, beforeChange,
                                                                              afterChange));
    }

    score()->endCmd();
    notifyAutomationChanged(staffIdx);
}
