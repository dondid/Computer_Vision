#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QPoint>
#include <QColor>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CustomLabel : public QLabel {
    Q_OBJECT
public:
    explicit CustomLabel(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    bool isDragging;
    QPoint dragStartPosition;
    QPoint labelStartPosition;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadBackground();
    void loadOverlay();
    void removeBackground();
    void deleteSelected();
    void saveResult();
    void pickColor();

private:
    Ui::MainWindow *ui;
    CustomLabel* backgroundLabel;
    CustomLabel* overlayLabel;
    QPixmap backgroundPixmap;
    QPixmap overlayPixmap;
    QColor selectedColor;
    void initializeUI();
};

#endif
