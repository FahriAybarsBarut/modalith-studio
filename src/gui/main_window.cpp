#include "main_window.hpp"
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QHeaderView>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    loadSampleSystem();

    setupUi();
    createMenus();
    createToolBars();
    
    setWindowTitle("Modalith Studio - Premium Edition");
    resize(1280, 800);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    // Central widget: Tab widget for Layout, Spot Diagram, etc.
    m_centralTabs = new QTabWidget(this);
    m_layoutView = new LensLayoutView(this);
    m_layoutView->setSystem(&m_system);
    m_centralTabs->addTab(m_layoutView, "2D Lens Layout");
    
    // Placeholder for other tabs
    m_centralTabs->addTab(new QWidget(), "Spot Diagram");
    setCentralWidget(m_centralTabs);
    
    // Left dock widget: Properties tree
    QDockWidget* leftDock = new QDockWidget("System Explorer", this);
    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propertiesTree = new QTreeView(leftDock);
    leftDock->setWidget(m_propertiesTree);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    // Bottom dock widget: Lens Data Editor
    QDockWidget* bottomDock = new QDockWidget("Lens Data Editor (LDE)", this);
    bottomDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    m_lensDataEditor = new QTableView(bottomDock);
    
    m_lensModel = new LensDataModel(&m_system, this);
    m_lensDataEditor->setModel(m_lensModel);
    
    // Connect model changes to layout updates
    connect(m_lensModel, &QAbstractTableModel::dataChanged, this, [this]() {
        m_layoutView->updateLayout();
    });

    // Make it look like a spreadsheet
    m_lensDataEditor->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_lensDataEditor->setAlternatingRowColors(true);
    bottomDock->setWidget(m_lensDataEditor);
    addDockWidget(Qt::BottomDockWidgetArea, bottomDock);

    // Status bar
    statusBar()->showMessage("Ready");
}

void MainWindow::createMenus() {
    m_fileMenu = menuBar()->addMenu("&File");
    m_fileMenu->addAction("&New");
    m_fileMenu->addAction("&Open...");
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("E&xit", this, &QWidget::close);

    m_viewMenu = menuBar()->addMenu("&View");

    m_helpMenu = menuBar()->addMenu("&Help");
    m_helpMenu->addAction("&About");
}

void MainWindow::createToolBars() {
    m_mainToolBar = addToolBar("Main Toolbar");
    m_mainToolBar->addAction("New");
    m_mainToolBar->addAction("Open");
    m_mainToolBar->addAction("Save");
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction("Update UI", this, [this]() {
        m_layoutView->updateLayout();
    });
}

void MainWindow::loadSampleSystem() {
    // Create a dummy singlet lens for demonstration
    m_system.title = "Sample Singlet Lens";
    
    modalith::OpticalSurface obj;
    obj.label = "OBJ";
    obj.transform.translation.z = -50.0;
    
    modalith::OpticalSurface front;
    front.label = "Front";
    front.profile.type = modalith::SurfaceType::Sphere;
    front.profile.radius = 50.0;
    front.transform.translation.z = 0.0;
    front.semi_diameter = 15.0;
    modalith::MaterialCatalog catalog;
    front.material_after = catalog.find("N-BK7");
    
    modalith::OpticalSurface back;
    back.label = "Back";
    back.profile.type = modalith::SurfaceType::Sphere;
    back.profile.radius = -100.0;
    back.transform.translation.z = 10.0;
    back.semi_diameter = 15.0;
    
    modalith::OpticalSurface ima;
    ima.label = "IMA";
    ima.transform.translation.z = 80.0;
    
    m_system.surfaces = {obj, front, back, ima};
}
