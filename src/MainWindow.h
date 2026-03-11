#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include "GitManager.h"
#include "DiffView.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void refreshStatus();
    void onFileSelected(QListWidgetItem *item);
    void stageSelected();
    void unstageSelected();
    void commitChanges();
    void pushChanges();
    void pullChanges();

private:
    GitManager *gitManager;
    QListWidget *stagedList;
    QListWidget *unstagedList;
    DiffView *diffView;
    QLineEdit *commitMessageEdit;
    
    void setupUi();
};

#endif // MAINWINDOW_H
