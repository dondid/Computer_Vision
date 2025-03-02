#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#include <QIcon>
#include <QDebug>
#include <QPushButton>
#include <QStackedWidget>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QSlider>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QGraphicsTextItem>
#include <QColor>
#include <QColorDialog>
#include <QFont>
#include <QPainter>

class MovableTextItem : public QGraphicsTextItem {
public:
    MovableTextItem(const QString& text, QGraphicsItem* parent = nullptr)
        : QGraphicsTextItem(text, parent) {
        setFlag(ItemIsMovable);
        setFlag(ItemIsSelectable);
        setFlag(ItemSendsScenePositionChanges);
    }

    void setFontWeight(int weight) {
        QFont currentFont = font();
        currentFont.setWeight(static_cast<QFont::Weight>(weight));
        setFont(currentFont);
    }

    void setTextColor(const QColor& color) {
        setDefaultTextColor(color);
    }

    void setFontSize(int size) {
        QFont currentFont = font();
        currentFont.setPointSize(size);
        setFont(currentFont);
    }
};

class ImageItem : public QGraphicsPixmapItem {
public:
    ImageItem(const QPixmap& pixmap) : QGraphicsPixmapItem(pixmap) {
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemIsSelectable);
        setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    }

    void addText(const QString& text, const QPointF& position) {
        MovableTextItem* textItem = new MovableTextItem(text, this);
        textItem->setDefaultTextColor(Qt::black);
        textItem->setFont(QFont("Arial", 20));
        textItem->setPos(position);
        scene()->addItem(textItem);
    }
};

class SimpleImageViewer : public QWidget {
public:
    SimpleImageViewer(QWidget* parent = nullptr) : QWidget(parent) {
        setWindowTitle("Computer Vision 2024-2025 © Dodoc Ionuț-Daniel");
        resize(800, 600);
        QString iconPath = "C:\\openCV\\project\\T01\\foto\\ucv-logo.png";
        qDebug() << "Setting window icon from:" << iconPath;
        setWindowIcon(QIcon(iconPath));

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        QStackedWidget* stackedWidget = new QStackedWidget(this);

        QWidget* photoEditWidget = createPhotoEditWidget();
        stackedWidget->addWidget(photoEditWidget);

        mainLayout->addWidget(stackedWidget);
        setLayout(mainLayout);
    }

private:
    QWidget* createPhotoEditWidget() {
        QWidget* widget = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(widget);

        QPushButton* importButton = new QPushButton("Încarcă imaginea", widget);
        connect(importButton, &QPushButton::clicked, this, &SimpleImageViewer::importImage);

        QGraphicsView* graphicsView = new QGraphicsView(widget);
        QGraphicsScene* scene = new QGraphicsScene(this);
        graphicsView->setScene(scene);
        layout->addWidget(importButton);
        layout->addWidget(graphicsView);

        QLineEdit* textInput = new QLineEdit(widget);
        textInput->setPlaceholderText("Introdu text");
        layout->addWidget(textInput);

        QPushButton* addTextButton = new QPushButton("Adaugă textul", widget);
        connect(addTextButton, &QPushButton::clicked, [this, textInput, scene]() {
            QString text = textInput->text();
            if (text.isEmpty()) {
                QMessageBox::warning(this, "Eroare de introducere", "Vă rugăm să introduceți text pentru a fi adăugat.");
                return;
            }

            QPointF position(10, 30);
            QList<QGraphicsItem*> selectedItems = scene->selectedItems();
            if (!selectedItems.isEmpty()) {
                if (ImageItem* imageItem = dynamic_cast<ImageItem*>(selectedItems.first())) {
                    imageItem->addText(text, position);
                }
            }
        });
        layout->addWidget(addTextButton);

        QPushButton* deleteButton = new QPushButton("Șterge", widget);
        deleteButton->setIcon(QIcon::fromTheme("edit-delete"));
        connect(deleteButton, &QPushButton::clicked, this, &SimpleImageViewer::deleteSelectedItems);
        layout->addWidget(deleteButton);

        QHBoxLayout* textWeightLayout = new QHBoxLayout();
        QLabel* textWeightLabel = new QLabel("Grosimea textului:", widget);
        QSlider* textWeightSlider = new QSlider(Qt::Horizontal, widget);
        textWeightSlider->setRange(QFont::Thin, QFont::Black);
        textWeightSlider->setValue(QFont::Normal);

        connect(textWeightSlider, &QSlider::valueChanged, [this](int value) {
            QGraphicsView* view = findChild<QGraphicsView*>();
            if (view) {
                QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
                for (QGraphicsItem* item : selectedItems) {
                    if (MovableTextItem* textItem = dynamic_cast<MovableTextItem*>(item)) {
                        textItem->setFontWeight(value);
                    }
                }
            }
        });

        textWeightLayout->addWidget(textWeightLabel);
        textWeightLayout->addWidget(textWeightSlider);
        layout->addLayout(textWeightLayout);

        QHBoxLayout* textSizeLayout = new QHBoxLayout();
        QLabel* textSizeLabel = new QLabel("Dimensiunea textului:", widget);
        QSlider* textSizeSlider = new QSlider(Qt::Horizontal, widget);
        textSizeSlider->setRange(5, 100);
        textSizeSlider->setValue(20);
        connect(textSizeSlider, &QSlider::valueChanged, [this](int value) {
            QGraphicsView* view = findChild<QGraphicsView*>();
            if (view) {
                QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
                for (QGraphicsItem* item : selectedItems) {
                    if (MovableTextItem* textItem = dynamic_cast<MovableTextItem*>(item)) {
                        textItem->setFontSize(value);
                    }
                }
            }
        });
        textSizeLayout->addWidget(textSizeLabel);
        textSizeLayout->addWidget(textSizeSlider);
        layout->addLayout(textSizeLayout);

        // Text Color Control
        QHBoxLayout* textColorLayout = new QHBoxLayout();
        QLabel* textColorLabel = new QLabel("Culoare:", widget);
        QPushButton* colorButton = new QPushButton("Selectează culoarea", widget);
        colorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QLabel* colorDisplayLabel = new QLabel(widget);
        colorDisplayLabel->setFixedSize(50, 20);
        colorDisplayLabel->setStyleSheet("background-color: black;");

        connect(colorButton, &QPushButton::clicked, [this, colorDisplayLabel]() {
            QColor color = QColorDialog::getColor(Qt::black, this, "Selectează culoarea textului");
            if (color.isValid()) {
                QGraphicsView* view = findChild<QGraphicsView*>();
                if (view) {
                    QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
                    for (QGraphicsItem* item : selectedItems) {
                        if (MovableTextItem* textItem = dynamic_cast<MovableTextItem*>(item)) {
                            textItem->setTextColor(color);
                        }
                    }
                }
                QString style = QString("background-color: %1;").arg(color.name());
                colorDisplayLabel->setStyleSheet(style);
            }
        });

        QHBoxLayout* centerLayout = new QHBoxLayout();
        centerLayout->addWidget(colorButton);
        centerLayout->addWidget(colorDisplayLabel);
        textColorLayout->addWidget(textColorLabel);
        textColorLayout->addLayout(centerLayout);
        layout->addLayout(textColorLayout);

        QPushButton* saveButton = new QPushButton("Salvează", widget);
        connect(saveButton, &QPushButton::clicked, [this, scene]() {
            QString filePath = QFileDialog::getSaveFileName(this, "Salvează imaginea ca", "", "PNG Files (*.png);;JPEG Files (*.jpg *.jpeg);;All Files (*)");
            if (!filePath.isEmpty()) {
                QRectF boundingRect = scene->itemsBoundingRect();
                QSize size = boundingRect.size().toSize();
                QImage image(size, QImage::Format_ARGB32);
                image.fill(Qt::white);

                QPainter painter(&image);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                scene->render(&painter, QRectF(boundingRect), boundingRect);

                image.save(filePath);
            }
        });
        layout->addWidget(saveButton);

        widget->setLayout(layout);
        return widget;
    }

    void importImage() {
        QString filePath = QFileDialog::getOpenFileName(this, "Încarcă imaginea", "", "Imagine cu extensia (*.png *.jpg *.jpeg *.bmp)");
        if (filePath.isEmpty()) return;

        QGraphicsView* view = findChild<QGraphicsView*>();
        if (!view) return;

        QGraphicsScene* scene = view->scene();
        scene->clear();

        QPixmap pixmap(filePath);
        ImageItem* imageItem = new ImageItem(pixmap);
        scene->addItem(imageItem);
        view->fitInView(imageItem, Qt::KeepAspectRatio);
    }

    void deleteSelectedItems() {
        QGraphicsView* view = findChild<QGraphicsView*>();
        if (!view) return;

        QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
        for (QGraphicsItem* item : selectedItems) {
            view->scene()->removeItem(item);
            delete item;
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    SimpleImageViewer viewer;
    viewer.show();
    return app.exec();
}