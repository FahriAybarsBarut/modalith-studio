#include "plot_items.hpp"

#include "modalith_controller.hpp"

#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include <algorithm>
#include <cmath>

namespace {
constexpr QColor kBackground{24, 27, 31};
constexpr QColor kGrid{52, 58, 67};
constexpr QColor kAxis{113, 120, 132};
constexpr QColor kText{200, 205, 212};
constexpr QColor kCyan{91, 141, 239};
constexpr QColor kOrange{213, 162, 75};
constexpr QColor kBlue{111, 159, 237};
constexpr QColor kRed{201, 107, 112};

void drawEmptyMessage(QPainter* painter, const QRectF& bounds, const QString& message) {
  painter->setPen(kAxis);
  painter->drawText(bounds, Qt::AlignCenter, message);
}
}  // namespace

ModalithPlotItem::ModalithPlotItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
  setAntialiasing(true);
}

QObject* ModalithPlotItem::controller() const noexcept { return controller_; }

void ModalithPlotItem::setController(QObject* controller) {
  auto* typed = qobject_cast<ModalithController*>(controller);
  if (controller_ == typed) return;
  controller_ = typed;
  emit controllerChanged();
  update();
}

void ModalithPlotItem::setRevision(int revision) {
  if (revision_ == revision) return;
  revision_ = revision;
  emit revisionChanged();
  update();
}

void ModalithPlotItem::drawFrame(QPainter* painter, const QRectF& plot,
                               const QString& xLabel, const QString& yLabel) const {
  painter->fillRect(boundingRect(), kBackground);
  painter->setPen(QPen{kGrid, 1.0});
  for (int division = 0; division <= 8; ++division) {
    const double fraction = static_cast<double>(division) / 8.0;
    painter->drawLine(QPointF{plot.left() + plot.width() * fraction, plot.top()},
                      QPointF{plot.left() + plot.width() * fraction, plot.bottom()});
    painter->drawLine(QPointF{plot.left(), plot.top() + plot.height() * fraction},
                      QPointF{plot.right(), plot.top() + plot.height() * fraction});
  }
  painter->setPen(QPen{kAxis, 1.2});
  painter->drawRect(plot);
  painter->setPen(kText);
  painter->drawText(QRectF{plot.left(), plot.bottom() + 8.0, plot.width(), 24.0},
                    Qt::AlignHCenter | Qt::AlignTop, xLabel);
  painter->save();
  painter->translate(14.0, plot.center().y());
  painter->rotate(-90.0);
  painter->drawText(QRectF{-plot.height() / 2.0, -10.0, plot.height(), 20.0},
                    Qt::AlignCenter, yLabel);
  painter->restore();
}

void SpotPlotItem::paint(QPainter* painter) {
  painter->setRenderHint(QPainter::Antialiasing);
  const QRectF plot = boundingRect().adjusted(48.0, 24.0, -20.0, -42.0);
  drawFrame(painter, plot, "Image X [mm]", "Image Y [mm]");
  const auto* controller = modalithController();
  if (!controller || controller->spotPoints().empty()) {
    drawEmptyMessage(painter, plot, "No spot data");
    return;
  }

  double extent = 0.0;
  for (const auto& point : controller->spotPoints()) {
    extent = std::max({extent, std::abs(point.x), std::abs(point.y)});
  }
  extent = std::max(extent * 1.12, 1.0e-6);
  const auto mapPoint = [&](double x, double y) {
    return QPointF{plot.center().x() + x / extent * plot.width() * 0.5,
                   plot.center().y() - y / extent * plot.height() * 0.5};
  };

  painter->setPen(QPen{kAxis, 1.0, Qt::DashLine});
  painter->drawLine(QPointF{plot.center().x(), plot.top()},
                    QPointF{plot.center().x(), plot.bottom()});
  painter->drawLine(QPointF{plot.left(), plot.center().y()},
                    QPointF{plot.right(), plot.center().y()});
  painter->setPen(Qt::NoPen);
  for (const auto& point : controller->spotPoints()) {
    const QColor color = point.wavelength < 530.0 ? kBlue :
                         point.wavelength > 620.0 ? kRed : kCyan;
    painter->setBrush(color);
    painter->drawEllipse(mapPoint(point.x, point.y), 2.3, 2.3);
  }
  painter->setBrush(Qt::NoBrush);
  painter->setPen(QPen{kOrange, 1.5, Qt::DashLine});
  const double rms = controller->rmsSpotRadius();
  const double radiusPixels = rms / extent * std::min(plot.width(), plot.height()) * 0.5;
  painter->drawEllipse(plot.center(), radiusPixels, radiusPixels);
  painter->setPen(kText);
  painter->drawText(QPointF{plot.left() + 8.0, plot.top() + 18.0},
                    QString("±%1 mm").arg(extent, 0, 'g', 4));
}

void RayFanPlotItem::paint(QPainter* painter) {
  painter->setRenderHint(QPainter::Antialiasing);
  const QRectF plot = boundingRect().adjusted(48.0, 24.0, -20.0, -42.0);
  drawFrame(painter, plot, "Normalized pupil", "Transverse error [mm]");
  const auto* controller = modalithController();
  if (!controller || controller->fanPoints().empty()) {
    drawEmptyMessage(painter, plot, "No ray-fan data");
    return;
  }

  double extent = 0.0;
  for (const auto& point : controller->fanPoints()) {
    extent = std::max({extent, std::abs(point.tangential), std::abs(point.sagittal)});
  }
  extent = std::max(extent * 1.12, 1.0e-6);
  const auto mapPoint = [&](double pupil, double error) {
    return QPointF{plot.left() + (pupil + 1.0) * 0.5 * plot.width(),
                   plot.center().y() - error / extent * plot.height() * 0.5};
  };
  painter->setPen(QPen{kAxis, 1.0, Qt::DashLine});
  painter->drawLine(QPointF{plot.left(), plot.center().y()},
                    QPointF{plot.right(), plot.center().y()});
  painter->drawLine(QPointF{plot.center().x(), plot.top()},
                    QPointF{plot.center().x(), plot.bottom()});

  QPainterPath tangential;
  QPainterPath sagittal;
  bool first = true;
  for (const auto& point : controller->fanPoints()) {
    const QPointF t = mapPoint(point.pupil, point.tangential);
    const QPointF s = mapPoint(point.pupil, point.sagittal);
    if (first) { tangential.moveTo(t); sagittal.moveTo(s); first = false; }
    else { tangential.lineTo(t); sagittal.lineTo(s); }
  }
  painter->setPen(QPen{kCyan, 2.0});
  painter->drawPath(tangential);
  painter->setPen(QPen{kOrange, 2.0});
  painter->drawPath(sagittal);
  painter->setPen(kCyan);
  painter->drawText(QPointF{plot.left() + 8.0, plot.top() + 18.0}, "Tangential");
  painter->setPen(kOrange);
  painter->drawText(QPointF{plot.left() + 92.0, plot.top() + 18.0}, "Sagittal");
}

void OpticalLayoutItem::paint(QPainter* painter) {
  painter->setRenderHint(QPainter::Antialiasing);
  painter->fillRect(boundingRect(), QColor{21, 23, 26});
  const QRectF plot = boundingRect().adjusted(34.0, 28.0, -28.0, -34.0);
  const auto* controller = modalithController();
  if (!controller || controller->layoutSurfaces().empty()) {
    drawEmptyMessage(painter, plot, "No optical layout");
    return;
  }

  double minZ = -3.0;
  double maxZ = minZ + 1.0;
  double maxY = 1.0;
  for (const auto& surface : controller->layoutSurfaces()) {
    const double axialAllowance = std::isfinite(surface.radius)
        ? std::min(std::abs(surface.radius) * 0.08, surface.semiDiameter) : 0.0;
    maxZ = std::max(maxZ, surface.vertexZ + axialAllowance);
    maxY = std::max(maxY, surface.semiDiameter);
  }
  maxZ += std::max(2.0, (maxZ - minZ) * 0.04);
  maxY *= 1.25;
  const auto map = [&](double z, double y) {
    return QPointF{plot.left() + (z - minZ) / (maxZ - minZ) * plot.width(),
                   plot.center().y() - y / maxY * plot.height() * 0.5};
  };

  painter->setPen(QPen{kGrid, 1.0});
  painter->drawLine(map(minZ, 0.0), map(maxZ, 0.0));
  for (const auto& ray : controller->layoutRays()) {
    if (ray.size() < 2) continue;
    QPainterPath path{map(ray.front().x(), ray.front().y())};
    for (qsizetype i = 1; i < ray.size(); ++i) path.lineTo(map(ray[i].x(), ray[i].y()));
    painter->setPen(QPen{QColor{250, 204, 21, 155}, 1.0});
    painter->drawPath(path);
  }

  for (std::size_t index = 0; index < controller->layoutSurfaces().size(); ++index) {
    const auto& surface = controller->layoutSurfaces()[index];
    QPainterPath profile;
    constexpr int segments = 64;
    for (int segment = 0; segment <= segments; ++segment) {
      const double y = -surface.semiDiameter + 2.0 * surface.semiDiameter *
          static_cast<double>(segment) / segments;
      modalith::SurfaceProfile opticalProfile;
      opticalProfile.type = surface.type;
      opticalProfile.radius = surface.radius;
      opticalProfile.conic_constant = surface.conic;
      double sag = modalith::surface_sag(opticalProfile, 0.0, y);
      if (!std::isfinite(sag)) sag = 0.0;
      const QPointF point = map(surface.vertexZ + sag, y);
      if (segment == 0) profile.moveTo(point); else profile.lineTo(point);
    }
    painter->setPen(QPen{index + 1 == controller->layoutSurfaces().size() ? kOrange : kCyan,
                         index + 1 == controller->layoutSurfaces().size() ? 1.5 : 2.2});
    painter->drawPath(profile);
    painter->setPen(kText);
    painter->drawText(map(surface.vertexZ, -maxY * 0.88), QString::number(index + 1));
  }

  painter->setPen(kAxis);
  painter->drawText(QRectF{plot.left(), plot.bottom() + 8.0, plot.width(), 20.0},
                    Qt::AlignCenter, "Optical axis Z [mm]");
}
