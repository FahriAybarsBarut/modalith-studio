#include "modalith_controller.hpp"

#include <gtest/gtest.h>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QUrl>
#include <cmath>

TEST(FormatCompatibility, ZemaxZmxImportExport) {
  QTemporaryDir directory(QDir::current().filePath("modalith-zmx-test-XXXXXX"));
  ASSERT_TRUE(directory.isValid());
  const QString zmxPath = directory.filePath("test_singlet.zmx");

  ModalithController writer;
  writer.setSystemTitle("Zmx Test Singlet");
  writer.setTemperatureC(25.0);
  writer.setWavelengthText("486.1327, 587.5618, 656.2725");

  // Modify some surface records
  SurfaceTableModel* model = writer.surfaceModel();
  ASSERT_EQ(model->rowCount(), 3);
  
  // Set surface 0 (first lens surface)
  model->setData(model->index(0, SurfaceTableModel::Radius), 60.0, Qt::EditRole);
  model->setData(model->index(0, SurfaceTableModel::Thickness), 6.5, Qt::EditRole);
  model->setData(model->index(0, SurfaceTableModel::Glass), "N-BK7", Qt::EditRole);
  model->setData(model->index(0, SurfaceTableModel::SemiDiameter), 15.0, Qt::EditRole);
  model->setData(model->index(0, SurfaceTableModel::Conic), -0.5, Qt::EditRole);
  model->setData(model->index(0, SurfaceTableModel::Stop), true, Qt::EditRole);

  // Set surface 1 (second lens surface)
  model->setData(model->index(1, SurfaceTableModel::Radius), -60.0, Qt::EditRole);
  model->setData(model->index(1, SurfaceTableModel::Thickness), 55.0, Qt::EditRole);
  model->setData(model->index(1, SurfaceTableModel::Glass), "AIR", Qt::EditRole);

  ASSERT_TRUE(writer.saveProject(QUrl::fromLocalFile(zmxPath)))
      << writer.statusText().toStdString();

  // Now read it back using a new reader
  ModalithController reader;
  ASSERT_TRUE(reader.openProject(QUrl::fromLocalFile(zmxPath)))
      << reader.statusText().toStdString();

  EXPECT_EQ(reader.systemTitle(), "Zmx Test Singlet");
  EXPECT_NEAR(reader.temperatureC(), 25.0, 1e-5);
  EXPECT_EQ(reader.wavelengthCount(), 3);
  
  SurfaceTableModel* readModel = reader.surfaceModel();
  ASSERT_EQ(readModel->rowCount(), 3);

  // Surface 0 check
  EXPECT_NEAR(readModel->data(readModel->index(0, SurfaceTableModel::Radius), Qt::DisplayRole).toDouble(), 60.0, 1e-5);
  EXPECT_NEAR(readModel->data(readModel->index(0, SurfaceTableModel::Thickness), Qt::DisplayRole).toDouble(), 6.5, 1e-5);
  EXPECT_EQ(readModel->data(readModel->index(0, SurfaceTableModel::Glass), Qt::DisplayRole).toString(), "N-BK7");
  EXPECT_NEAR(readModel->data(readModel->index(0, SurfaceTableModel::SemiDiameter), Qt::DisplayRole).toDouble(), 15.0, 1e-5);
  EXPECT_NEAR(readModel->data(readModel->index(0, SurfaceTableModel::Conic), Qt::DisplayRole).toDouble(), -0.5, 1e-5);
  EXPECT_TRUE(readModel->data(readModel->index(0, SurfaceTableModel::Stop), Qt::DisplayRole).toBool());

  // Surface 1 check
  EXPECT_NEAR(readModel->data(readModel->index(1, SurfaceTableModel::Radius), Qt::DisplayRole).toDouble(), -60.0, 1e-5);
  EXPECT_NEAR(readModel->data(readModel->index(1, SurfaceTableModel::Thickness), Qt::DisplayRole).toDouble(), 55.0, 1e-5);
  EXPECT_EQ(readModel->data(readModel->index(1, SurfaceTableModel::Glass), Qt::DisplayRole).toString(), "AIR");
}

TEST(FormatCompatibility, CodeVSeqImport) {
  QTemporaryDir directory(QDir::current().filePath("modalith-seq-test-XXXXXX"));
  ASSERT_TRUE(directory.isValid());
  const QString seqPath = directory.filePath("test_lens.seq");

  QFile file(seqPath);
  ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
  QTextStream stream(&file);
  stream << "! Mock CODE V Sequence file\n"
         << "TIT 'CODEV Test Lens'\n"
         << "WL 486.13 587.56 656.27\n"
         << "STO 1\n"
         << "S 1 RAD 80.0\n"
         << "S 1 THI 8.0\n"
         << "S 1 GLASS 'N-BK7'\n"
         << "S 1 SD 18.0\n"
         << "S 2 RAD -80.0\n"
         << "S 2 THI 60.0\n"
         << "S 2 GLASS 'AIR'\n"
         << "S 2 SD 18.0\n"
         << "S 3 RAD 0.0\n"
         << "S 3 THI 0.0\n";
  file.close();

  ModalithController reader;
  ASSERT_TRUE(reader.openProject(QUrl::fromLocalFile(seqPath)))
      << reader.statusText().toStdString();

  EXPECT_EQ(reader.systemTitle(), "CODEV Test Lens");
  EXPECT_EQ(reader.wavelengthCount(), 3);
  
  SurfaceTableModel* model = reader.surfaceModel();
  ASSERT_EQ(model->rowCount(), 3);

  EXPECT_NEAR(model->data(model->index(0, SurfaceTableModel::Radius), Qt::DisplayRole).toDouble(), 80.0, 1e-5);
  EXPECT_NEAR(model->data(model->index(0, SurfaceTableModel::Thickness), Qt::DisplayRole).toDouble(), 8.0, 1e-5);
  EXPECT_EQ(model->data(model->index(0, SurfaceTableModel::Glass), Qt::DisplayRole).toString(), "N-BK7");
  EXPECT_NEAR(model->data(model->index(0, SurfaceTableModel::SemiDiameter), Qt::DisplayRole).toDouble(), 18.0, 1e-5);
  EXPECT_TRUE(model->data(model->index(0, SurfaceTableModel::Stop), Qt::DisplayRole).toBool());

  EXPECT_NEAR(model->data(model->index(1, SurfaceTableModel::Radius), Qt::DisplayRole).toDouble(), -80.0, 1e-5);
  EXPECT_NEAR(model->data(model->index(1, SurfaceTableModel::Thickness), Qt::DisplayRole).toDouble(), 60.0, 1e-5);
}

TEST(FormatCompatibility, ZemaxAgfCatalogImport) {
  QTemporaryDir directory(QDir::current().filePath("modalith-agf-test-XXXXXX"));
  ASSERT_TRUE(directory.isValid());
  const QString agfPath = directory.filePath("test_catalog.agf");

  QFile file(agfPath);
  ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
  QTextStream stream(&file);
  stream << "NM AGF_TEST_BK7 1 0 1.5168 64.17\n"
         << "CD 1.03961212 0.00600069867 0.231792344 0.0200179144 1.01046945 103.560653\n"
         << "NM AGF_TEST_CONSTANT 2 0 1.620 36.4\n";
  file.close();

  ModalithController reader;
  ASSERT_TRUE(reader.importGlassCatalog(QUrl::fromLocalFile(agfPath)))
      << reader.statusText().toStdString();

  SurfaceTableModel* model = reader.surfaceModel();
  
  model->setData(model->index(0, SurfaceTableModel::Glass), "AGF_TEST_BK7", Qt::EditRole);
  reader.analyze();
  EXPECT_FALSE(reader.statusText().contains("not found"));

  model->setData(model->index(0, SurfaceTableModel::Glass), "AGF_TEST_CONSTANT", Qt::EditRole);
  reader.analyze();
  EXPECT_FALSE(reader.statusText().contains("not found"));
}
