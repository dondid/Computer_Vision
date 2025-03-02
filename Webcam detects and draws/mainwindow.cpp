#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QColorDialog>
#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , cameraScene(new QGraphicsScene(this))
    , webcam(new cv::VideoCapture(0))
    , frameTimer(new QTimer(this))
    , isColorDetectionMode(false)
    , isDrawingMode(false)
    , selectedColor(cv::Scalar(0, 255, 0))
    , brushSize(5)
    , isTracking(false)
{
    ui->setupUi(this);
    setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");
    QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
    qDebug() << "Setting window icon from:" << iconPath;
    setWindowIcon(QIcon(iconPath));

    ui->graphicsView_camera->setScene(cameraScene);

    setupWebcam();

    connect(frameTimer, &QTimer::timeout, this, &MainWindow::updateWebcamFrame);
    frameTimer->start(30);

    connect(ui->btnDetectColor, &QPushButton::clicked, this, &MainWindow::onDetectColorClicked);
    connect(ui->btnSelectDrawingColor, &QPushButton::clicked, this, &MainWindow::onSelectColorClicked);
    connect(ui->btnDrawing, &QPushButton::clicked, this, &MainWindow::onDrawingModeToggled);
    connect(ui->btnClearCanvas, &QPushButton::clicked, this, &MainWindow::onClearCanvasClicked);
    connect(ui->sliderBrushSize, &QSlider::valueChanged, this, &MainWindow::onBrushSizeChanged);

    drawingCanvas = cv::Mat::zeros(480, 640, CV_8UC3);
}

void MainWindow::setupWebcam()
{
    if (!webcam->isOpened()) {
        QMessageBox::critical(this, "Eroare Cameră", "Nu se poate deschide camera web. Verificați:\n");
        return;
    }

    webcam->set(cv::CAP_PROP_FRAME_WIDTH, 640);
    webcam->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
}

void MainWindow::updateWebcamFrame()
{
    if (!webcam->read(currentFrame)) return;

    cv::addWeighted(currentFrame, 0.7, drawingCanvas, 0.3, 0, displayFrame);

    if (isColorDetectionMode && isDrawingMode) {
        processColorTracking();
    }

    cv::cvtColor(displayFrame, displayFrame, cv::COLOR_BGR2RGB);
    QImage qimg(displayFrame.data, displayFrame.cols, displayFrame.rows,
                displayFrame.step, QImage::Format_RGB888);

    cameraScene->clear();
    cameraScene->addPixmap(QPixmap::fromImage(qimg));
    ui->graphicsView_camera->fitInView(cameraScene->sceneRect(), Qt::KeepAspectRatio);
}

cv::Mat MainWindow::createColorMask(const cv::Mat& frame)
{
    cv::Mat hsvFrame;
    cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);
    cv::Mat mask;
    cv::inRange(hsvFrame, cv::Scalar(colorLowerBound[0], 50, 50), cv::Scalar(colorUpperBound[0], 255, 255), mask);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::erode(mask, mask, kernel, cv::Point(-1, -1), 2);
    cv::dilate(mask, mask, kernel, cv::Point(-1, -1), 2);

    return mask;
}

void MainWindow::processColorTracking()
{
    cv::Mat mask = createColorMask(currentFrame);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        qDebug() << "Nu s-au găsit contururi.";
        isTracking = false;
        return;
    }

    auto largestContour = *std::max_element(
        contours.begin(), contours.end(),
        [](const auto &a, const auto &b) { return cv::contourArea(a) < cv::contourArea(b); });

    if (cv::contourArea(largestContour) > 100) {
        qDebug() << "S-a detectat un contur valid.";

        cv::Moments m = cv::moments(largestContour);
        cv::Point centroid(m.m10 / m.m00, m.m01 / m.m00);

        cv::drawContours(displayFrame, std::vector<std::vector<cv::Point>>{largestContour}, -1,
                         cv::Scalar(0, 255, 255), 2);

        cv::circle(displayFrame, centroid, 5, cv::Scalar(0, 0, 255), -1);

        if (isTracking) {
            cv::line(drawingCanvas, lastTrackedPoint, centroid, selectedColor, brushSize);
        }

        lastTrackedPoint = centroid;
        isTracking = true;
    } else {
        qDebug() << "Conturul detectat este prea mic.";
        isTracking = false;
    }
}

void MainWindow::onDetectColorClicked()
{
    if (!isColorDetectionMode) {
        cv::Mat hsvFrame;
        cv::cvtColor(currentFrame, hsvFrame, cv::COLOR_BGR2HSV);

        cv::Rect roi(currentFrame.cols/4, currentFrame.rows/4,
                     currentFrame.cols/2, currentFrame.rows/2);
        cv::Mat selectedRegion = hsvFrame(roi);

        cv::Scalar meanColor, stdColor;
        cv::meanStdDev(selectedRegion, meanColor, stdColor);

        colorLowerBound = cv::Scalar(std::max(0.0, meanColor[0] - 10), 50, 50);
        colorUpperBound = cv::Scalar(std::min(180.0, meanColor[0] + 10), 255, 255);

        QMessageBox::information(this, "Detecție", "Culoarea a fost detectată. Mișcați obiectul pentru a desena.");
    }

    isColorDetectionMode = !isColorDetectionMode;
}

void MainWindow::onSelectColorClicked()
{
    QColor color = QColorDialog::getColor(Qt::green, this, "Culoare");
    if (color.isValid()) {
        selectedColor = cv::Scalar(color.blue(), color.green(), color.red());
        QMessageBox::information(this, "Culoare selectată", QString("Culoarea selectată: R=%1, G=%2, B=%3")
                                                                .arg(color.red()).arg(color.green()).arg(color.blue()));
    }
}

void MainWindow::onClearCanvasClicked()
{
    drawingCanvas = cv::Mat::zeros(drawingCanvas.size(), drawingCanvas.type());
}

void MainWindow::onDrawingModeToggled()
{
    isDrawingMode = !isDrawingMode;
    isTracking = false;
    QString statusMessage = isDrawingMode ? "Mod desen activat" : "Mod desen dezactivat";
    ui->statusBar->showMessage(statusMessage, 2000);
}

void MainWindow::onBrushSizeChanged(int size)
{
    brushSize = size;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_S) {
        onDrawingModeToggled();
    }

    QMainWindow::keyPressEvent(event);
}

MainWindow::~MainWindow()
{
    if (webcam) {
        webcam->release();
        delete webcam;
    }
    delete ui;
}
