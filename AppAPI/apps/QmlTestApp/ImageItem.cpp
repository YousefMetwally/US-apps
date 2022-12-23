#include "ImageItem.hpp"

using namespace AppAPI;

ImageItem::ImageItem(QQuickItem *parent) : QQuickPaintedItem(parent) {
    QImage default_img(640, 480, QImage::Format_RGBA8888);
    default_img.fill(Qt::lightGray);

    m_image = default_img;
}

ImageItem::~ImageItem() {
}

void ImageItem::paint(QPainter *painter) {
    QImage image = m_image.mirrored(m_view_cfg.flip_left_right, m_view_cfg.flip_up_down);

    QRectF bounding_rect = boundingRect();
    image = image.scaledToHeight(bounding_rect.height());
    QPointF center = bounding_rect.center() - image.rect().center();

    painter->drawImage(center, image);
}

void ImageItem::onImageAvailable(QImage image) {
    m_image = image;
    update();
}

void ImageItem::onViewSettingsChanged(ViewSettings view_cfg) {
    m_view_cfg = view_cfg;
    update();
}
