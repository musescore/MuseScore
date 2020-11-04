//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "mscore/musescore.h"
#include "mscore/palette/paletteworkspace.h"
#include "mscore/workspace.h"

#define DIR QString("mscore/workspaces/")

namespace Ms {

//---------------------------------------------------------
//   TestWorkspaces
//---------------------------------------------------------

class TestWorkspaces : public QObject
      {
      Q_OBJECT

      QString basicWorkspaceRefXml = "basic_ref.xml";
      QString advancedWorkspaceRefXml = "advanced_ref.xml";

      std::unique_ptr<QTemporaryDir> tmpDataDir = nullptr;

      void prepareStandardWorkspaceXml(const QString& name, const QString& refXml);
      void prepareReferenceWorkspacesXml();

      void prepareStandardWorkspacesTestData();

      void setFirstStartWorkspace(const QString& trName);

      QAbstractItemModel* editPaletteTreeModel() const;
      QModelIndex editPaletteIndex() const;
      void editPalette();

      void initMuseScore();
      void cleanupMuseScore();
      void restartMuseScore();

   private slots:
      void initTestCase();
      void init();
      void cleanup();

      // tests on standard workspaces customization
      void testEditPalette_data() { prepareStandardWorkspacesTestData(); }
      void testEditPalette();
      void testResetEditedPalette_data() { prepareStandardWorkspacesTestData(); }
      void testResetEditedPalette();
      void testResetEditedWorkspace_data() { prepareStandardWorkspacesTestData(); }
      void testResetEditedWorkspace();
      void testDeleteEditedWorkspace_data() { prepareStandardWorkspacesTestData(); }
      void testDeleteEditedWorkspace();

      // tests on custom workspaces
      void testCreateNewWorkspace_data();
      void testCreateNewWorkspace();
      };

void TestWorkspaces::initTestCase()
      {
      qputenv("QML_DISABLE_DISK_CACHE", "true");
      qSetMessagePattern("%{function}: %{message}");
      // Force INI settings format to store settings in a temporary directory
      QSettings::setDefaultFormat(QSettings::IniFormat);
      MScore::noGui = true;
      MScore::testMode = true;
      initMuseScoreResources();

      qRegisterMetaType<QAbstractItemModel*>();

      prepareReferenceWorkspacesXml();
      }

void TestWorkspaces::initMuseScore()
      {
      MuseScoreApplication::setCustomConfigFolder(tmpDataDir->path());
      QStringList temp;
      MuseScore::init(temp);
      WorkspacesManager::initCurrentWorkspace();
      WorkspacesManager::currentWorkspace()->read();
      qApp->processEvents();
      }

void TestWorkspaces::cleanupMuseScore()
      {
      WorkspacesManager::clearWorkspaces();
      qApp->processEvents();
      delete Ms::mscore;
      Ms::mscore = nullptr;
      }

void TestWorkspaces::restartMuseScore()
      {
      cleanupMuseScore();
      initMuseScore();
      }

void TestWorkspaces::init()
      {
      tmpDataDir.reset(new QTemporaryDir());
      initMuseScore();
      }

void TestWorkspaces::cleanup()
      {
      cleanupMuseScore();
      tmpDataDir.reset();
      }

static QString forceSaveWorkspace(Workspace* w)
      {
      if (!w->readOnly()) {
            w->write();
            return w->path();
            }

      const QString name = w->translatableName().isEmpty() ? w->name() : w->translatableName();
      const QString workspacePath = name + ".workspace";
      w->setPath(workspacePath);
      w->setReadOnly(false);
      w->write();
      return workspacePath;
      }

void TestWorkspaces::prepareStandardWorkspaceXml(const QString& name, const QString& refXml)
      {
      setFirstStartWorkspace(name);
      Workspace* w = WorkspacesManager::currentWorkspace();
      const QString workspacePath = forceSaveWorkspace(w);
      MTest::extractRootFile(workspacePath, refXml);
      }

void TestWorkspaces::prepareReferenceWorkspacesXml()
      {
      init();

      // source Basic.xml and Advanced.xml cannot be used as test
      // references as resaving would change them largely
      prepareStandardWorkspaceXml("Basic", basicWorkspaceRefXml);
      prepareStandardWorkspaceXml("Advanced", advancedWorkspaceRefXml);

      cleanup();
      }

void TestWorkspaces::setFirstStartWorkspace(const QString& trName)
      {
      Workspace* w = WorkspacesManager::findByTranslatableName(trName);
      Ms::mscore->changeWorkspace(w, /* firstStart */ true);
      }

QAbstractItemModel* TestWorkspaces::editPaletteTreeModel() const
      {
      PaletteWorkspace* pw = Ms::mscore->getPaletteWorkspace();
      QAbstractItemModel* paletteModel = pw->property("mainPaletteModel").value<QAbstractItemModel*>();
      Q_ASSERT(paletteModel);
      return paletteModel;
      }

QModelIndex TestWorkspaces::editPaletteIndex() const
      {
      // find and return the first palette
      return editPaletteTreeModel()->index(0, 0);
      }

void TestWorkspaces::editPalette()
      {
      // hide the first cell in the edited palette
      QAbstractItemModel* paletteModel = editPaletteTreeModel();
      const QModelIndex paletteIndex = editPaletteIndex();
      const QModelIndex cellIndex = paletteModel->index(0, 0, paletteIndex);
      paletteModel->setData(cellIndex, false, PaletteTreeModel::VisibleRole);

      qApp->processEvents();
      }

void TestWorkspaces::prepareStandardWorkspacesTestData()
      {
      QTest::addColumn<QString>("workspaceName");
      QTest::addColumn<QString>("editedWorkspaceName");
      QTest::addColumn<QString>("workspaceRefXml");
      QTest::addColumn<bool>("restart");

      QTest::addRow("Basic") << "Basic" << "Basic edited" << basicWorkspaceRefXml << false;
      QTest::addRow("Basic_restart") << "Basic" << "Basic edited" << basicWorkspaceRefXml << true;
      QTest::addRow("Advanced") << "Advanced" << "Advanced edited" << advancedWorkspaceRefXml << false;
      QTest::addRow("Advanced_restart") << "Advanced" << "Advanced edited" << advancedWorkspaceRefXml << true;
      }

void TestWorkspaces::testEditPalette()
      {
      QFETCH(QString, workspaceName);
      QFETCH(QString, editedWorkspaceName);
      QFETCH(bool, restart);

      setFirstStartWorkspace(workspaceName);
      editPalette();

      if (restart)
            restartMuseScore();

      const auto workspaces = WorkspacesManager::visibleWorkspaces();

      QCOMPARE(workspaces.size(), 2); // e.g. "Basic" and "Advanced edited"
      QCOMPARE(WorkspacesManager::currentWorkspace()->translatableName(), editedWorkspaceName); // workspace has renamed

      const QString expectedFirstWorkspaceName = (workspaceName == "Basic") ? editedWorkspaceName : "Basic";
      QEXPECT_FAIL("Basic_restart", "\"Basic edited\" workspace doesn't show first in the list after MuseScore restart. Needs to be fixed.", Continue);
      QCOMPARE(workspaces[0]->translatableName(), expectedFirstWorkspaceName); // "Basic" or "Basic edited" is the first item in workspaces list
      }

void TestWorkspaces::testResetEditedPalette()
      {
      QFETCH(QString, workspaceName);
      QFETCH(QString, editedWorkspaceName);
      QFETCH(QString, workspaceRefXml);
      QFETCH(bool, restart);

      setFirstStartWorkspace(workspaceName);
      editPalette();

      if (restart)
            restartMuseScore();

      // TODO: check the edited palette content?
      QCOMPARE(WorkspacesManager::currentWorkspace()->translatableName(), editedWorkspaceName);

      PaletteWorkspace* pw = Ms::mscore->getPaletteWorkspace();
      pw->resetPalette(editPaletteIndex());

      Workspace* curr = WorkspacesManager::currentWorkspace();
      QCOMPARE(curr->translatableName(), editedWorkspaceName);
      curr->save();

      // verify workspace content
      const QString currXml = QString(QTest::currentTestFunction()) + "_curr.xml";
      MTest::extractRootFile(curr->path(), currXml);
      QVERIFY(MTest::compareFilesFromPaths(currXml, workspaceRefXml));
      }

void TestWorkspaces::testResetEditedWorkspace()
      {
      QFETCH(QString, workspaceName);
      QFETCH(QString, editedWorkspaceName);
      QFETCH(QString, workspaceRefXml);
      QFETCH(bool, restart);

      setFirstStartWorkspace(workspaceName);
      editPalette();

      if (restart)
            restartMuseScore();

      // TODO: check the edited palette content?
      QCOMPARE(WorkspacesManager::currentWorkspace()->translatableName(), editedWorkspaceName);

      Ms::mscore->resetWorkspace();

      Workspace* curr = WorkspacesManager::currentWorkspace();
      QCOMPARE(curr->translatableName(), workspaceName);

      // check actual workspace content
      const QString currentWorkspacePath = forceSaveWorkspace(curr);
      const QString currXml = QString(QTest::currentTestFunction()) + "_curr.xml";
      MTest::extractRootFile(currentWorkspacePath, currXml);
      QVERIFY(MTest::compareFilesFromPaths(currXml, workspaceRefXml));
      }

void TestWorkspaces::testDeleteEditedWorkspace()
      {
      QSKIP("Fails if calling restartMuseScore(). Need to check this test after fixing issue #296408 (and write the one covering the issue).");

      QFETCH(QString, workspaceName);
      QFETCH(QString, editedWorkspaceName);
      QFETCH(QString, workspaceRefXml);
      QFETCH(bool, restart);

      setFirstStartWorkspace(workspaceName);
      editPalette();

      if (restart)
            restartMuseScore();

      // TODO: check the edited palette content?
      QVERIFY(WorkspacesManager::currentWorkspace()->translatableName() == editedWorkspaceName);

      Ms::mscore->deleteWorkspace();

      const auto workspaces = WorkspacesManager::visibleWorkspaces();
      QCOMPARE(workspaces.size(), 2); // "Basic" and "Advanced"
      QCOMPARE(workspaces[0]->translatableName(), QString("Basic"));
      QCOMPARE(workspaces[1]->translatableName(), QString("Advanced"));

      Workspace* curr = WorkspacesManager::currentWorkspace();
      QVERIFY(curr == workspaces[0]); // after deleting a workspace current workspace should switch to Basic

      // check actual workspace content
      const QString currentWorkspacePath = forceSaveWorkspace(curr);
      const QString currXml = QString(QTest::currentTestFunction()) + "_curr.xml";
      MTest::extractRootFile(currentWorkspacePath, currXml);
      QVERIFY(MTest::compareFilesFromPaths(currXml, workspaceRefXml));
      }

void TestWorkspaces::testCreateNewWorkspace_data()
      {
      QTest::addColumn<QString>("baseWorkspace");

      QTest::newRow("Basic") << "Basic";
      QTest::newRow("Advanced") << "Advanced";
      }

void TestWorkspaces::testCreateNewWorkspace()
      {
      QFETCH(QString, baseWorkspace);

      setFirstStartWorkspace(baseWorkspace);

      const QString newWorkspaceName = "test";
      Workspace* w = WorkspacesManager::createNewWorkspace(newWorkspaceName);

      const auto workspaces = WorkspacesManager::visibleWorkspaces();
      QCOMPARE(workspaces.size(), 3);
      QCOMPARE(workspaces[0]->translatableName(), QString("Basic"));
      QCOMPARE(workspaces[1]->translatableName(), QString("Advanced"));
      QCOMPARE(workspaces[2]->name(), newWorkspaceName);

      w->save();
      const QString workspacePath = w->path();
      QVERIFY(workspacePath != workspaces[0]->path());
      QVERIFY(workspacePath != workspaces[1]->path());

      QString sourceTagValue;
      bool sourceTagUnique = true;

      WorkspacesManager::readWorkspaceFile(workspacePath, [&](XmlReader& e){
            while (e.readNextStartElement()) {
                  if (e.name() == "source") {
                        const QString val = e.readElementText();
                        if (sourceTagValue.isEmpty())
                              sourceTagValue = val;
                        else
                              sourceTagUnique = false;
                        }
                  else
                        e.skipCurrentElement();
                  }
            });

      QVERIFY(sourceTagUnique);
      QCOMPARE(sourceTagValue, baseWorkspace);
      }
} // namespace Ms

QTEST_MAIN(Ms::TestWorkspaces)
#include "tst_workspaces.moc"
