#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setStyleSheet(
        "QMainWindow { background-color: #1e1e1e; color: #d4d4d4; }"
        "QWidget { background-color: #1e1e1e; color: #d4d4d4; }"
        "QSplitter::handle { background-color: #333; }"
        "QSplitter::handle:horizontal { width: 1px; }"
        "QSplitter::handle:vertical { height: 1px; }"
        "QListWidget { background-color: #252526; border: none; outline: none; border-radius: 4px; }"
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #2d2d2d; }"
        "QListWidget::item:selected { background-color: #37373d; color: #fff; border-radius: 4px; }"
        "QListWidget::item:hover { background-color: #2a2d2e; border-radius: 4px; }"
        "QPushButton { background-color: #333333; color: #cccccc; border: 1px solid #444444; padding: 6px 12px; border-radius: 4px; min-height: 20px; }"
        "QPushButton:hover { background-color: #444444; border: 1px solid #555555; }"
        "QPushButton:pressed { background-color: #222222; }"
        "QPushButton#primaryBtn { background-color: #007acc; color: white; border: none; }"
        "QPushButton#primaryBtn:hover { background-color: #0062a3; }"
        "QPushButton#secondaryBtn { background-color: #3a3d41; border: none; }"
        "QPushButton#secondaryBtn:hover { background-color: #45494e; }"
        "QLineEdit, QTextEdit, QComboBox { background-color: #3c3c3c; color: #cccccc; border: 1px solid #3c3c3c; padding: 5px; border-radius: 4px; }"
        "QLineEdit:focus, QTextEdit:focus, QComboBox:focus { border: 1px solid #007acc; }"
        "QLabel { color: #aaaaaa; font-size: 11px; font-weight: bold; text-transform: uppercase; margin-top: 10px; }"
        "QScrollBar:vertical { border: none; background: #1e1e1e; width: 10px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #424242; min-height: 20px; border-radius: 5px; }"
        "QScrollBar::handle:vertical:hover { background: #4f4f4f; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QTabBar::tab { background: #2d2d2d; color: #888; padding: 10px 16px; border-right: 1px solid #1e1e1e; border-bottom: 1px solid #1e1e1e; min-width: 100px; }"
        "QTabBar::tab:selected { background: #1e1e1e; color: #fff; border-bottom: 2px solid #007acc; }"
        "QTabBar::tab:hover { background: #333; }"
    );
    
    MainWindow window;
    window.resize(1024, 768);
    window.show();
    
    return app.exec();
}
