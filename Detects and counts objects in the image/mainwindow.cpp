#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , originalScene(new QGraphicsScene(this))
    , processedScene(new QGraphicsScene(this))
    , thresholdValue(100)
    , blurAmount(5)
    , minContourArea(500.0)
    , morphKernelSize(3)
{
    ui->setupUi(this);

    ui->graphicsView_original->setScene(originalScene);
    ui->graphicsView_processed->setScene(processedScene);

    ui->horizontalSlider_threshold->setRange(0, 255);
    ui->horizontalSlider_threshold->setValue(thresholdValue);
    connect(ui->horizontalSlider_threshold, &QSlider::valueChanged, this, &MainWindow::processAndCountObjects);

    ui->horizontalSlider_blur->setRange(1, 15);
    ui->horizontalSlider_blur->setValue(blurAmount);
    connect(ui->horizontalSlider_blur, &QSlider::valueChanged, this, &MainWindow::processAndCountObjects);

    ui->horizontalSlider_minArea->setRange(100, 2000);
    ui->horizontalSlider_minArea->setValue(minContourArea);
    connect(ui->horizontalSlider_minArea, &QSlider::valueChanged, this, &MainWindow::processAndCountObjects);

    ui->horizontalSlider_morphValue->setRange(1, 10);
    ui->horizontalSlider_morphValue->setValue(morphKernelSize);
    connect(ui->horizontalSlider_morphValue, &QSlider::valueChanged, this, &MainWindow::processAndCountObjects);

    setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");
    QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
    qDebug() << "Setting window icon from:" << iconPath;
    setWindowIcon(QIcon(iconPath));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_loadImage_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Load Image", "", "Image Files (*.png *.jpg *.bmp)");

    if (filename.isEmpty()) return;

    originalImage = cv::imread(filename.toStdString());

    if (originalImage.empty()) {
        QMessageBox::warning(this, "Error", "Could not load image");
        return;
    }

    cv::Mat rgbImage;
    cv::cvtColor(originalImage, rgbImage, cv::COLOR_BGR2RGB);
    QImage qimg(rgbImage.data, rgbImage.cols, rgbImage.rows,
                rgbImage.step, QImage::Format_RGB888);

    originalScene->clear();
    originalScene->addPixmap(QPixmap::fromImage(qimg));
    ui->graphicsView_original->fitInView(originalScene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::processAndCountObjects()
{
    if (originalImage.empty()) {
        QMessageBox::warning(this, "Error", "Load an image first");
        return;
    }

    cv::Mat preprocessed = preprocessImage(originalImage);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(preprocessed, contours, hierarchy,
                     cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> filteredContours;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) >= minContourArea) {
            filteredContours.push_back(contour);
        }
    }

    cv::Mat result = cv::Mat::zeros(originalImage.size(), originalImage.type());

    for (size_t i = 0; i < filteredContours.size(); i++) {
        cv::Scalar color(rand() & 255, rand() & 255, rand() & 255);

        cv::drawContours(result, filteredContours, i, color, cv::FILLED);

        cv::Moments m = cv::moments(filteredContours[i]);

        cv::Point center(m.m10 / m.m00, m.m01 / m.m00);

        cv::putText(result, std::to_string(i + 1), center,
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
    }


    processedImage = result;
    updateProcessedImage();

    ui->label_objectCount->setText(QString("Objects Detected: %1").arg(filteredContours.size()));
}

cv::Mat MainWindow::preprocessImage(const cv::Mat& inputImage)
{
    cv::Mat gray, blurred, thresholded;

    cv::cvtColor(inputImage, gray, cv::COLOR_BGR2GRAY);

    cv::GaussianBlur(gray, blurred, cv::Size(blurAmount * 2 + 1, blurAmount * 2 + 1), 0);

    cv::threshold(blurred, thresholded, thresholdValue, 255, cv::THRESH_BINARY_INV);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(morphKernelSize, morphKernelSize));
    cv::morphologyEx(thresholded, thresholded, cv::MORPH_CLOSE, kernel);

    return thresholded;
}

void MainWindow::updateProcessedImage()
{
    if (processedImage.empty()) return;

    cv::Mat rgbImage;
    cv::cvtColor(processedImage, rgbImage, cv::COLOR_BGR2RGB);
    QImage qimg(rgbImage.data, rgbImage.cols, rgbImage.rows,
                rgbImage.step, QImage::Format_RGB888);

    processedScene->clear();
    processedScene->addPixmap(QPixmap::fromImage(qimg));
    ui->graphicsView_processed->fitInView(processedScene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::on_pushButton_processImage_clicked()
{
    processAndCountObjects();
}

void MainWindow::on_pushButton_saveImage_clicked()
{
    if (processedImage.empty()) {
        QMessageBox::warning(this, "Error", "Process an image first");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Save Processed Image", "", "Image Files (*.png *.jpg *.bmp)");

    if (filename.isEmpty()) return;

    cv::imwrite(filename.toStdString(), processedImage);
}

void MainWindow::on_horizontalSlider_threshold_valueChanged(int value)
{
    thresholdValue = value;
}

void MainWindow::on_horizontalSlider_blur_valueChanged(int value)
{
    blurAmount = value;
}

void MainWindow::on_horizontalSlider_minArea_valueChanged(int value)
{
    minContourArea = value;
}

void MainWindow::on_horizontalSlider_morphValue_valueChanged(int value)
{
    morphKernelSize = value;
}

