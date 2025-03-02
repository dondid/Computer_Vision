#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
, ui(new Ui::MainWindow)
, scene(new CustomGraphicsScene(this))
, backgroundColor(255, 255, 255)
, useBackgroundImage(false)
, thresholdValue(128)
, isSelectingROI(false)
, roiRect(nullptr)
, hasROI(false)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(scene);

    setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");
    QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
    qDebug() << "Setting window icon from:" << iconPath;
    setWindowIcon(QIcon(iconPath));

    ui->horizontalSlider_threshold->setRange(0, 255);
    ui->horizontalSlider_threshold->setValue(thresholdValue);

    connect(scene, &CustomGraphicsScene::mousePressed, this, &MainWindow::handleMousePress);
    connect(scene, &CustomGraphicsScene::mouseMoved, this, &MainWindow::handleMouseMove);
    connect(scene, &CustomGraphicsScene::mouseReleased, this, &MainWindow::handleMouseRelease);
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

void MainWindow::handleMousePress(QPointF point)
{
    if (!isSelectingROI || originalImage.empty()) return;

    roiStartPoint = point;
    if (roiRect) {
        scene->removeItem(static_cast<QGraphicsItem*>(roiRect));
        delete roiRect;
    }
    roiRect = new QGraphicsRectItem();
    roiRect->setPen(QPen(Qt::red));
    roiRect->setRect(QRectF(point, point));
    scene->addItem(roiRect);
}

void MainWindow::handleMouseMove(QPointF point)
{
    if (!isSelectingROI || !roiRect) return;

    roiEndPoint = point;
    QRectF rect(roiStartPoint, roiEndPoint);
    roiRect->setRect(rect.normalized());
}

void MainWindow::handleMouseRelease(QPointF point)
{
    if (!isSelectingROI || !roiRect) return;

    roiEndPoint = point;
    QRectF rect = roiRect->rect();

    double scaleX = originalImage.cols / static_cast<double>(scene->width());
    double scaleY = originalImage.rows / static_cast<double>(scene->height());

    int x = std::max(0, static_cast<int>(rect.x() * scaleX));
    int y = std::max(0, static_cast<int>(rect.y() * scaleY));
    int width = std::min(originalImage.cols - x, static_cast<int>(rect.width() * scaleX));
    int height = std::min(originalImage.rows - y, static_cast<int>(rect.height() * scaleY));

    cv::Rect roiRect(x, y, width, height);

    roiMask = cv::Mat::zeros(originalImage.size(), CV_8UC1);
    cv::rectangle(roiMask, roiRect, cv::Scalar(255), -1);

    hasROI = true;
    isSelectingROI = false;
    updateImage();
}

void MainWindow::on_pushButton_selectROI_clicked()
{
    if (originalImage.empty()) {
        QMessageBox::warning(this, tr("Eroare"), tr("Mai întâi încărcați o imagine."));
        return;
    }

    isSelectingROI = true;
    if (roiRect) {
        scene->removeItem(static_cast<QGraphicsItem*>(roiRect));
        delete roiRect;
        roiRect = nullptr;
    }

    QMessageBox::information(this, tr("Selectare ROI"),
                             tr("Faceți clic și trageți pentru a selecta regiunea de interes."));
}

//Abordarea 1.
//aici am abortat astfel: punctul de interes ramane ca in imaginea initiala,
//iar restul imagini poate fi modificat in functie de cerinta utilizatorului
void MainWindow::updateImage()
{
    if (originalImage.empty()) return;

    result = originalImage.clone();

    if (hasROI) {
        if (useBackgroundImage && !backgroundImage.empty()) {
            cv::Mat resizedBackground;
            cv::resize(backgroundImage, resizedBackground, originalImage.size());
            cv::Mat invertedROIMask;
            cv::bitwise_not(roiMask, invertedROIMask);

            resizedBackground.copyTo(result, invertedROIMask);
        } else {
            cv::Mat colorBackground(originalImage.size(), CV_8UC3, backgroundColor);
            cv::Mat invertedROIMask;
            cv::bitwise_not(roiMask, invertedROIMask);

            colorBackground.copyTo(result, invertedROIMask);
        }
    } else {
        if (useBackgroundImage && !backgroundImage.empty()) {
            cv::Mat resizedBackground;
            cv::resize(backgroundImage, resizedBackground, originalImage.size());
            resizedBackground.copyTo(result);
        } else {
            cv::Mat colorBackground(originalImage.size(), CV_8UC3, backgroundColor);
            colorBackground.copyTo(result);
        }
    }

    cv::Mat grayImage, thresholdMask;
    cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);
    cv::threshold(grayImage, thresholdMask, thresholdValue, 255, cv::THRESH_BINARY_INV);

    originalImage.copyTo(result, thresholdMask);

    QImage qImage = cvMatToQImage(result);
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(qImage));

    if (isSelectingROI && roiRect) {
        roiRect->setRect(QRectF(roiStartPoint, roiEndPoint));
        scene->addItem(roiRect);
    }

    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

//Abordarea 2.
//aici am abordat astfel: dupa alegerea punctului de intereres putem efectua modificari doar asupra acestuia,
//restul imagini fiind continuat de fundalul ales
/*
void MainWindow::updateImage()
{
    if (originalImage.empty()) return;

    cv::Mat grayImage, mask;
    cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);
    cv::threshold(grayImage, mask, thresholdValue, 255, cv::THRESH_BINARY_INV);

    if (hasROI) {
        mask = mask & roiMask;
    }

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

    if (isSelectingROI && roiRect) {
        roiRect = new QGraphicsRectItem();
        roiRect->setPen(QPen(Qt::red));
        roiRect->setRect(QRectF(roiStartPoint, roiEndPoint));
        scene->addItem(roiRect);
    }

    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}*/

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
    else if (mat.type() == CV_8UC4)
    {
        return QImage(mat.data, mat.cols, mat.rows,
                      static_cast<int>(mat.step),
                      QImage::Format_ARGB32).copy();
    }
    else if (mat.type() == CV_8UC1)
    {
        return QImage(mat.data, mat.cols, mat.rows,
                      static_cast<int>(mat.step),
                      QImage::Format_Grayscale8).copy();
    }

    return QImage();
}
