#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QColorDialog>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <QGraphicsSceneMouseEvent>
#include <QRubberBand>
#include <QGraphicsRectItem>

class CustomGraphicsScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit CustomGraphicsScene(QObject *parent = nullptr) : QGraphicsScene(parent) {}

signals:
    void mousePressed(QPointF point);
    void mouseMoved(QPointF point);
    void mouseReleased(QPointF point);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        emit mousePressed(event->scenePos());
        QGraphicsScene::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        emit mouseMoved(event->scenePos());
        QGraphicsScene::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        emit mouseReleased(event->scenePos());
        QGraphicsScene::mouseReleaseEvent(event);
    }
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_loadImage_clicked();
    void on_pushButton_backgroundImage_clicked();
    void on_pushButton_backgroundColor_clicked();
    void on_horizontalSlider_threshold_valueChanged(int value);
    void on_pushButton_saveImage_clicked();
    void on_pushButton_selectROI_clicked();
    void handleMousePress(QPointF point);
    void handleMouseMove(QPointF point);
    void handleMouseRelease(QPointF point);

private:
    void updateImage();
    QImage cvMatToQImage(const cv::Mat& mat);
    void updateROISelection();

    Ui::MainWindow *ui;
    CustomGraphicsScene *scene;
    cv::Mat originalImage;
    cv::Mat backgroundImage;
    cv::Mat result;
    cv::Mat roiMask;
    cv::Scalar backgroundColor;
    bool useBackgroundImage;
    int thresholdValue;

    bool isSelectingROI;
    QPointF roiStartPoint;
    QPointF roiEndPoint;
    QGraphicsRectItem* roiRect;
    bool hasROI;
};

#endif
