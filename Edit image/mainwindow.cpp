#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPainter>
#include <QColorDialog>
#include <QMouseEvent>
#include <QImageReader>
#include <QMessageBox>
#include <QIcon>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeUI();

    setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");

    QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
    qDebug() << "Setting window icon from:" << iconPath;
    setWindowIcon(QIcon(iconPath));

    connect(ui->btnLoadBackground, &QPushButton::clicked, this, &MainWindow::loadBackground);
    connect(ui->btnLoadOverlay, &QPushButton::clicked, this, &MainWindow::loadOverlay);
    connect(ui->btnPickColor, &QPushButton::clicked, this, &MainWindow::pickColor);
    connect(ui->btnRemoveBackground, &QPushButton::clicked, this, &MainWindow::removeBackground);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::deleteSelected);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveResult);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initializeUI() {
    backgroundLabel = new CustomLabel(this);
    backgroundLabel->setMinimumSize(600, 400);
    backgroundLabel->setAlignment(Qt::AlignCenter);
    backgroundLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid gray; }");

    overlayLabel = new CustomLabel(this);
    overlayLabel->setMinimumSize(100, 100);
    overlayLabel->setStyleSheet("QLabel { background: transparent; }");

    ui->imageContainer->addWidget(backgroundLabel);
    ui->imageContainer->addWidget(overlayLabel);
    overlayLabel->hide();
}

CustomLabel::CustomLabel(QWidget* parent)
    : QLabel(parent)
    , isDragging(false)
{
    setMouseTracking(true);
}

void CustomLabel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragStartPosition = event->pos();
        labelStartPosition = pos();
    }
}

void CustomLabel::mouseMoveEvent(QMouseEvent* event) {
    if (isDragging) {
        move(labelStartPosition + (event->pos() - dragStartPosition));
    }
}

void CustomLabel::mouseReleaseEvent(QMouseEvent*) {
    isDragging = false;
}

void CustomLabel::wheelEvent(QWheelEvent* event) {
    QSize currentSize = pixmap().size();

    if (event->angleDelta().y() > 0) {
        QSize newSize = currentSize * 1.1;
        setPixmap(pixmap().scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QSize newSize = currentSize * 0.9;
        setPixmap(pixmap().scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}




void MainWindow::loadBackground() {
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("Open Background Image"), QString(), tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (!filePath.isEmpty()) {
        QImageReader reader(filePath);
        reader.setAutoTransform(true);

        QImage image = reader.read();
        if (image.isNull()) {
            QMessageBox::warning(this, tr("Error"), tr("Could not load image."));
            return;
        }

        backgroundPixmap = QPixmap::fromImage(image);
        backgroundLabel->setPixmap(backgroundPixmap.scaled(backgroundLabel->size(),
        Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void MainWindow::loadOverlay() {
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("Open Overlay Image"), QString(), tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (!filePath.isEmpty()) {
        QImageReader reader(filePath);
        reader.setAutoTransform(true);

        QImage image = reader.read();
        if (image.isNull()) {
            QMessageBox::warning(this, tr("Error"), tr("Could not load image."));
            return;
        }

        overlayPixmap = QPixmap::fromImage(image);
        overlayLabel->setPixmap(overlayPixmap.scaled(overlayLabel->size(),
        Qt::KeepAspectRatio, Qt::SmoothTransformation));
        overlayLabel->show();
        overlayLabel->raise();
    }
}

void MainWindow::removeBackground() {
    if (overlayPixmap.isNull() || !selectedColor.isValid()) {
        QMessageBox::warning(this, tr("Error"),
        tr("Please load an overlay image and select a color first."));
        return;
    }

    QImage image = overlayPixmap.toImage();
    QImage newImage(image.size(), QImage::Format_ARGB32);

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixelColor = image.pixelColor(x, y);
            if (qAbs(pixelColor.red() - selectedColor.red()) < 30 &&
                qAbs(pixelColor.green() - selectedColor.green()) < 30 &&
                qAbs(pixelColor.blue() - selectedColor.blue()) < 30) {
                newImage.setPixelColor(x, y, QColor(0, 0, 0, 0));  // Set transparent color
            } else {
                newImage.setPixelColor(x, y, pixelColor);
            }
        }
    }

    overlayPixmap = QPixmap::fromImage(newImage);
    overlayLabel->setPixmap(overlayPixmap.scaled(overlayLabel->size(),
        Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::deleteSelected() {
    if (overlayLabel->isVisible()) {
        overlayLabel->clear();
        overlayLabel->hide();
        overlayPixmap = QPixmap();
    }
}

void MainWindow::saveResult() {
    if (backgroundPixmap.isNull()) {
        QMessageBox::warning(this, tr("Error"), tr("No image to save."));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this,
        tr("Save Image"), QString(), tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));

    if (!filePath.isEmpty()) {
        QPixmap result(backgroundLabel->size());
        result.fill(Qt::transparent);

        QPainter painter(&result);
        painter.drawPixmap(0, 0, backgroundLabel->pixmap());

        if (!overlayPixmap.isNull() && overlayLabel->isVisible()) {
            QPoint overlayPos = overlayLabel->pos() - backgroundLabel->pos();
            painter.drawPixmap(overlayPos, overlayLabel->pixmap());
        }

        if (!result.save(filePath)) {
            QMessageBox::warning(this, tr("Error"), tr("Could not save image."));
        }
    }
}

void MainWindow::pickColor() {
    selectedColor = QColorDialog::getColor(Qt::white, this, tr("Select Color to Remove"));
}
