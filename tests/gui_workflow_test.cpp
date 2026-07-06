#include "modalith_controller.hpp"

#include <gtest/gtest.h>

#include <QFile>
#include <QDir>
#include <QTemporaryDir>
#include <QUrl>

TEST(SurfaceEditor, UndoRedoRestoresPrescriptionValues) {
  SurfaceTableModel model;
  ASSERT_EQ(model.rowCount(), 3);
  EXPECT_TRUE(model.setCell(0, SurfaceTableModel::Radius, 62.5));
  EXPECT_DOUBLE_EQ(model.data(model.index(0, SurfaceTableModel::Radius), Qt::DisplayRole).toDouble(),
                   62.5);
  EXPECT_TRUE(model.canUndo());

  model.undo();
  EXPECT_DOUBLE_EQ(model.data(model.index(0, SurfaceTableModel::Radius), Qt::DisplayRole).toDouble(),
                   50.0);
  EXPECT_TRUE(model.canRedo());

  model.redo();
  EXPECT_DOUBLE_EQ(model.data(model.index(0, SurfaceTableModel::Radius), Qt::DisplayRole).toDouble(),
                   62.5);
}

TEST(SurfaceEditor, DuplicateAndUndoAreAtomic) {
  SurfaceTableModel model;
  model.duplicateSurface(0);
  ASSERT_EQ(model.rowCount(), 4);
  EXPECT_EQ(model.data(model.index(1, SurfaceTableModel::Glass), Qt::DisplayRole).toString(),
            "N-BK7");
  model.undo();
  EXPECT_EQ(model.rowCount(), 3);
}

TEST(ProjectWorkflow, NativeProjectRoundTripsWithoutLosingEngineeringData) {
  QTemporaryDir directory(QDir::current().filePath("modalith-project-test-XXXXXX"));
  ASSERT_TRUE(directory.isValid());
  const QString path = directory.filePath("roundtrip.modalith");

  ModalithController controller;
  controller.setSystemTitle("Round-trip singlet");
  controller.setTemperatureC(35.0);
  controller.setFieldY(4.5);
  controller.setWavelengthText("532, 632.8");
  ASSERT_TRUE(controller.surfaceModel()->setCell(0, SurfaceTableModel::Radius, 48.25));
  ASSERT_TRUE(controller.saveProject(QUrl::fromLocalFile(path))) << controller.statusText().toStdString();
  EXPECT_FALSE(controller.modified());

  controller.newProject();
  ASSERT_TRUE(controller.openProject(QUrl::fromLocalFile(path))) << controller.statusText().toStdString();
  EXPECT_EQ(controller.systemTitle(), "Round-trip singlet");
  EXPECT_DOUBLE_EQ(controller.temperatureC(), 35.0);
  EXPECT_DOUBLE_EQ(controller.fieldY(), 4.5);
  EXPECT_EQ(controller.wavelengthCount(), 2);
  EXPECT_DOUBLE_EQ(controller.surfaceModel()
                       ->data(controller.surfaceModel()->index(0, SurfaceTableModel::Radius),
                              Qt::DisplayRole).toDouble(),
                   48.25);
  EXPECT_FALSE(controller.modified());
}

TEST(ProjectWorkflow, AnalysisExportsSpotAndRayFanCsv) {
  QTemporaryDir directory(QDir::current().filePath("modalith-export-test-XXXXXX"));
  ASSERT_TRUE(directory.isValid());
  const QString path = directory.filePath("analysis.csv");
  ModalithController controller;
  controller.analyze();
  ASSERT_TRUE(controller.exportAnalysisCsv(QUrl::fromLocalFile(path))) << controller.statusText().toStdString();

  QFile file(path);
  ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
  const QByteArray contents = file.readAll();
  EXPECT_TRUE(contents.contains("analysis,x_mm,y_mm"));
  EXPECT_TRUE(contents.contains("spot,"));
  EXPECT_TRUE(contents.contains("ray_fan,"));
}
