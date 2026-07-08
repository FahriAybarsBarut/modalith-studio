#include <QApplication>
#include <QFile>
#include <QString>
#include "gui/main_window.hpp"

int main(int argc, char* argv[]) {
    // Enable high DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    QApplication app(argc, argv);
    
    app.setApplicationName("Modalith Studio");
    app.setApplicationVersion("1.0.0");
    
    // Load QSS stylesheet
    QFile file("resources/modern_theme.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
    }
    
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
