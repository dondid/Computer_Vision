#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <filesystem>
#include <vector>
#include <QString>
#include <QMessageBox>
#include <QIcon>
#include <QDebug>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // NU imi placea cum se deschide pe tot ecranul asa ca am ajustat la o marime adecvata.
    QWidget window;
    window.setWindowTitle("Oglindire Imagine");
    window.resize(400, 400);

    // Aici am vrut sa para putin mai bine aplicatia si am adaugat un logo. dupa ce am terminat cu cerintele...
    QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
    qDebug() << "Setting window icon from:" << iconPath; // am avut mai multe probleme si am introdus acesta linia pentu a vedea ruta la DEBUGER
    window.setWindowIcon(QIcon(iconPath));

    // Dupa cea am terminat am decis sa adaug si o imagine deasupra butoanelor sa arate mai bine
    QLabel *imageLabel = new QLabel();
    QImage image("C:\\openCV\\project\\T01\\foto\\inf.png");
    if (image.isNull()) {
        QMessageBox::warning(&window, "Eroare", "Imaginea nu a putut fi încărcată.");//nu am reusit din prima asa ca am adaugat o atentionare, sa fie mai usor de lucrat
        return -1;
    }

    // aici am modificat dimensiunea imagini si am centrato
    QPixmap pixmap = QPixmap::fromImage(image).scaled(300, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter);

    // Butoane radio
    QRadioButton *horizontalMirror = new QRadioButton("Oglindire orizontală");
    QRadioButton *verticalMirror = new QRadioButton("Oglindire verticală");
    QRadioButton *bothMirror = new QRadioButton("Oglindire orizontală și verticală");
    horizontalMirror->setChecked(true);

    // Butoane pentru selectarea folderelor sursa si destinatie
    QPushButton *selectSourceFolderButton = new QPushButton("Selectează folder sursă");
    selectSourceFolderButton->setFixedSize(200, 40);
    selectSourceFolderButton->setStyleSheet("background-color: blue; color: white;");

    QPushButton *selectDestinationFolderButton = new QPushButton("Selectează folder destinație");
    selectDestinationFolderButton->setFixedSize(200, 40);
    selectDestinationFolderButton->setStyleSheet("background-color: blue; color: white;");

    // Butonul de aplicare
    QPushButton *applyButton = new QPushButton("Aplică oglindirea la toate pozele");
    applyButton->setFixedSize(200, 40);

    // Layout-ul pentru butoane radio
    QVBoxLayout *radioLayout = new QVBoxLayout;
    radioLayout->addWidget(horizontalMirror);
    radioLayout->addWidget(verticalMirror);
    radioLayout->addWidget(bothMirror);

    // Layout principal pentru a gestiona mai bine
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // imaginea si celelalte butoane in layout-ul principal
    mainLayout->addWidget(imageLabel);
    mainLayout->addLayout(radioLayout);
    mainLayout->addWidget(selectSourceFolderButton);
    mainLayout->addWidget(selectDestinationFolderButton);
    mainLayout->addWidget(applyButton);

    // Centram layout-ul principal
    mainLayout->setAlignment(Qt::AlignCenter);

    window.setLayout(mainLayout);

    // Variabile pentru stocarea cailor folderelor
    QString sourceFolderPath;
    QString destinationFolderPath;
    std::vector<QString> imageFiles;

    // Conectam butonul "Selecteaza folder sursa", am adaugat atentionari pentru a si utilizatorului posibila problema
    QObject::connect(selectSourceFolderButton, &QPushButton::clicked, [&]() {
        sourceFolderPath = QFileDialog::getExistingDirectory(&window, "Selectează folderul sursă");
        if (sourceFolderPath.isEmpty()) {
            QMessageBox::warning(&window, "Atenție", "Nu a fost selectat niciun folder sursă.");
        }
    });

    QObject::connect(selectDestinationFolderButton, &QPushButton::clicked, [&]() {
        destinationFolderPath = QFileDialog::getExistingDirectory(&window, "Selectează folderul destinație");
        if (destinationFolderPath.isEmpty()) {
            QMessageBox::warning(&window, "Atenție", "Nu a fost selectat niciun folder destinație.");
        }
    });

    QObject::connect(applyButton, &QPushButton::clicked, [&]() {
        if (sourceFolderPath.isEmpty() || destinationFolderPath.isEmpty()) {
            QMessageBox::warning(&window, "Atenție", "Vă rugăm să selectați atât folderul sursă cât și folderul destinație.");
            return;
        }

        // Parcurgem toate fisierele din folderul sursa, respectiv eroare daca folderul nu are imagini
        for (const auto& entry : fs::directory_iterator(sourceFolderPath.toStdString())) {
            QString filePath = QString::fromStdString(entry.path().string());
            if (filePath.endsWith(".jpg") || filePath.endsWith(".png") || filePath.endsWith(".jpeg")) {
                imageFiles.push_back(filePath);
            }
        }

        if (imageFiles.empty()) {
            QMessageBox::warning(&window, "Atenție", "Nu s-au găsit imagini în folderul sursă.");
            return;
        }

        // Oglindirea tuturor imaginilor
        for (const QString &imageFile : imageFiles) {
            QImage image;
            if (!image.load(imageFile)) {
                QMessageBox::warning(&window, "Eroare", "Nu s-a putut încărca imaginea: " + imageFile);
                continue;
            }

            QImage mirroredImage = image;

            // Oglindirea in functie de selectie
            if (horizontalMirror->isChecked()) {
                mirroredImage = mirroredImage.mirrored(true, false);  // orizontala
            } else if (verticalMirror->isChecked()) {
                mirroredImage = mirroredImage.mirrored(false, true);  // verticala
            } else if (bothMirror->isChecked()) {
                mirroredImage = mirroredImage.mirrored(true, true);   // orizontala si verticala
            }

            // Salvam imaginea in folderul destinație cu acelasi nume
            QString imageName = QFileInfo(imageFile).fileName();
            QString destinationFilePath = destinationFolderPath + "/" + imageName;

            if (!mirroredImage.save(destinationFilePath)) {
                QMessageBox::warning(&window, "Eroare", "Nu s-a putut salva imaginea: " + destinationFilePath);
            }
        }

        QMessageBox::information(&window, "Succes", "Oglindirea imaginilor a fost realizată cu succes.");
    });

    window.show();
    return app.exec();
}

//După ce am realizat cerinețele inițiale, am adăugat atențonarile, respectiv aranjat puțin dizainul pentru o mai bună experientă.
//Am folosit în principal informațile din curs, din videoclipul atașat temei dar și de pe internet unde au fost neclarități.