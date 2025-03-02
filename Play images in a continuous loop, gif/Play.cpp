#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QPixmap>
#include <QIcon>
#include <QFileDialog>
#include <QTimer>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>

class SimpleApp : public QWidget {
public:
    SimpleApp(QWidget* parent = nullptr) : QWidget(parent), currentImageIndex(0) {
        setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");
        resize(800, 600);


        QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
        setWindowIcon(QIcon(iconPath));

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QLabel* logoLabel = new QLabel(this);
        QString logoPath = "C:\\openCV\\project\\T01\\foto\\inf.png";
        QPixmap logoPixmap(logoPath);
        logoLabel->setPixmap(logoPixmap.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);

        QLabel* descriptionLabel = new QLabel("Selectați imagini pentru a le reda una după alta.", this);
        descriptionLabel->setAlignment(Qt::AlignCenter);
        descriptionLabel->setWordWrap(true);

        imageLabel = new QLabel(this);
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setMinimumSize(600, 400);
        imageLabel->setScaledContents(true); // Enable scaling
        imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QPushButton* loadButton = new QPushButton("Încarcă imagini", this);
        QPushButton* startButton = new QPushButton("Începe redarea", this);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(loadButton);
        buttonLayout->addWidget(startButton);


        mainLayout->addWidget(logoLabel);
        mainLayout->addWidget(descriptionLabel);
        mainLayout->addWidget(imageLabel);
        mainLayout->addLayout(buttonLayout);

        connect(loadButton, &QPushButton::clicked, this, &SimpleApp::loadImages);
        connect(startButton, &QPushButton::clicked, this, &SimpleApp::startSlideshow);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &SimpleApp::showNextImage);
    }

private:
    QLabel* imageLabel;
    QStringList imagePaths;
    int currentImageIndex;
    QTimer* timer;

    void loadImages() {
        imagePaths = QFileDialog::getOpenFileNames(
            this,
            "Selectați imagini",
            QString(),
            "Imagini (*.png *.jpg *.jpeg *.bmp)");

        if (imagePaths.isEmpty()) {
            QMessageBox::warning(this, "Atenție", "Nu au fost selectate imagini!");
        } else {
            QMessageBox::information(this, "Info", QString("%1 imagini selectate.").arg(imagePaths.size()));
            currentImageIndex = 0;
        }
    }

    void startSlideshow() {
        if (imagePaths.isEmpty()) {
            QMessageBox::warning(this, "Atenție", "Întâi selectați imagini!");
            return;
        }

        timer->start(2000);
        showNextImage();
    }

    void showNextImage() {
        if (imagePaths.isEmpty()) {
            timer->stop();
            return;
        }

        QPixmap pixmap(imagePaths[currentImageIndex]);
        if (pixmap.isNull()) {
            QMessageBox::warning(this, "Atenție", "Imaginea nu a putut fi încărcată: " + imagePaths[currentImageIndex]);
            currentImageIndex = (currentImageIndex + 1) % imagePaths.size();
            return;
        }

        imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        currentImageIndex = (currentImageIndex + 1) % imagePaths.size();
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    SimpleApp viewer;
    viewer.show();
    return app.exec();
}