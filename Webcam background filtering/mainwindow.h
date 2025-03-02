#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
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
    void on_pushButton_camera_clicked();
    void on_pushButton_blur_clicked();
    void on_horizontalSlider_blur_valueChanged(int value);
    void processFrame();
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void initializeCamera();
    void updateFrame(const cv::Mat& frame);
    QImage cvMatToQImage(const cv::Mat& mat);
    cv::Mat applyBackgroundBlur(const cv::Mat& frame);
    void preprocessMask(cv::Mat& mask);
    cv::Mat refinePersonMask(const cv::Mat& frame, cv::Mat& mask);

    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QTimer *timer;
    cv::VideoCapture camera;
    cv::Ptr<cv::BackgroundSubtractorMOG2> bgSubtractor;
    bool isBlurEnabled;
    bool isCameraOn;
    int blurAmount;
    cv::Mat lastFrame;
    cv::Mat lastMask;
    bool isFirstFrame;
    const int HISTORY_FRAMES = 60;
    const double LEARNING_RATE = 0.001;
};
#endif // MAINWINDOW_H
