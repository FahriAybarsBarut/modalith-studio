#pragma once

#include <QQuickPaintedItem>

class ModalithController;

class ModalithPlotItem : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(QObject* controller READ controller WRITE setController NOTIFY controllerChanged)
  Q_PROPERTY(int revision READ revision WRITE setRevision NOTIFY revisionChanged)

public:
  explicit ModalithPlotItem(QQuickItem* parent = nullptr);
  [[nodiscard]] QObject* controller() const noexcept;
  void setController(QObject* controller);
  [[nodiscard]] int revision() const noexcept { return revision_; }
  void setRevision(int revision);

signals:
  void controllerChanged();
  void revisionChanged();

protected:
  [[nodiscard]] ModalithController* modalithController() const noexcept { return controller_; }
  void drawFrame(QPainter* painter, const QRectF& plot, const QString& xLabel,
                 const QString& yLabel) const;

private:
  ModalithController* controller_{};
  int revision_{};
};

class SpotPlotItem : public ModalithPlotItem {
  Q_OBJECT
public:
  using ModalithPlotItem::ModalithPlotItem;
  void paint(QPainter* painter) override;
};

class RayFanPlotItem : public ModalithPlotItem {
  Q_OBJECT
public:
  using ModalithPlotItem::ModalithPlotItem;
  void paint(QPainter* painter) override;
};

class OpticalLayoutItem : public ModalithPlotItem {
  Q_OBJECT
public:
  using ModalithPlotItem::ModalithPlotItem;
  void paint(QPainter* painter) override;
};
