#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QColorDialog>
#include <QFileDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_importImage_clicked();
    void on_pushButton_addText_clicked();
    void on_pushButton_deleteSelected_clicked();
    void on_pushButton_saveImage_clicked();
    void on_pushButton_selectColor_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
};

#endif