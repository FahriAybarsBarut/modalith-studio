#pragma once

#include "modalith/analysis/analysis.hpp"

#include <QAbstractTableModel>
#include <QPointF>
#include <QString>
#include <QUndoStack>
#include <QUrl>
#include <QVector>

#include <vector>

struct SurfaceRecord {
  QString type;
  double radius{};
  double thickness{};
  QString glass;
  double semiDiameter{};
  double conic{};
  QString asphereCoeffs;
  bool stop{};
  double decenterX{};
  double decenterY{};
  double tiltX{};
  double tiltY{};
};

class SurfaceTableModel final : public QAbstractTableModel {
  Q_OBJECT

public:
  enum Column {
    Type,
    Radius,
    Thickness,
    Glass,
    SemiDiameter,
    Conic,
    AsphereCoeffs,
    Stop,
    DecenterX,
    DecenterY,
    TiltX,
    TiltY,
    ColumnCount
  };

  explicit SurfaceTableModel(QObject* parent = nullptr);
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  Q_INVOKABLE bool setCell(int row, int column, const QVariant& value);
  Q_INVOKABLE void addSurface();
  Q_INVOKABLE void removeSurface(int row);
  Q_INVOKABLE void duplicateSurface(int row);
  Q_INVOKABLE void undo();
  Q_INVOKABLE void redo();

  void resetToSinglet();
  void replaceRecords(std::vector<SurfaceRecord> records, bool clearHistory = true);
  void applySnapshot(const std::vector<SurfaceRecord>& records);
  [[nodiscard]] const std::vector<SurfaceRecord>& records() const noexcept { return records_; }
  [[nodiscard]] bool canUndo() const noexcept { return undoStack_.canUndo(); }
  [[nodiscard]] bool canRedo() const noexcept { return undoStack_.canRedo(); }

signals:
  void opticalSystemChanged();
  void undoStateChanged();

private:
  void pushSnapshot(QString label, std::vector<SurfaceRecord> after);
  std::vector<SurfaceRecord> records_;
  QUndoStack undoStack_;
};

struct GuiSpotPoint { double x{}; double y{}; double wavelength{}; };
struct GuiFanPoint { double pupil{}; double tangential{}; double sagittal{}; };
struct GuiSurface {
  double vertexZ{};
  double radius{};
  double semiDiameter{};
  double conic{};
  double decenterX{};
  double decenterY{};
  double tiltX{};
  double tiltY{};
  modalith::SurfaceType type{modalith::SurfaceType::Plane};
};

class ModalithController final : public QObject {
  Q_OBJECT
  Q_PROPERTY(double rmsSpotRadius READ rmsSpotRadius NOTIFY analysisChanged)
  Q_PROPERTY(double geometricSpotRadius READ geometricSpotRadius NOTIFY analysisChanged)
  Q_PROPERTY(int tracedRays READ tracedRays NOTIFY analysisChanged)
  Q_PROPERTY(int vignettedRays READ vignettedRays NOTIFY analysisChanged)
  Q_PROPERTY(int fanSamples READ fanSamples NOTIFY analysisChanged)
  Q_PROPERTY(double fieldX READ fieldX WRITE setFieldX NOTIFY settingsChanged)
  Q_PROPERTY(double fieldY READ fieldY WRITE setFieldY NOTIFY settingsChanged)
  Q_PROPERTY(int pupilRings READ pupilRings WRITE setPupilRings NOTIFY settingsChanged)
  Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY settingsChanged)
  Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
  Q_PROPERTY(int analysisRevision READ analysisRevision NOTIFY analysisChanged)
  Q_PROPERTY(QString systemTitle READ systemTitle WRITE setSystemTitle NOTIFY projectChanged)
  Q_PROPERTY(double temperatureC READ temperatureC WRITE setTemperatureC NOTIFY projectChanged)
  Q_PROPERTY(QString wavelengthText READ wavelengthText WRITE setWavelengthText NOTIFY projectChanged)
  Q_PROPERTY(int wavelengthCount READ wavelengthCount NOTIFY projectChanged)
  Q_PROPERTY(double referenceWavelength READ referenceWavelength NOTIFY projectChanged)
  Q_PROPERTY(QString currentFile READ currentFile NOTIFY projectChanged)
  Q_PROPERTY(bool modified READ modified NOTIFY projectChanged)
  Q_PROPERTY(bool canUndo READ canUndo NOTIFY projectChanged)
  Q_PROPERTY(bool canRedo READ canRedo NOTIFY projectChanged)

public:
  explicit ModalithController(QObject* parent = nullptr);

  [[nodiscard]] SurfaceTableModel* surfaceModel() noexcept { return &surfaceModel_; }
  [[nodiscard]] double rmsSpotRadius() const noexcept { return rmsSpotRadius_; }
  [[nodiscard]] double geometricSpotRadius() const noexcept { return geometricSpotRadius_; }
  [[nodiscard]] int tracedRays() const noexcept { return tracedRays_; }
  [[nodiscard]] int vignettedRays() const noexcept { return vignettedRays_; }
  [[nodiscard]] int fanSamples() const noexcept { return static_cast<int>(fanPoints_.size()); }
  [[nodiscard]] double fieldX() const noexcept { return fieldX_; }
  [[nodiscard]] double fieldY() const noexcept { return fieldY_; }
  [[nodiscard]] int pupilRings() const noexcept { return pupilRings_; }
  [[nodiscard]] bool autoUpdate() const noexcept { return autoUpdate_; }
  [[nodiscard]] QString statusText() const { return statusText_; }
  [[nodiscard]] int analysisRevision() const noexcept { return analysisRevision_; }
  [[nodiscard]] QString systemTitle() const { return systemTitle_; }
  [[nodiscard]] double temperatureC() const noexcept { return temperatureC_; }
  [[nodiscard]] QString wavelengthText() const;
  [[nodiscard]] int wavelengthCount() const noexcept { return static_cast<int>(wavelengthsNm_.size()); }
  [[nodiscard]] double referenceWavelength() const noexcept;
  [[nodiscard]] QString currentFile() const { return currentFile_; }
  [[nodiscard]] bool modified() const noexcept { return modified_; }
  [[nodiscard]] bool canUndo() const noexcept { return surfaceModel_.canUndo(); }
  [[nodiscard]] bool canRedo() const noexcept { return surfaceModel_.canRedo(); }

  void setFieldX(double value);
  void setFieldY(double value);
  void setPupilRings(int value);
  void setAutoUpdate(bool value);
  void setSystemTitle(const QString& value);
  void setTemperatureC(double value);
  void setWavelengthText(const QString& value);

  [[nodiscard]] const std::vector<GuiSpotPoint>& spotPoints() const noexcept { return spotPoints_; }
  [[nodiscard]] const std::vector<GuiFanPoint>& fanPoints() const noexcept { return fanPoints_; }
  [[nodiscard]] const std::vector<GuiSurface>& layoutSurfaces() const noexcept { return layoutSurfaces_; }
  [[nodiscard]] const QVector<QVector<QPointF>>& layoutRays() const noexcept { return layoutRays_; }

  Q_INVOKABLE void analyze();
  Q_INVOKABLE void resetSystem();
  Q_INVOKABLE void newProject();
  Q_INVOKABLE bool openProject(const QUrl& url);
  Q_INVOKABLE bool saveProject(const QUrl& url = {});
  Q_INVOKABLE bool exportAnalysisCsv(const QUrl& url);
  Q_INVOKABLE void undo();
  Q_INVOKABLE void redo();
  Q_INVOKABLE void duplicateSurface(int row);

signals:
  void analysisChanged();
  void settingsChanged();
  void statusChanged();
  void projectChanged();

private:
  [[nodiscard]] modalith::OpticalSystem buildOpticalSystem();
  void buildLayoutData(const modalith::OpticalSystem& system);
  void scheduleAnalysis();
  void markModified();
  void setStatus(QString status);

  modalith::MaterialCatalog catalog_;
  SurfaceTableModel surfaceModel_;
  modalith::SequentialTracer tracer_;
  modalith::SequentialAnalysis analysis_{tracer_};

  double rmsSpotRadius_{};
  double geometricSpotRadius_{};
  int tracedRays_{};
  int vignettedRays_{};
  double fieldX_{};
  double fieldY_{};
  int pupilRings_{8};
  bool autoUpdate_{true};
  QString statusText_{"Ready"};
  int analysisRevision_{};
  QString systemTitle_{"Untitled system"};
  double temperatureC_{20.0};
  std::vector<double> wavelengthsNm_{486.1327, 587.5618, 656.2725};
  QString currentFile_;
  bool modified_{};
  bool loadingProject_{};
  std::vector<GuiSpotPoint> spotPoints_;
  std::vector<GuiFanPoint> fanPoints_;
  std::vector<GuiSurface> layoutSurfaces_;
  QVector<QVector<QPointF>> layoutRays_;
};
