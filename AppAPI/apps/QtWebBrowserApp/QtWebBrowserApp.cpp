#include "QtWebBrowserApp.hpp"

#include <QColor>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QQmlError>
#include <QQuickItem>


QtWebBrowserApp::QtWebBrowserApp(QWidget *parent) :
    QMainWindow(parent)
{
    // Ensure that the QML objects are registered
    qmlRegisterType<QtWebBrowserApp>("qtwebbrowser", 1, 0, "QtWebBrowserApp");

    m_ui.reset(new QQuickView);
}


QtWebBrowserApp::~QtWebBrowserApp()
{
}


#ifdef QT_AXSERVER_LIB
QAxAggregated* QtWebBrowserApp::createAggregate()
#else
PluginImpl* QtQuickApp::createAggregate()
#endif
{
    if (!m_impl)
        m_impl = new PluginImpl(this);

    // Connect signaller
    connect(m_impl->GetSignaller(), &PluginImplSignaller::ParentSet, this, &QtWebBrowserApp::OnParentSet);
    return m_impl; // transfer ownership to caller
}


void QtWebBrowserApp::CreateUI() {
    if (m_ui->rootObject())
        return;

    m_ui->setResizeMode(QQuickView::SizeRootObjectToView);

    m_ui->setSource(QUrl("qrc:/qtwebbrowser/main.qml"));
    if (m_ui->status() == QQuickView::Error) {
        QList<QQmlError> errors = m_ui->errors(); // nice for debugging
        std::string message = "QtQuickApp QML load error: ";
        for (QQmlError error : errors)
            message += error.toString().toStdString() + "\n";

        throw std::logic_error(message.c_str());
    }

    // Create a separate window to display the QtQuick UI
    // The window is not displayed directly, but instead wrapped in a widget
    // and added directly to our layout. The effect is that the QtQuick window is
    // displayed as if it was a widget.
    auto container = QWidget::createWindowContainer(m_ui.get(), this);
    setCentralWidget(container);
}


void QtWebBrowserApp::OnParentSet()
{
    CreateUI();

    QObject *object = m_ui->rootObject();
    QQuickItem *web = object->findChild<QQuickItem*>("webView");

    QMetaObject::invokeMethod(web, "openURL", Q_ARG(QVariant, "https://www.google.com"));
}
