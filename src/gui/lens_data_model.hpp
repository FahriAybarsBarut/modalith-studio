#pragma once

#include <QAbstractTableModel>
#include "modalith/core/sequential_tracer.hpp"

class LensDataModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        ColSurface = 0,
        ColType,
        ColRadius,
        ColThickness,
        ColMaterial,
        ColSemiDiameter,
        ColConic,
        ColumnCount
    };

    explicit LensDataModel(modalith::OpticalSystem* system, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void updateSystem(); // Call this when the underlying system changes externally

private:
    modalith::OpticalSystem* m_system;
};
