// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#ifndef NO_QT_SUPPORT
#include <QString>
#include <QRectF>
#include <QVariantList>
#include <vector>
#include <memory>
#include "image.h"
namespace mu::engraving {
class Score;
class Staff;
namespace SvgDrawing {
class MeasureInsertionState;
class MeasureInsertionBatch;
std::shared_ptr<MeasureInsertionBatch> batchMeasureInsertion(Score* score);
std::shared_ptr<MeasureInsertionState> prepareMeasureInsertion(Score* score, const Fraction& tick, const Fraction& length);
std::vector<Staff*> linkedStaves(const Image* image);
staff_idx_t excerptSourceStaff(const Image* image, const std::vector<staff_idx_t>& sourceStaves);
std::string serializedData(const Image* image);
std::string serializedPart(const Image* image);
QString insert(Score* score, const QString& sourceDocument, const QString& mode, const QRectF& sourceBounds, const RectF& canvasBounds,
               bool onScore, bool ownTransaction = true, bool reuseFragments = false);
QString syncNative(Score* score, const QString& document);
QString nativeDocument(Score* score);
QVariantList nativeFragments(Score* score, const QString& previewDocument = {});
void setEditing(Score* score, bool active);
QString selectedDocument(Score* score);
QVariantList anchors(Score* score, int subdivision);
QVariantList editingPoints(Score* score, const Image* image = nullptr);
bool layout(const Image* image, Image::LayoutData* data);
}
}
#endif
