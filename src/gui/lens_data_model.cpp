#include "lens_data_model.hpp"
#include <QBrush>
#include <QColor>
#include <cmath>

LensDataModel::LensDataModel(modalith::OpticalSystem* system, QObject* parent)
    : QAbstractTableModel(parent), m_system(system) {
}

int LensDataModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || !m_system) return 0;
    return static_cast<int>(m_system->surfaces.size());
}

int LensDataModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant LensDataModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || !m_system) return QVariant();

    const int row = index.row();
    const auto& surface = m_system->surfaces[row];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case ColSurface:
                if (row == 0) return "OBJ";
                if (row == m_system->surfaces.size() - 1) return "IMA";
                if (surface.stop) return "STO";
                return QString::number(row);
            case ColType: {
                switch (surface.profile.type) {
                    case modalith::SurfaceType::Plane: return "Plane";
                    case modalith::SurfaceType::Sphere: return "Sphere";
                    case modalith::SurfaceType::Conic: return "Conic";
                    case modalith::SurfaceType::EvenAsphere: return "Even Asphere";
                    case modalith::SurfaceType::OddAsphere: return "Odd Asphere";
                    case modalith::SurfaceType::ZernikeSag: return "Zernike";
                    case modalith::SurfaceType::CoordinateBreak: return "Coord Break";
                    case modalith::SurfaceType::Mirror: return "Mirror";
                    case modalith::SurfaceType::Biconic: return "Biconic";
                    default: return "Unknown";
                }
            }
            case ColRadius:
                return std::isinf(surface.profile.radius) ? QString("Infinity") : QVariant(surface.profile.radius);
            case ColThickness: {
                if (row == m_system->surfaces.size() - 1) return QVariant(""); // Image has no thickness
                double current_z = surface.transform.translation.z;
                double next_z = m_system->surfaces[row + 1].transform.translation.z;
                return QVariant(next_z - current_z);
            }
            case ColMaterial:
                return QString::fromStdString(surface.material_after ? std::string(surface.material_after->name()) : "Air");
            case ColSemiDiameter:
                return std::isinf(surface.semi_diameter) ? QString("Auto") : QVariant(surface.semi_diameter);
            case ColConic:
                return QVariant(surface.profile.conic_constant);
            default:
                return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        if (surface.stop) {
            return QBrush(QColor(40, 70, 40)); // Dark green for STOP
        }
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    return QVariant();
}

bool LensDataModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::EditRole || !m_system) return false;

    int row = index.row();
    auto& surface = m_system->surfaces[row];
    bool ok = false;

    switch (index.column()) {
        case ColRadius: {
            QString str = value.toString().toLower();
            if (str == "inf" || str == "infinity") {
                surface.profile.radius = std::numeric_limits<double>::infinity();
            } else {
                double val = value.toDouble(&ok);
                if (ok) surface.profile.radius = val;
                else return false;
            }
            emit dataChanged(index, index, {Qt::DisplayRole});
            return true;
        }
        case ColThickness: {
            if (row == m_system->surfaces.size() - 1) return false;
            double val = value.toDouble(&ok);
            if (!ok) return false;
            
            double current_z = surface.transform.translation.z;
            double old_next_z = m_system->surfaces[row + 1].transform.translation.z;
            double old_thickness = old_next_z - current_z;
            double diff = val - old_thickness;
            
            // Shift all subsequent surfaces
            for (size_t i = row + 1; i < m_system->surfaces.size(); ++i) {
                m_system->surfaces[i].transform.translation.z += diff;
            }
            emit dataChanged(index, index, {Qt::DisplayRole});
            // Emit data changed for all subsequent thicknesses? Technically thickness of next is same, but their Z changed.
            return true;
        }
        case ColSemiDiameter: {
            QString str = value.toString().toLower();
            if (str == "auto") {
                surface.semi_diameter = std::numeric_limits<double>::infinity();
            } else {
                double val = value.toDouble(&ok);
                if (ok) surface.semi_diameter = val;
                else return false;
            }
            emit dataChanged(index, index, {Qt::DisplayRole});
            return true;
        }
        case ColConic: {
            double val = value.toDouble(&ok);
            if (ok) surface.profile.conic_constant = val;
            else return false;
            emit dataChanged(index, index, {Qt::DisplayRole});
            return true;
        }
    }
    return false;
}

QVariant LensDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case ColSurface: return "Surface";
            case ColType: return "Type";
            case ColRadius: return "Radius";
            case ColThickness: return "Thickness";
            case ColMaterial: return "Glass";
            case ColSemiDiameter: return "Semi-Diameter";
            case ColConic: return "Conic";
            default: return QVariant();
        }
    } else {
        return QString::number(section);
    }
}

Qt::ItemFlags LensDataModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    
    Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    // Allow editing specific columns
    if (index.column() == ColRadius || index.column() == ColThickness || 
        index.column() == ColSemiDiameter || index.column() == ColConic) {
        return defaultFlags | Qt::ItemIsEditable;
    }
    
    return defaultFlags;
}

void LensDataModel::updateSystem() {
    beginResetModel();
    endResetModel();
}
