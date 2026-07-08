#pragma once

#include <QGraphicsView>
#include "modalith/core/sequential_tracer.hpp"

class LensLayoutView : public QGraphicsView {
    Q_OBJECT

public:
    explicit LensLayoutView(QWidget* parent = nullptr);
    
    void setSystem(const modalith::OpticalSystem* system);
    void updateLayout();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawSurfaces(QGraphicsScene* scene);
    void drawRays(QGraphicsScene* scene);

    const modalith::OpticalSystem* m_system{nullptr};
    double m_zoomLevel{1.0};
};
