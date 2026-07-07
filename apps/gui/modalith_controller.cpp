#include "modalith_controller.hpp"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringConverter>
#include <QTextStream>
#include <QTimer>
#include <QUndoCommand>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace {

class SurfaceSnapshotCommand final : public QUndoCommand {
public:
  SurfaceSnapshotCommand(QString label, std::vector<SurfaceRecord> before,
                         std::vector<SurfaceRecord> after, SurfaceTableModel* model)
      : QUndoCommand(std::move(label)), before_(std::move(before)),
        after_(std::move(after)), model_(model) {}
  void undo() override { model_->applySnapshot(before_); }
  void redo() override { model_->applySnapshot(after_); }

private:
  std::vector<SurfaceRecord> before_;
  std::vector<SurfaceRecord> after_;
  SurfaceTableModel* model_{};
};

[[nodiscard]] QString localPath(const QUrl& url) {
  return url.isLocalFile() ? url.toLocalFile() : url.toString();
}

[[nodiscard]] double degreesToRadians(double degrees) noexcept {
  return degrees * std::numbers::pi / 180.0;
}

[[nodiscard]] QJsonObject surfaceToJson(const SurfaceRecord& surface) {
  QJsonObject object;
  object["type"] = surface.type;
  object["radius"] = std::isfinite(surface.radius)
      ? QJsonValue{surface.radius} : QJsonValue{"Infinity"};
  object["thickness"] = surface.thickness;
  object["glass"] = surface.glass;
  object["semiDiameter"] = surface.semiDiameter;
  object["conic"] = surface.conic;
  object["asphereCoeffs"] = surface.asphereCoeffs;
  object["stop"] = surface.stop;
  object["decenterX"] = surface.decenterX;
  object["decenterY"] = surface.decenterY;
  object["tiltX"] = surface.tiltX;
  object["tiltY"] = surface.tiltY;
  return object;
}

[[nodiscard]] SurfaceRecord surfaceFromJson(const QJsonObject& object) {
  SurfaceRecord surface;
  surface.type = object["type"].toString("Plane");
  const auto radius = object["radius"];
  surface.radius = radius.isString()
      ? std::numeric_limits<double>::infinity() : radius.toDouble();
  surface.thickness = object["thickness"].toDouble();
  surface.glass = object["glass"].toString("AIR").toUpper();
  surface.semiDiameter = object["semiDiameter"].toDouble(10.0);
  surface.conic = object["conic"].toDouble();
  surface.asphereCoeffs = object["asphereCoeffs"].toString("");
  surface.stop = object["stop"].toBool();
  surface.decenterX = object["decenterX"].toDouble();
  surface.decenterY = object["decenterY"].toDouble();
  surface.tiltX = object["tiltX"].toDouble();
  surface.tiltY = object["tiltY"].toDouble();
  if (!(surface.semiDiameter > 0.0)) {
    throw std::runtime_error("Surface semi-diameter must be positive");
  }
  return surface;
}

}  // namespace

SurfaceTableModel::SurfaceTableModel(QObject* parent)
    : QAbstractTableModel(parent), undoStack_(this) {
  connect(&undoStack_, &QUndoStack::canUndoChanged, this, &SurfaceTableModel::undoStateChanged);
  connect(&undoStack_, &QUndoStack::canRedoChanged, this, &SurfaceTableModel::undoStateChanged);
  resetToSinglet();
}

int SurfaceTableModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : static_cast<int>(records_.size());
}

int SurfaceTableModel::columnCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : ColumnCount;
}

QVariant SurfaceTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= rowCount() ||
      (role != Qt::DisplayRole && role != Qt::EditRole)) return {};
  const auto& record = records_[static_cast<std::size_t>(index.row())];
  switch (index.column()) {
    case Type: return record.type;
    case Radius: return std::isfinite(record.radius) ? QVariant{record.radius} : QVariant{"Infinity"};
    case Thickness: return record.thickness;
    case Glass: return record.glass;
    case SemiDiameter: return record.semiDiameter;
    case Conic: return record.conic;
    case AsphereCoeffs: return record.asphereCoeffs;
    case Stop: return record.stop ? "Yes" : "No";
    case DecenterX: return record.decenterX;
    case DecenterY: return record.decenterY;
    case TiltX: return record.tiltX;
    case TiltY: return record.tiltY;
    default: return {};
  }
}

QVariant SurfaceTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return QAbstractTableModel::headerData(section, orientation, role);
  static const QStringList headers{"Surface Type", "Radius", "Thickness", "Material",
                                   "Semi-Diameter", "Conic", "Asphere Coeffs", "Stop",
                                   "Decenter X", "Decenter Y", "Tilt X", "Tilt Y"};
  return section >= 0 && section < headers.size() ? headers[section] : QVariant{};
}

Qt::ItemFlags SurfaceTableModel::flags(const QModelIndex& index) const {
  return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
                         : Qt::NoItemFlags;
}

bool SurfaceTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid() || role != Qt::EditRole || index.row() < 0 ||
      index.row() >= rowCount()) return false;

  auto after = records_;
  auto& record = after[static_cast<std::size_t>(index.row())];
  bool valid = true;
  switch (index.column()) {
    case Type: record.type = value.toString().trimmed(); break;
    case Radius: {
      const QString text = value.toString().trimmed();
      if (text.compare("Infinity", Qt::CaseInsensitive) == 0 ||
          text.compare("Inf", Qt::CaseInsensitive) == 0) {
        record.radius = std::numeric_limits<double>::infinity();
      } else {
        const double parsed = text.toDouble(&valid);
        if (valid) record.radius = parsed;
      }
      break;
    }
    case Thickness: {
      const double parsed = value.toString().toDouble(&valid);
      if (valid) record.thickness = parsed;
      break;
    }
    case Glass: record.glass = value.toString().trimmed().toUpper(); break;
    case SemiDiameter: {
      const double parsed = value.toString().toDouble(&valid);
      if (valid && parsed > 0.0) record.semiDiameter = parsed; else valid = false;
      break;
    }
    case Conic: {
      const double parsed = value.toString().toDouble(&valid);
      if (valid) record.conic = parsed;
      break;
    }
    case AsphereCoeffs: {
      record.asphereCoeffs = value.toString().trimmed();
      break;
    }
    case Stop: {
      const QString text = value.toString().trimmed().toLower();
      if (text == "1" || text == "true" || text == "yes" || text == "stop") {
        record.stop = true;
      } else if (text == "0" || text == "false" || text == "no" || text.isEmpty()) {
        record.stop = false;
      } else {
        valid = false;
      }
      break;
    }
    case DecenterX:
    case DecenterY:
    case TiltX:
    case TiltY: {
      const double parsed = value.toString().toDouble(&valid);
      if (!valid || !std::isfinite(parsed)) { valid = false; break; }
      if (index.column() == DecenterX) record.decenterX = parsed;
      else if (index.column() == DecenterY) record.decenterY = parsed;
      else if (index.column() == TiltX) record.tiltX = parsed;
      else record.tiltY = parsed;
      break;
    }
    default: return false;
  }
  if (!valid || record.type.isEmpty() || record.glass.isEmpty()) return false;
  pushSnapshot(QString("Edit surface %1").arg(index.row() + 1), std::move(after));
  return true;
}

bool SurfaceTableModel::setCell(int row, int column, const QVariant& value) {
  return setData(index(row, column), value, Qt::EditRole);
}

void SurfaceTableModel::pushSnapshot(QString label, std::vector<SurfaceRecord> after) {
  undoStack_.push(new SurfaceSnapshotCommand(std::move(label), records_, std::move(after), this));
}

void SurfaceTableModel::applySnapshot(const std::vector<SurfaceRecord>& records) {
  beginResetModel();
  records_ = records;
  endResetModel();
  emit opticalSystemChanged();
}

void SurfaceTableModel::addSurface() {
  auto after = records_;
  const auto insertion = std::max<std::ptrdiff_t>(0, static_cast<std::ptrdiff_t>(after.size()) - 1);
  after.insert(after.begin() + insertion,
               {"Sphere", 100.0, 5.0, "AIR", 12.5, 0.0, "", false});
  pushSnapshot("Insert surface", std::move(after));
}

void SurfaceTableModel::removeSurface(int row) {
  if (row < 0 || row >= rowCount() - 1 || rowCount() <= 2) return;
  auto after = records_;
  after.erase(after.begin() + row);
  pushSnapshot("Delete surface", std::move(after));
}

void SurfaceTableModel::duplicateSurface(int row) {
  if (row < 0 || row >= rowCount()) return;
  auto after = records_;
  SurfaceRecord copy = after[static_cast<std::size_t>(row)];
  copy.stop = false;
  after.insert(after.begin() + row + 1, std::move(copy));
  pushSnapshot("Duplicate surface", std::move(after));
}

void SurfaceTableModel::undo() { undoStack_.undo(); }
void SurfaceTableModel::redo() { undoStack_.redo(); }

void SurfaceTableModel::replaceRecords(std::vector<SurfaceRecord> records, bool clearHistory) {
  beginResetModel();
  records_ = std::move(records);
  endResetModel();
  if (clearHistory) undoStack_.clear();
  emit undoStateChanged();
  emit opticalSystemChanged();
}

void SurfaceTableModel::resetToSinglet() {
  replaceRecords({
    {"Sphere", 50.0, 5.0, "N-BK7", 12.5, 0.0, "", true},
    {"Sphere", -50.0, 47.0, "AIR", 12.5, 0.0, "", false},
    {"Plane", std::numeric_limits<double>::infinity(), 0.0, "AIR", 25.0, 0.0, "", false}
  });
}

ModalithController::ModalithController(QObject* parent)
    : QObject(parent), surfaceModel_(this) {
  connect(&surfaceModel_, &SurfaceTableModel::opticalSystemChanged, this, [this] {
    markModified();
    scheduleAnalysis();
  });
  connect(&surfaceModel_, &SurfaceTableModel::undoStateChanged,
          this, &ModalithController::projectChanged);
  QTimer::singleShot(0, this, &ModalithController::analyze);
}

void ModalithController::markModified() {
  if (loadingProject_) return;
  modified_ = true;
  emit projectChanged();
}

void ModalithController::setStatus(QString status) {
  statusText_ = std::move(status);
  emit statusChanged();
}

void ModalithController::setFieldX(double value) {
  if (qFuzzyCompare(fieldX_, value)) return;
  fieldX_ = value; markModified(); emit settingsChanged(); scheduleAnalysis();
}

void ModalithController::setFieldY(double value) {
  if (qFuzzyCompare(fieldY_, value)) return;
  fieldY_ = value; markModified(); emit settingsChanged(); scheduleAnalysis();
}

void ModalithController::setPupilRings(int value) {
  value = std::clamp(value, 1, 30);
  if (pupilRings_ == value) return;
  pupilRings_ = value; markModified(); emit settingsChanged(); scheduleAnalysis();
}

void ModalithController::setAutoUpdate(bool value) {
  if (autoUpdate_ == value) return;
  autoUpdate_ = value; emit settingsChanged();
  if (autoUpdate_) analyze();
}

void ModalithController::setSystemTitle(const QString& value) {
  const QString clean = value.trimmed();
  if (clean.isEmpty() || systemTitle_ == clean) return;
  systemTitle_ = clean; markModified(); emit projectChanged();
}

void ModalithController::setTemperatureC(double value) {
  if (!std::isfinite(value) || qFuzzyCompare(temperatureC_, value)) return;
  temperatureC_ = value; markModified(); emit projectChanged(); scheduleAnalysis();
}

QString ModalithController::wavelengthText() const {
  QStringList values;
  for (double wavelength : wavelengthsNm_) values << QString::number(wavelength, 'g', 10);
  return values.join(", ");
}

double ModalithController::referenceWavelength() const noexcept {
  if (wavelengthsNm_.empty()) return 587.5618;
  return wavelengthsNm_[wavelengthsNm_.size() / 2];
}

void ModalithController::setWavelengthText(const QString& value) {
  std::vector<double> parsed;
  const auto parts = value.split(QRegularExpression("[,;\\s]+"), Qt::SkipEmptyParts);
  for (const QString& part : parts) {
    bool ok = false;
    const double wavelength = part.toDouble(&ok);
    if (!ok || !(wavelength > 0.0) || !std::isfinite(wavelength)) {
      setStatus(QString("Invalid wavelength: %1").arg(part));
      return;
    }
    parsed.push_back(wavelength);
  }
  if (parsed.empty()) { setStatus("At least one wavelength is required"); return; }
  wavelengthsNm_ = std::move(parsed);
  markModified(); emit projectChanged(); scheduleAnalysis();
}

void ModalithController::scheduleAnalysis() {
  if (autoUpdate_) QTimer::singleShot(0, this, &ModalithController::analyze);
}

modalith::OpticalSystem ModalithController::buildOpticalSystem() {
  modalith::OpticalSystem system;
  system.title = systemTitle_.toStdString();
  system.temperature_c = temperatureC_;
  double vertexZ = 0.0;
  for (const auto& record : surfaceModel_.records()) {
    modalith::OpticalSurface surface;
    surface.label = record.type.toStdString();
    surface.transform.translation = {record.decenterX, record.decenterY, vertexZ};
    surface.transform.rotation = modalith::Mat3::from_euler_xyz(
        degreesToRadians(record.tiltX), degreesToRadians(record.tiltY), 0.0);
    const QString type = record.type.toLower();
    if (type.contains("asphere")) surface.profile.type = modalith::SurfaceType::EvenAsphere;
    else if (type.contains("conic")) surface.profile.type = modalith::SurfaceType::Conic;
    else if (type.contains("plane") || !std::isfinite(record.radius)) surface.profile.type = modalith::SurfaceType::Plane;
    else surface.profile.type = modalith::SurfaceType::Sphere;
    surface.profile.radius = record.radius;
    surface.profile.conic_constant = record.conic;
    if (!record.asphereCoeffs.isEmpty()) {
      const auto parts = record.asphereCoeffs.split(QRegularExpression("[,;\\s]+"), Qt::SkipEmptyParts);
      for (const QString& part : parts) {
        bool ok = false;
        const double coeff = part.toDouble(&ok);
        if (ok && std::isfinite(coeff)) {
          surface.profile.even_asphere_coefficients.push_back(coeff);
        }
      }
    }
    surface.semi_diameter = record.semiDiameter;
    surface.stop = record.stop;
    surface.material_after = catalog_.find(record.glass.toStdString());
    system.surfaces.push_back(std::move(surface));
    vertexZ += record.thickness;
  }
  return system;
}

void ModalithController::analyze() {
  setStatus("Tracing polychromatic pupil…");
  try {
    const modalith::OpticalSystem system = buildOpticalSystem();
    const modalith::FieldAngle field{fieldX_, fieldY_};
    const auto spot = analysis_.spot_diagram(system, field, wavelengthsNm_,
                                              static_cast<std::size_t>(pupilRings_));
    const double referenceWavelength = wavelengthsNm_[wavelengthsNm_.size() / 2];
    const auto fan = analysis_.ray_fan(system, field, referenceWavelength, 41);
    rmsSpotRadius_ = spot.rms_radius;
    geometricSpotRadius_ = spot.geometric_radius;
    tracedRays_ = static_cast<int>(spot.samples.size());
    vignettedRays_ = static_cast<int>(spot.vignetted_rays);
    spotPoints_.clear();
    for (const auto& sample : spot.samples)
      spotPoints_.push_back({sample.image.x - spot.centroid.x,
                             sample.image.y - spot.centroid.y, sample.wavelength_nm});
    fanPoints_.clear();
    for (const auto& sample : fan.samples)
      fanPoints_.push_back({sample.normalized_pupil, sample.tangential_error,
                            sample.sagittal_error});
    buildLayoutData(system);
    ++analysisRevision_;
    emit analysisChanged();
    setStatus(QString("Analysis complete · %1 rays · %2 vignetted")
                  .arg(tracedRays_).arg(vignettedRays_));
  } catch (const std::exception& error) {
    setStatus(QString("Analysis error: %1").arg(error.what()));
  }
}

void ModalithController::resetSystem() { newProject(); }

void ModalithController::newProject() {
  loadingProject_ = true;
  surfaceModel_.resetToSinglet();
  systemTitle_ = "Untitled system";
  temperatureC_ = 20.0;
  wavelengthsNm_ = {486.1327, 587.5618, 656.2725};
  fieldX_ = fieldY_ = 0.0;
  pupilRings_ = 8;
  currentFile_.clear();
  modified_ = false;
  loadingProject_ = false;
  emit settingsChanged(); emit projectChanged();
  analyze();
}

bool ModalithController::openProject(const QUrl& url) {
  const QString path = localPath(url);
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) { setStatus("Unable to open project: " + file.errorString()); return false; }
  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
    setStatus("Invalid project file: " + parseError.errorString()); return false;
  }
  try {
    const QJsonObject root = document.object();
    const QString format = root["format"].toString();
    if (format != "modalith-system" && format != "photon-system")
      throw std::runtime_error("Unrecognized project format");
    std::vector<SurfaceRecord> records;
    for (const auto& value : root["surfaces"].toArray()) records.push_back(surfaceFromJson(value.toObject()));
    if (records.size() < 2) throw std::runtime_error("A project requires at least two surfaces");
    std::vector<double> wavelengths;
    for (const auto& value : root["wavelengthsNm"].toArray()) wavelengths.push_back(value.toDouble());
    if (wavelengths.empty()) throw std::runtime_error("A project requires at least one wavelength");

    loadingProject_ = true;
    surfaceModel_.replaceRecords(std::move(records));
    systemTitle_ = root["title"].toString(QFileInfo(path).completeBaseName());
    temperatureC_ = root["temperatureC"].toDouble(20.0);
    wavelengthsNm_ = std::move(wavelengths);
    const auto field = root["fieldDegrees"].toObject();
    fieldX_ = field["x"].toDouble();
    fieldY_ = field["y"].toDouble();
    pupilRings_ = std::clamp(root["pupilRings"].toInt(8), 1, 30);
    currentFile_ = QFileInfo(path).absoluteFilePath();
    modified_ = false;
    loadingProject_ = false;
    emit settingsChanged(); emit projectChanged();
    analyze();
    setStatus("Opened " + QFileInfo(path).fileName());
    return true;
  } catch (const std::exception& error) {
    loadingProject_ = false;
    setStatus(QString("Invalid project: %1").arg(error.what()));
    return false;
  }
}

bool ModalithController::saveProject(const QUrl& url) {
  QString path = url.isEmpty() ? currentFile_ : localPath(url);
  if (path.isEmpty()) { setStatus("Choose a project filename"); return false; }
  if (QFileInfo(path).suffix().isEmpty()) path += ".modalith";
  QJsonObject root;
  root["format"] = "modalith-system";
  root["version"] = 1;
  root["title"] = systemTitle_;
  root["temperatureC"] = temperatureC_;
  root["pupilRings"] = pupilRings_;
  root["fieldDegrees"] = QJsonObject{{"x", fieldX_}, {"y", fieldY_}};
  QJsonArray wavelengths;
  for (double wavelength : wavelengthsNm_) wavelengths.append(wavelength);
  root["wavelengthsNm"] = wavelengths;
  QJsonArray surfaces;
  for (const auto& surface : surfaceModel_.records()) surfaces.append(surfaceToJson(surface));
  root["surfaces"] = surfaces;

  QSaveFile file(path);
  if (!file.open(QIODevice::WriteOnly)) { setStatus("Unable to save project: " + file.errorString()); return false; }
  file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  if (!file.commit()) { setStatus("Unable to commit project: " + file.errorString()); return false; }
  currentFile_ = QFileInfo(path).absoluteFilePath();
  modified_ = false;
  emit projectChanged();
  setStatus("Saved " + QFileInfo(path).fileName());
  return true;
}

bool ModalithController::exportAnalysisCsv(const QUrl& url) {
  QString path = localPath(url);
  if (QFileInfo(path).suffix().isEmpty()) path += ".csv";
  QSaveFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    setStatus("Unable to export analysis: " + file.errorString()); return false;
  }
  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::Utf8);
  stream << "analysis,x_mm,y_mm,wavelength_nm,pupil,tangential_mm,sagittal_mm\n";
  for (const auto& point : spotPoints_)
    stream << "spot," << point.x << ',' << point.y << ',' << point.wavelength << ",,,\n";
  for (const auto& point : fanPoints_)
    stream << "ray_fan,,,," << point.pupil << ',' << point.tangential << ',' << point.sagittal << '\n';
  stream.flush();
  if (!file.commit()) { setStatus("Unable to commit CSV export"); return false; }
  setStatus("Exported " + QFileInfo(path).fileName());
  return true;
}

void ModalithController::undo() { surfaceModel_.undo(); }
void ModalithController::redo() { surfaceModel_.redo(); }
void ModalithController::duplicateSurface(int row) { surfaceModel_.duplicateSurface(row); }

void ModalithController::buildLayoutData(const modalith::OpticalSystem& system) {
  layoutSurfaces_.clear();
  for (std::size_t index = 0; index < system.surfaces.size(); ++index) {
    const auto& surface = system.surfaces[index];
    const auto& record = surfaceModel_.records()[index];
    layoutSurfaces_.push_back({surface.transform.translation.z, surface.profile.radius,
                               surface.semi_diameter, surface.profile.conic_constant,
                               surface.transform.translation.x, surface.transform.translation.y,
                               record.tiltX, record.tiltY, surface.profile.type});
  }
  layoutRays_.clear();
  if (system.surfaces.empty()) return;
  const double aperture = system.surfaces.front().semi_diameter;
  const double radiansX = fieldX_ * std::numbers::pi / 180.0;
  const double radiansY = fieldY_ * std::numbers::pi / 180.0;
  const double referenceWavelength = wavelengthsNm_[wavelengthsNm_.size() / 2];
  const auto& first = system.surfaces.front();
  for (int sample = -5; sample <= 5; ++sample) {
    const double pupilY = aperture * static_cast<double>(sample) / 5.0;
    const double first_sag = modalith::surface_sag(first.profile, 0.0, pupilY);
    const double z_start = std::min(-1.0, (std::isfinite(first_sag) ? first_sag : 0.0) - 1.0);
    const modalith::Vec3 direction{std::tan(radiansX), std::tan(radiansY), 1.0};
    const modalith::Vec3 u = modalith::normalized(direction);
    const modalith::Vec3 origin_local{
      z_start * u.x / u.z,
      pupilY + z_start * u.y / u.z,
      z_start
    };
    modalith::Ray ray{
      .origin = first.transform.point_to_global(origin_local),
      .direction = u,
      .wavelength_nm = referenceWavelength
    };
    const auto trace = tracer_.trace(system, ray);
    QVector<QPointF> path;
    path.append(QPointF(ray.origin.z, ray.origin.y));
    for (const auto& segment : trace.segments) path.append(QPointF(segment.intercept.z, segment.intercept.y));
    if (path.size() > 1) layoutRays_.append(path);
  }
}
