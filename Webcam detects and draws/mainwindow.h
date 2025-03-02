#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QColorDialog>
#include <QKeyEvent>
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

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateWebcamFrame();
    void onDetectColorClicked();
    void onDrawingModeToggled();
    void onSelectColorClicked();
    void onClearCanvasClicked();
    void onBrushSizeChanged(int size);

private:
    cv::Mat createColorMask(const cv::Mat& frame);
    void setupWebcam();
    void processColorTracking();
    void debugShowMask(const cv::Mat& mask);
    QImage convertMatToQImage(const cv::Mat& mat);

    Ui::MainWindow *ui;
    QGraphicsScene *cameraScene;
    cv::VideoCapture *webcam;
    QTimer *frameTimer;

    cv::Mat currentFrame;
    cv::Mat drawingCanvas;
    cv::Mat displayFrame;
    cv::Scalar detectedColor;

    bool isColorDetectionMode;
    bool isDrawingMode;

    cv::Scalar selectedColor;
    cv::Scalar colorLowerBound;
    cv::Scalar colorUpperBound;
    cv::Scalar targetColor;

    int brushSize;
    cv::Point lastTrackedPoint;
    bool isTracking;
};
#endif
