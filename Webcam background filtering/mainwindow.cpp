#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QStyle>
#include <QDebug>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , timer(new QTimer(this))
    , isBlurEnabled(false)
    , isCameraOn(false)
    , blurAmount(21)
    , isFirstFrame(true)
{
    ui->setupUi(this);
    setupUI();
    connect(timer, &QTimer::timeout, this, &MainWindow::processFrame);

    setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");
    QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
    qDebug() << "Setting window icon from:" << iconPath;
    setWindowIcon(QIcon(iconPath));

    // Initialize background subtractor with optimized parameters
    bgSubtractor = cv::createBackgroundSubtractorMOG2(HISTORY_FRAMES, 16, true);
    bgSubtractor->setBackgroundRatio(0.7);
    bgSubtractor->setNMixtures(5);
    bgSubtractor->setVarThreshold(16);
}

MainWindow::~MainWindow()
{
    if (camera.isOpened()) {
        camera.release();
    }
    timer->stop();
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("Real-time Background Blur");

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->pushButton_camera->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->pushButton_camera->setToolTip("Start/Stop Camera");

    ui->pushButton_blur->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
    ui->pushButton_blur->setToolTip("Toggle Background Blur");
    ui->pushButton_blur->setEnabled(false);

    ui->horizontalSlider_blur->setRange(1, 99);
    ui->horizontalSlider_blur->setValue(21);
    ui->horizontalSlider_blur->setEnabled(false);
}

void MainWindow::initializeCamera()
{
    if (!camera.open(0)) {
        QMessageBox::critical(this, "Error", "Could not open camera");
        return;
    }

    camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    camera.set(cv::CAP_PROP_FPS, 30);

    ui->pushButton_blur->setEnabled(true);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (!lastFrame.empty()) {
        updateFrame(lastFrame);
    }
}

void MainWindow::preprocessMask(cv::Mat& mask)
{
    cv::Mat kernel3 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::Mat kernel5 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));

    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel5);
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel3);
    cv::dilate(mask, mask, kernel3, cv::Point(-1, -1), 2);
    cv::GaussianBlur(mask, mask, cv::Size(5, 5), 0);
}

cv::Mat MainWindow::refinePersonMask(const cv::Mat& frame, cv::Mat& mask)
{
    cv::Mat refinedMask = mask.clone();

    // Convert to HSV for better skin detection
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // Detect skin tones
    cv::Mat skinMask;
    cv::inRange(hsv, cv::Scalar(0, 20, 70), cv::Scalar(20, 255, 255), skinMask);

    // Combine masks
    cv::bitwise_or(refinedMask, skinMask, refinedMask);

    // Clean up the mask
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(refinedMask, refinedMask, cv::MORPH_CLOSE, kernel);

    // Keep only large components
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(refinedMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    refinedMask = cv::Mat::zeros(refinedMask.size(), CV_8UC1);
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) > 1000) {
            cv::drawContours(refinedMask, std::vector<std::vector<cv::Point>>{contour}, 0, 255, -1);
        }
    }

    return refinedMask;
}

cv::Mat MainWindow::applyBackgroundBlur(const cv::Mat& frame)
{
    cv::Mat foregroundMask;

    // Apply background subtraction
    bgSubtractor->apply(frame, foregroundMask, isFirstFrame ? 1.0 : LEARNING_RATE);
    if (isFirstFrame) {
        isFirstFrame = false;
        lastMask = foregroundMask.clone();
        return frame;
    }

    // Convert mask to binary
    cv::threshold(foregroundMask, foregroundMask, 250, 255, cv::THRESH_BINARY);

    // Refine the mask
    foregroundMask = refinePersonMask(frame, foregroundMask);

    // Temporal smoothing
    cv::addWeighted(foregroundMask, 0.7, lastMask, 0.3, 0, foregroundMask);
    lastMask = foregroundMask.clone();

    // Process the mask
    preprocessMask(foregroundMask);

    // Create 3-channel mask
    std::vector<cv::Mat> channels = {foregroundMask, foregroundMask, foregroundMask};
    cv::Mat foregroundMask3Ch;
    cv::merge(channels, foregroundMask3Ch);
    foregroundMask3Ch.convertTo(foregroundMask3Ch, CV_8U);

    // Apply blur to background
    cv::Mat blurredFrame;
    cv::GaussianBlur(frame, blurredFrame, cv::Size(blurAmount, blurAmount), 0);

    // Combine foreground and blurred background
    cv::Mat resultFrame = frame.clone();
    cv::bitwise_and(frame, foregroundMask3Ch, resultFrame);
    cv::bitwise_not(foregroundMask3Ch, foregroundMask3Ch);
    cv::Mat blurredBackground;
    cv::bitwise_and(blurredFrame, foregroundMask3Ch, blurredBackground);
    cv::add(resultFrame, blurredBackground, resultFrame);

    return resultFrame;
}

void MainWindow::on_pushButton_camera_clicked()
{
    if (!isCameraOn) {
        initializeCamera();
        timer->start(33);
        ui->pushButton_camera->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
        isCameraOn = true;
    } else {
        timer->stop();
        camera.release();
        scene->clear();
        lastFrame = cv::Mat();
        ui->pushButton_camera->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        isCameraOn = false;
        ui->pushButton_blur->setEnabled(false);
        ui->horizontalSlider_blur->setEnabled(false);
        isBlurEnabled = false;
    }
}

void MainWindow::on_pushButton_blur_clicked()
{
    isBlurEnabled = !isBlurEnabled;
    ui->horizontalSlider_blur->setEnabled(isBlurEnabled);
    if (isBlurEnabled) {
        isFirstFrame = true;
        bgSubtractor = cv::createBackgroundSubtractorMOG2(HISTORY_FRAMES, 16, true);
        bgSubtractor->setBackgroundRatio(0.7);
        bgSubtractor->setNMixtures(5);
        bgSubtractor->setVarThreshold(16);
    }
}

void MainWindow::on_horizontalSlider_blur_valueChanged(int value)
{
    blurAmount = value * 2 + 1;
}

void MainWindow::processFrame()
{
    cv::Mat frame;
    if (!camera.read(frame)) {
        timer->stop();
        QMessageBox::warning(this, "Error", "Could not read frame from camera");
        return;
    }

    if (isBlurEnabled) {
        frame = applyBackgroundBlur(frame);
    }

    lastFrame = frame.clone();
    updateFrame(frame);
}

void MainWindow::updateFrame(const cv::Mat& frame)
{
    QImage qImage = cvMatToQImage(frame);
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(qImage));
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

QImage MainWindow::cvMatToQImage(const cv::Mat& mat)
{
    if (mat.empty())
        return QImage();

    if (mat.type() == CV_8UC3)
    {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows,
                      static_cast<int>(rgb.step),
                      QImage::Format_RGB888).copy();
    }
    return QImage();
}
