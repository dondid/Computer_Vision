#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_loadImage_clicked();
    void on_pushButton_processImage_clicked();
    void on_pushButton_saveImage_clicked();
    void on_horizontalSlider_threshold_valueChanged(int value);
    void on_horizontalSlider_blur_valueChanged(int value);
    void on_horizontalSlider_minArea_valueChanged(int value);
    void on_horizontalSlider_morphValue_valueChanged(int value);

private:
    void processAndCountObjects();
    void updateProcessedImage();
    QImage cvMatToQImage(const cv::Mat& mat);
    cv::Mat preprocessImage(const cv::Mat& inputImage);

    Ui::MainWindow *ui;
    QGraphicsScene *originalScene;
    QGraphicsScene *processedScene;

    cv::Mat originalImage;
    cv::Mat processedImage;

    int thresholdValue;
    int blurAmount;
    double minContourArea;
    int morphKernelSize;
};

#endif
