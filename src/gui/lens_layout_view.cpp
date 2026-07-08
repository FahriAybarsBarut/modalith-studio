#include "lens_layout_view.hpp"
#include <QGraphicsScene>
#include <QPainterPath>
#include <QWheelEvent>
#include <QPen>

LensLayoutView::LensLayoutView(QWidget* parent)
    : QGraphicsView(parent) {
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setScene(new QGraphicsScene(this));
}

void LensLayoutView::setSystem(const modalith::OpticalSystem* system) {
    m_system = system;
    updateLayout();
}

void LensLayoutView::updateLayout() {
    auto* s = scene();
    s->clear();
    
    if (!m_system || m_system->surfaces.empty()) return;

    drawSurfaces(s);
    drawRays(s);

    s->setSceneRect(s->itemsBoundingRect().adjusted(-10, -10, 10, 10));
}

void LensLayoutView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void LensLayoutView::wheelEvent(QWheelEvent* event) {
    double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

void LensLayoutView::drawSurfaces(QGraphicsScene* scene) {
    QPen surfacePen(Qt::white, 0.5);
    QPen edgePen(Qt::lightGray, 0.2);
    
    for (size_t i = 0; i < m_system->surfaces.size(); ++i) {
        const auto& surf = m_system->surfaces[i];
        
        // Z-axis is horizontal (x in Qt), Y-axis is vertical (y in Qt)
        double z_pos = surf.transform.translation.z;
        double sd = std::isinf(surf.semi_diameter) ? 20.0 : surf.semi_diameter; // Default SD if inf
        
        // Very basic sag calculation (just a straight line if plane, arc if sphere)
        QPainterPath path;
        int num_points = 20;
        for (int p = 0; p <= num_points; ++p) {
            double y = -sd + (2.0 * sd * p / num_points);
            double sag = modalith::surface_sag(surf.profile, 0.0, y);
            if (std::isnan(sag)) sag = 0.0;
            
            // Note: Qt y-axis is down, but we want mathematical y-axis (up is +).
            // We just invert y visually if needed, but for layout it's symmetric.
            if (p == 0) path.moveTo(z_pos + sag, y);
            else path.lineTo(z_pos + sag, y);
        }
        scene->addPath(path, surfacePen);

        // Draw top/bottom edges if it's a solid element (not air)
        if (i < m_system->surfaces.size() - 1 && surf.material_after && surf.material_after->name() != "Air") {
            const auto& next_surf = m_system->surfaces[i+1];
            double next_z = next_surf.transform.translation.z;
            double next_sd = std::isinf(next_surf.semi_diameter) ? 20.0 : next_surf.semi_diameter;
            
            double sag_top = modalith::surface_sag(surf.profile, 0.0, sd);
            double next_sag_top = modalith::surface_sag(next_surf.profile, 0.0, next_sd);
            scene->addLine(z_pos + sag_top, sd, next_z + next_sag_top, next_sd, edgePen);
            
            double sag_bottom = modalith::surface_sag(surf.profile, 0.0, -sd);
            double next_sag_bottom = modalith::surface_sag(next_surf.profile, 0.0, -next_sd);
            scene->addLine(z_pos + sag_bottom, -sd, next_z + next_sag_bottom, -next_sd, edgePen);
        }
    }
}

void LensLayoutView::drawRays(QGraphicsScene* scene) {
    if (m_system->surfaces.empty()) return;

    modalith::SequentialTracer tracer;
    QPen rayPen(Qt::blue, 0.1);
    
    // Trace a fan of rays from object
    double start_z = m_system->surfaces[0].transform.translation.z;
    double entrance_pupil_radius = 10.0; // Assume 10mm pupil for visualization if not calculated
    
    // Quick heuristic for entrance pupil
    for (const auto& surf : m_system->surfaces) {
        if (surf.stop && !std::isinf(surf.semi_diameter)) {
            entrance_pupil_radius = surf.semi_diameter;
            break;
        }
    }

    int num_rays = 5;
    for (int i = 0; i < num_rays; ++i) {
        double py = -entrance_pupil_radius + (2.0 * entrance_pupil_radius * i / (num_rays - 1));
        
        // Assuming infinite conjugate (parallel rays)
        modalith::Ray ray;
        ray.origin = modalith::Vec3{0, py, start_z - 10.0};
        ray.direction = modalith::Vec3{0, 0, 1};
        
        auto result = tracer.trace(*m_system, ray);
        
        // Draw segments
        modalith::Vec3 current_pt = ray.origin;
        for (const auto& seg : result.segments) {
            scene->addLine(current_pt.z, current_pt.y, seg.intercept.z, seg.intercept.y, rayPen);
            current_pt = seg.intercept;
        }
    }
}
