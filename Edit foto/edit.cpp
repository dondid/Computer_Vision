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
#include <QTransform>
#include <QPainter>
#include <QLineEdit>
#include <QGraphicsTextItem>
#include <QGraphicsOpacityEffect>
#include <QSpinBox>
#include <QImageWriter>

    class MovableTextItem : public QGraphicsTextItem {
public:
    MovableTextItem(const QString& text, QGraphicsItem* parent = nullptr)
        : QGraphicsTextItem(text, parent) {
        setFlag(ItemIsMovable);
        setFlag(ItemIsSelectable);
        setFlag(ItemSendsScenePositionChanges);

        opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(opacityEffect);
    }

    void mirror(bool horizontal) {
        QTransform transform;
        if (horizontal) {
            transform.scale(-1, 1);
        } else {
            transform.scale(1, -1);
        }
        setTransform(transform * this->transform());
    }

    void setScale(qreal scale) {
        QTransform transform = this->transform();
        transform.scale(scale, scale);
        setTransform(transform);
    }

    void setOpacity(qreal value) {
        if (opacityEffect) {
            opacityEffect->setOpacity(value);
        }
    }

    qreal opacity() const {
        return opacityEffect ? opacityEffect->opacity() : 1.0;
    }

private:
    QGraphicsOpacityEffect* opacityEffect;
};

class ImageItem : public QGraphicsPixmapItem {
public:
    ImageItem(const QPixmap& pixmap) : QGraphicsPixmapItem(pixmap) {
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemIsSelectable);
        setFlag(QGraphicsItem::ItemSendsScenePositionChanges);

        opacityEffect = new QGraphicsOpacityEffect();
        setGraphicsEffect(opacityEffect);
    }

    void mirror(bool horizontal) {
        QTransform transform;
        if (horizontal) {
            transform.scale(-1, 1);
        } else {
            transform.scale(1, -1);
        }
        setPixmap(pixmap().transformed(transform, Qt::SmoothTransformation));
    }

    void setOpacity(qreal value) {
        if (opacityEffect) {
            opacityEffect->setOpacity(value);
        }
    }

    qreal opacity() const {
        return opacityEffect ? opacityEffect->opacity() : 1.0;
    }

    void addText(const QString& text, const QPointF& position) {
        MovableTextItem* textItem = new MovableTextItem(text, this);
        textItem->setDefaultTextColor(Qt::black);
        textItem->setFont(QFont("Arial", 20));
        textItem->setPos(position);
        scene()->addItem(textItem);
    }

private:
    QGraphicsOpacityEffect* opacityEffect;
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

        QPushButton* importButton = new QPushButton("Import Image", widget);
        connect(importButton, &QPushButton::clicked, this, &SimpleImageViewer::importImage);

        QGraphicsView* graphicsView = new QGraphicsView(widget);
        QGraphicsScene* scene = new QGraphicsScene(this);
        graphicsView->setScene(scene);
        layout->addWidget(importButton);
        layout->addWidget(graphicsView);

        QLineEdit* textInput = new QLineEdit(widget);
        textInput->setPlaceholderText("Enter text to add to the image");
        layout->addWidget(textInput);

        QPushButton* addTextButton = new QPushButton("Add Text", widget);
        connect(addTextButton, &QPushButton::clicked, [this, textInput, scene]() {
            QString text = textInput->text();
            if (text.isEmpty()) {
                QMessageBox::warning(this, "Input Error", "Please enter text to add.");
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

        QPushButton* deleteButton = new QPushButton("Delete Selected Items", widget);
        deleteButton->setIcon(QIcon::fromTheme("edit-delete"));
        connect(deleteButton, &QPushButton::clicked, this, &SimpleImageViewer::deleteSelectedItems);
        layout->addWidget(deleteButton);

        QHBoxLayout* scaleLayout = new QHBoxLayout();
        QLabel* scaleLabel = new QLabel("Scale:", widget);
        QSlider* scaleSlider = new QSlider(Qt::Horizontal, widget);
        scaleSlider->setRange(1, 300);
        scaleSlider->setValue(100);
        connect(scaleSlider, &QSlider::valueChanged, this, &SimpleImageViewer::scaleItem);
        scaleLayout->addWidget(scaleLabel);
        scaleLayout->addWidget(scaleSlider);
        layout->addLayout(scaleLayout);

        QHBoxLayout* opacityLayout = new QHBoxLayout();
        QLabel* opacityLabel = new QLabel("Transparency:", widget);
        QSlider* opacitySlider = new QSlider(Qt::Horizontal, widget);
        opacitySlider->setRange(0, 100);
        opacitySlider->setValue(100);
        connect(opacitySlider, &QSlider::valueChanged, this, &SimpleImageViewer::setOpacity);
        opacityLayout->addWidget(opacityLabel);
        opacityLayout->addWidget(opacitySlider);
        layout->addLayout(opacityLayout);

        QHBoxLayout* mirrorLayout = new QHBoxLayout();
        QPushButton* mirrorHButton = new QPushButton("Mirror Horizontal", widget);
        QPushButton* mirrorVButton = new QPushButton("Mirror Vertical", widget);
        connect(mirrorHButton, &QPushButton::clicked, [this]() {
            mirrorSelectedItem(true);
        });
        connect(mirrorVButton, &QPushButton::clicked, [this]() {
            mirrorSelectedItem(false);
        });
        mirrorLayout->addWidget(mirrorHButton);
        mirrorLayout->addWidget(mirrorVButton);
        layout->addLayout(mirrorLayout);

        QHBoxLayout* qualityLayout = new QHBoxLayout();
        QLabel* qualityLabel = new QLabel("Quality (1-100):", widget);
        QSpinBox* qualitySpinBox = new QSpinBox(widget);
        qualitySpinBox->setRange(1, 100);
        qualitySpinBox->setValue(100);
        qualityLayout->addWidget(qualityLabel);
        qualityLayout->addWidget(qualitySpinBox);
        layout->addLayout(qualityLayout);

        QHBoxLayout* dimensionsLayout = new QHBoxLayout();
        QLabel* widthLabel = new QLabel("Width:", widget);
        QSpinBox* widthSpinBox = new QSpinBox(widget);
        widthSpinBox->setRange(1, 8000);
        widthSpinBox->setValue(800);

        QLabel* heightLabel = new QLabel("Height:", widget);
        QSpinBox* heightSpinBox = new QSpinBox(widget);
        heightSpinBox->setRange(1, 8000);
        heightSpinBox->setValue(600);

        dimensionsLayout->addWidget(widthLabel);
        dimensionsLayout->addWidget(widthSpinBox);
        dimensionsLayout->addWidget(heightLabel);
        dimensionsLayout->addWidget(heightSpinBox);
        layout->addLayout(dimensionsLayout);

        QPushButton* saveButton = new QPushButton("Save Image", widget);
        connect(saveButton, &QPushButton::clicked, [this, graphicsView, qualitySpinBox, widthSpinBox, heightSpinBox]() {
            QImage image(widthSpinBox->value(), heightSpinBox->value(), QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            QRect targetRect(0, 0, widthSpinBox->value(), heightSpinBox->value());

            QGraphicsScene* scene = graphicsView->scene();
            if (scene) {
                scene->render(&painter, targetRect, scene->itemsBoundingRect());
            }

            QString filePath = QFileDialog::getSaveFileName(this, "Save Image", "", "Images (*.png *.xpm *.jpg)");
            if (!filePath.isEmpty()) {
                // Salvează imaginea cu calitatea specificată
                QImageWriter writer(filePath);
                writer.setQuality(qualitySpinBox->value());
                writer.write(image);
            }
        });

        layout->addWidget(saveButton);

        return widget;
    }

    void deleteSelectedItems() {
        QGraphicsView* view = findChild<QGraphicsView*>();
        if (view) {
            QGraphicsScene* scene = view->scene();
            QList<QGraphicsItem*> selectedItems = scene->selectedItems();

            if (selectedItems.isEmpty()) {
                QMessageBox::information(this, "Delete", "No items selected");
                return;
            }

            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Confirm Delete",
                QString("Are you sure you want to delete %1 selected item(s)?").arg(selectedItems.size()),
                QMessageBox::Yes | QMessageBox::No
                );

            if (reply == QMessageBox::Yes) {
                for (QGraphicsItem* item : selectedItems) {
                    scene->removeItem(item);
                    delete item;
                }
            }
        }
    }

    void importImage() {
        QString filePath = QFileDialog::getOpenFileName(this, "Import Image", "", "Images (*.png *.jpg *.jpeg *.bmp)");
        if (!filePath.isEmpty()) {
            QGraphicsView* view = findChild<QGraphicsView*>();
            if (view) {
                QGraphicsScene* scene = view->scene();
                QPixmap pixmap(filePath);
                if (!pixmap.isNull()) {
                    ImageItem* imageItem = new ImageItem(pixmap);
                    scene->addItem(imageItem);
                    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
                } else {
                    QMessageBox::warning(this, "Import Error", "Failed to load image.");
                }
            }
        }
    }

    void scaleItem(int scaleValue) {
        QGraphicsView* view = findChild<QGraphicsView*>();
        if (view) {
            QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
            qreal scale = scaleValue / 100.0;
            for (QGraphicsItem* item : selectedItems) {
                if (ImageItem* imageItem = dynamic_cast<ImageItem*>(item)) {
                    imageItem->setScale(scale);
                } else if (MovableTextItem* textItem = dynamic_cast<MovableTextItem*>(item)) {
                    textItem->setScale(scale);
                }
            }
        }
    }

    void setOpacity(int opacityValue) {
        qreal opacity = opacityValue / 100.0;
        QGraphicsView* view = findChild<QGraphicsView*>();
        if (view) {
            QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
            for (QGraphicsItem* item : selectedItems) {
                if (ImageItem* imageItem = dynamic_cast<ImageItem*>(item)) {
                    imageItem->setOpacity(opacity);
                } else if (MovableTextItem* textItem = dynamic_cast<MovableTextItem*>(item)) {
                    textItem->setOpacity(opacity);
                }
            }
        }
    }

    void mirrorSelectedItem(bool horizontal) {
        QGraphicsView* view = findChild<QGraphicsView*>();
        if (view) {
            QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
            for (QGraphicsItem* item : selectedItems) {
                if (ImageItem* imageItem = dynamic_cast<ImageItem*>(item)) {
                    imageItem->mirror(horizontal);
                } else if (MovableTextItem* textItem = dynamic_cast<MovableTextItem*>(item)) {
                    textItem->mirror(horizontal);
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    SimpleImageViewer viewer;
    viewer.show();
    return app.exec();
}