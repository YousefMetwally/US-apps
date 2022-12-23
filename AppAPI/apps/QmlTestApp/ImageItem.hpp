#pragma once
#include <QQuickPaintedItem>
#include <QQuickItem>
#include <QPainter>
#include <QImage>
#include <AppAPI/AppAPI.tlh>


class ImageItem : public QQuickPaintedItem {
    Q_OBJECT
public:
    ImageItem(QQuickItem *parent = nullptr);
    ~ImageItem() override;

    void paint(QPainter *painter) override;

public slots:
    void onImageAvailable(QImage image);
    void onViewSettingsChanged(AppAPI::ViewSettings view_cfg);

private:
    QImage       m_image;
    AppAPI::ViewSettings m_view_cfg{};
};
