#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");

    QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
    qDebug() << "Setting window icon from:" << iconPath;
    setWindowIcon(QIcon(iconPath));

    connect(ui->loadImageButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(ui->predictButton, &QPushButton::clicked, this, &MainWindow::classifyImage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadImage()
{
    imagePath = QFileDialog::getOpenFileName(this, tr("Încărcați imagine"), "", tr("Imagini (*.png *.jpg *.bmp)"));
    if (!imagePath.isEmpty()) {
        ui->imageLabel->setPixmap(QPixmap(imagePath).scaled(200, 200, Qt::KeepAspectRatio));
    } else {
        QMessageBox::warning(this, tr("Eroare"), tr("Nu ați selectat nicio imagine."));
    }
}

void MainWindow::classifyImage()
{
    if (imagePath.isEmpty()) {
        QMessageBox::warning(this, tr("Eroare"), tr("Încărcați mai întâi o imagine."));
        return;
    }

    QProcess process;
    QString pythonScript = "C:/openCV/project/T01/predict.py";

    if (!QFile::exists(pythonScript)) {
        QMessageBox::critical(this, tr("Eroare"),
                              tr("Scriptul Python nu a fost găsit la calea: ") + pythonScript);
        return;
    }

    QStringList arguments;
    arguments << pythonScript << imagePath;

    qDebug() << "Executing command: python" << arguments;

    process.start("python", arguments);

    if (!process.waitForFinished(30000)) {
        QMessageBox::critical(this, tr("Eroare"),
                              tr("Timeout la execuția scriptului Python."));
        return;
    }

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    qDebug() << "Process exit code:" << process.exitCode();
    qDebug() << "Process exit status:" << process.exitStatus();
    qDebug() << "Standard output:" << output;
    qDebug() << "Standard error:" << error;

    if (process.exitCode() != 0 || !error.isEmpty()) {
        QString errorMessage = tr("Eroare la execuția scriptului Python:\n");
        if (!error.isEmpty()) {
            errorMessage += error;
        } else {
            errorMessage += output;
        }
        QMessageBox::critical(this, tr("Eroare"), errorMessage);
        return;
    }

    if (output.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Eroare"),
                             tr("Nu s-a primit niciun rezultat de la scriptul Python."));
        return;
    }

    ui->resultLabel->setText(output.trimmed());
}
