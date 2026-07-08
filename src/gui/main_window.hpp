#pragma once

#include <QMainWindow>
#include <QTreeView>
#include <QTableView>
#include <QMenu>
#include <QToolBar>
#include <QTabWidget>

#include "modalith/core/sequential_tracer.hpp"
#include "lens_data_model.hpp"
#include "lens_layout_view.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void setupUi();
    void createMenus();
    void createToolBars();
    void loadSampleSystem();

    QTreeView* m_propertiesTree{nullptr};
    QTableView* m_lensDataEditor{nullptr};
    QTabWidget* m_centralTabs{nullptr};
    
    LensLayoutView* m_layoutView{nullptr};
    LensDataModel* m_lensModel{nullptr};

    QMenu* m_fileMenu{nullptr};
    QMenu* m_viewMenu{nullptr};
    QMenu* m_helpMenu{nullptr};

    QToolBar* m_mainToolBar{nullptr};
    
    modalith::OpticalSystem m_system;
};
