#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QColorDialog>
#include <QFileDialog>
#include <opencv2/opencv.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_loadImage_clicked();
    void on_pushButton_backgroundImage_clicked();
    void on_pushButton_backgroundColor_clicked();
    void on_horizontalSlider_threshold_valueChanged(int value);
    void on_pushButton_saveImage_clicked();

private:
    void updateImage();
    QImage cvMatToQImage(const cv::Mat& mat);

    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    cv::Mat originalImage;
    cv::Mat backgroundImage;
    cv::Mat result;
    cv::Scalar backgroundColor;
    bool useBackgroundImage;
    int thresholdValue;

};

#endif
