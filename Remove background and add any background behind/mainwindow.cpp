#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , backgroundColor(255, 255, 255)
    , useBackgroundImage(false)
    , thresholdValue(128)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(scene);

    setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");
    QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
    qDebug() << "Setting window icon from:" << iconPath;
    setWindowIcon(QIcon(iconPath));

    ui->horizontalSlider_threshold->setRange(0, 255);
    ui->horizontalSlider_threshold->setValue(thresholdValue);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_loadImage_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Încarcă Imagine"), "", tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    if (!fileName.isEmpty()) {
        originalImage = cv::imread(fileName.toStdString());
        if (!originalImage.empty()) {
            updateImage();
        } else {
            QMessageBox::warning(this, tr("Eroare"),
                                 tr("Nu s-a putut încărca imaginea."));
        }
    }
}

void MainWindow::on_pushButton_backgroundImage_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Alege Imagine Fundal"), "", tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    if (!fileName.isEmpty()) {
        backgroundImage = cv::imread(fileName.toStdString());
        if (!backgroundImage.empty()) {
            useBackgroundImage = true;
            updateImage();
        } else {
            QMessageBox::warning(this, tr("Eroare"), tr("Nu s-a putut încărca imaginea de fundal."));
        }
    }
}

void MainWindow::on_pushButton_backgroundColor_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Alege Culoare Fundal"));

    if (color.isValid()) {
        backgroundColor = cv::Scalar(color.blue(), color.green(), color.red());
        useBackgroundImage = false;
        updateImage();
    }
}

void MainWindow::on_pushButton_saveImage_clicked()
{
    if (result.empty()) {
        QMessageBox::warning(this, tr("Eroare"), tr("Nu există imagine de salvat."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Salvează Imagine"), "", tr("Image Files (*.png *.jpg *.jpeg)"));

    if (!fileName.isEmpty()) {
        cv::imwrite(fileName.toStdString(), result);
        QMessageBox::information(this, tr("Succes"), tr("Imaginea a fost salvată cu succes."));
    }
}


void MainWindow::on_horizontalSlider_threshold_valueChanged(int value)
{
    thresholdValue = value;
    updateImage();
}

void MainWindow::updateImage()
{
    if (originalImage.empty()) return;

    cv::Mat grayImage, mask;
    cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);
    cv::threshold(grayImage, mask, thresholdValue, 255, cv::THRESH_BINARY_INV);

    if (useBackgroundImage && !backgroundImage.empty()) {
        cv::Mat resizedBackground;
        cv::resize(backgroundImage, resizedBackground, originalImage.size());

        resizedBackground.copyTo(result);
        originalImage.copyTo(result, mask);
    } else {
        cv::Mat colorBackground(originalImage.size(), CV_8UC3, backgroundColor);
        colorBackground.copyTo(result);
        originalImage.copyTo(result, mask);
    }

    QImage qImage = cvMatToQImage(result);
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(qImage));
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

QImage MainWindow::cvMatToQImage(const cv::Mat& mat)
{
    if(mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage((const unsigned char*)rgb.data,
                      rgb.cols, rgb.rows,
                      rgb.step,
                      QImage::Format_RGB888).copy();
    }
    return QImage();
}
