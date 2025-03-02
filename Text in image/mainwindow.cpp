#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QColorDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene(new QGraphicsScene(this)) {
    ui->setupUi(this);
    ui->graphicsView->setScene(scene);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_pushButton_importImage_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Import Image", "", "Images (*.png *.xpm *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        QPixmap pixmap(fileName);
        scene->clear();
        scene->addPixmap(pixmap);
    }
}

void MainWindow::on_pushButton_addText_clicked() {
    bool ok;
    QString text = QInputDialog::getText(this, "Add Text", "Text:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        QGraphicsTextItem *textItem = scene->addText(text);
        textItem->setDefaultTextColor(Qt::black);
    }
}

void MainWindow::on_pushButton_deleteSelected_clicked() {
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    foreach (QGraphicsItem *item, selectedItems) {
        scene->removeItem(item);
        delete item;
    }
}

void MainWindow::on_pushButton_saveImage_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "", "Images (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        scene->render(&painter);
        image.save(fileName);
    }
}

void MainWindow::on_pushButton_selectColor_clicked() {
    QColor color = QColorDialog::getColor(Qt::white, this, "Select Color");
    if (color.isValid()) {
        QGraphicsTextItem *textItem = scene->addText("Example Text");
        textItem->setDefaultTextColor(color);
    }
}