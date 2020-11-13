//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <vector>
#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "mscore/musescore.h"
#include "mscore/workspace.h"
#include "mscore/palette/palettemodel.h"
#include "mscore/palette/palettetree.h"

using namespace Ms;

class TestPaletteModel : public QObject, public MTest
      {
      Q_OBJECT

      PaletteTreeModel* paletteModel;
      QMap<QString, std::vector<QString>> paletteItemNames;

      void initMuseScore();
      void iterateOverModel(QAbstractItemModel *model, QModelIndex parent = QModelIndex());
      void loadPaletteModel(QString name);

   private slots:
      void initTestCase();
      void cleanupTestCase();

      void testDuplicateItemNames_data();
      void testDuplicateItemNames();
      };

//---------------------------------------------------------
//   TestPaletteModel::testDuplicateItemNames_data
//---------------------------------------------------------

void TestPaletteModel::testDuplicateItemNames_data()
      {
      QTest::addColumn<QString>("workspaceName");
      QTest::addRow("Basic") << "Basic";
      QTest::addRow("Advanced") << "Advanced";
      }

//---------------------------------------------------------
//   TestPaletteModel::testDuplicateItemNames
//---------------------------------------------------------

void TestPaletteModel::testDuplicateItemNames()
      {
      QFETCH(QString, workspaceName);
      loadPaletteModel(workspaceName);
      paletteItemNames.clear();
      iterateOverModel(paletteModel);
      bool duplicates = false;
      qDebug("In %s workspace", qPrintable(workspaceName));
      for (auto name = paletteItemNames.begin(); name != paletteItemNames.end(); ++name) {
            if (name.value().size() != 1) {
                  // Exceptions - allowed duplicates
                  if (name.key().endsWith(" repeat sign") || // repeat barlines in "Barlines" and "Repeats & Jumps" palette
                      //name.key().startsWith("To Coda") || // 2 different "To Coda" in "Repeats & Jumps" palette (so far Master palette only)
                      name.key() == "Open" || // articulations in "Articulations" and channel switch text in "Text" palette
                      name.key() == "Line" || // bracket type in "Brackets" and line type in "Lines" palette
                      //name.key() == "Caesura" || // 2 different Caesuras in the "Breaths & Pauses" palette (so far Master palette only)
                      name.key().startsWith("Add parentheses to ") // "Noteheads" and "Accidentals" palette
                      )
                        continue;
                  duplicates = true;
                  for (auto parent : name.value())
                        qDebug("%s (in %s)", qPrintable(name.key()), qPrintable(parent));
                  }
            }
      // make sure there are no duplicates
      QVERIFY(!duplicates);
      }

//---------------------------------------------------------
//   TestPaletteModel::iterateOverModel
//---------------------------------------------------------

void TestPaletteModel::iterateOverModel(QAbstractItemModel* model, QModelIndex parent)
      {
      for (int r = 0; r < model->rowCount(parent); ++r) {
            QModelIndex index = model->index(r, 0, parent);
            QString name = model->data(index, Qt::AccessibleTextRole).toString();
            QString parentName = model->data(parent, Qt::AccessibleTextRole).toString();

            paletteItemNames[name].push_back(parentName);

            if (model->hasChildren(index))
                  iterateOverModel(model, index);
            }
      }

//---------------------------------------------------------
//   TestPaletteModel::initTestCase
//---------------------------------------------------------

void TestPaletteModel::initTestCase()
      {
      initMuseScore();
      }

//---------------------------------------------------------
//   TestPaletteModel::loadPaletteModel
//---------------------------------------------------------

void TestPaletteModel::loadPaletteModel(QString name)
      {
      Workspace* w = WorkspacesManager::findByName(name);
      paletteModel = new PaletteTreeModel(w->getPaletteTree());
      }

//---------------------------------------------------------
//   TestPaletteModel::initMuseScore
//---------------------------------------------------------

void TestPaletteModel::initMuseScore()
      {
      qputenv("QML_DISABLE_DISK_CACHE", "true");
      qSetMessagePattern("%{function}: %{message}");
      MScore::noGui = true;
      MScore::testMode = true;
      initMuseScoreResources();
      QStringList temp;
      MuseScore::init(temp);
      }

//---------------------------------------------------------
//   TestPaletteModel::cleanupTestCase
//---------------------------------------------------------

void TestPaletteModel::cleanupTestCase()
      {
      qApp->processEvents();
      delete Ms::mscore;
      Ms::mscore = nullptr;
      }

QTEST_MAIN(TestPaletteModel)
#include "tst_palette.moc"
