#include "modalith_controller.hpp"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QUrl>

TEST(ProjectCompatibility, OpensLegacyPhotonProjectFiles) {
  QTemporaryDir directory(QDir::current().filePath("modalith-legacy-test-XXXXXX"));
  ASSERT_TRUE(directory.isValid());
  const QString modernPath = directory.filePath("modern.modalith");
  const QString legacyPath = directory.filePath("legacy.photon");

  ModalithController writer;
  writer.setSystemTitle("Legacy-compatible system");
  ASSERT_TRUE(writer.saveProject(QUrl::fromLocalFile(modernPath)))
      << writer.statusText().toStdString();

  QFile modernFile(modernPath);
  ASSERT_TRUE(modernFile.open(QIODevice::ReadOnly));
  QByteArray legacyData = modernFile.readAll();
  legacyData.replace("modalith-system", "photon-system");

  QFile legacyFile(legacyPath);
  ASSERT_TRUE(legacyFile.open(QIODevice::WriteOnly));
  ASSERT_EQ(legacyFile.write(legacyData), legacyData.size());
  legacyFile.close();

  ModalithController reader;
  ASSERT_TRUE(reader.openProject(QUrl::fromLocalFile(legacyPath)))
      << reader.statusText().toStdString();
  EXPECT_EQ(reader.systemTitle(), "Legacy-compatible system");
  EXPECT_EQ(reader.surfaceModel()->rowCount(), 3);
}
