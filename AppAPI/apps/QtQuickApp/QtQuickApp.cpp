#include "QtQuickApp.hpp"

#include <QColor>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QQmlError>
#include <QQuickItem>
#include <QQmlContext>
#include "ImageProvider2d.hpp"
#ifdef __EMSCRIPTEN__
  #include <HostSupport/HostSupport_i.c> // for PluginHostBasic CLSID
  #include "../shared/LoadFile.hpp"
#endif

using namespace AppAPI;


QtQuickApp::QtQuickApp(QWidget *parent) :
    QMainWindow(parent)
{
    // Ensure that the QML objects are registered
    qmlRegisterType<QtQuickApp>("test2d", 1, 0, "QtQuickApp");
    qmlRegisterType<ImageProvider2d>("test2d", 1, 0, "ImageProvider2d");

    m_ui.reset(new QuickUI);
}


QtQuickApp::~QtQuickApp()
{
}


#ifdef QT_AXSERVER_LIB
QAxAggregated* QtQuickApp::createAggregate()
#else
PluginImpl* QtQuickApp::createAggregate()
#endif
{
    if (m_impl)
        return m_impl;

    m_impl = new PluginImpl(this);

    CreateUI();

    // Connect signaller
    connect(m_impl->GetSignaller(), &PluginImplSignaller::ParentSet, this, &QtQuickApp::OnParentSet);
    return m_impl; // transfer ownership to caller
}


void QtQuickApp::CreateUI() {
    // Create a separate window to display the QtQuick UI
    // This is necessary for Qt3D to work, as it requires certain window
    // settings that are not set by ActiveQt.
    //
    // The window is not displayed directly, but instead wrapped in a widget
    // and added directly to our layout. The effect is that the QtQuick window is
    // displayed as if it was a widget.
#if defined __EMSCRIPTEN__ || defined _WIN32
    auto container = QWidget::createWindowContainer(m_ui.get(), this);
#endif
    m_ui->setResizeMode(m_ui->SizeRootObjectToView);
    QQmlEngine *engine = m_ui->engine();

    m_imageProvider = new ImageProvider2d(m_impl);
    connect(&m_timer, &QTimer::timeout, m_imageProvider, &ImageProvider2d::OnUpdateFrame);
    connect(m_impl->GetSignaller(), &PluginImplSignaller::ColorMapChanged, m_imageProvider, &ImageProvider2d::InvalidateColormap);
    connect(m_impl->GetSignaller(), &PluginImplSignaller::ImageSourceChanged, m_imageProvider, &ImageProvider2d::Invalidate);

    engine->addImageProvider(QLatin1String("image2dprovider"), m_imageProvider);//engine takes ownership: http://doc.qt.io/qt-5/qqmlengine.html#addImageProvider

    {
        // set context properties before QML is loaded
        QQmlContext * root = m_ui->rootContext();
        assert(root);
        root->setContextProperty("application", QVariant::fromValue(this));
    }

    m_ui->setSource(QUrl("qrc:/test2d/main.qml"));
    if (m_ui->status() == QuickUI::Error) {
        QList<QQmlError> errors = m_ui->errors(); // nice for debugging
        std::string message = "QtQuickApp QML load error: ";
        for (QQmlError error : errors)
            message += error.toString().toStdString() + "\n";

        throw std::logic_error(message.c_str());
    }

#ifdef __EMSCRIPTEN__
    // show Load button
    QObject *loadBtn = m_ui->rootObject()->findChild<QObject*>("loadBtn");
    loadBtn->setProperty("visible", true);

    connect(m_ui->rootObject(), SIGNAL(loadClicked()), this, SLOT(OnFileLoad()));
#endif

#if defined __EMSCRIPTEN__ || defined _WIN32
    setCentralWidget(container);
#else
    setCentralWidget(m_ui.get());
#endif
    connect(m_ui->rootObject(), SIGNAL(exitClicked()), this, SLOT(OnExit()));
    connect(m_ui->rootObject(), SIGNAL(playStopClicked()), this, SLOT(OnStopPlay()));
}


void QtQuickApp::OnParentSet()
{
    if (m_impl->GetMovie2d())
        OnStopPlay();
}

void QtQuickApp::OnExit()
{
    close();

    m_impl->PluginHostBasic().RequestDisplayMode(AppAPI::DISPLAY_HIDDEN);
}

void QtQuickApp::OnStopPlay()
{
    if (m_timer.isActive())
        m_timer.stop();
    else
        m_timer.start(100);
}

#ifdef __EMSCRIPTEN__
void QtQuickApp::OnFileLoad()
{
    using namespace std::placeholders;
    QFileDialog::getOpenFileContent("Image files (*.*)", std::bind(&QtQuickApp::OnFileSelected, this, _1, _2)); // async
}

void QtQuickApp::OnFileSelected(const QString & /*filename*/, const QByteArray & content)
{
    IPluginHostBasicPtr basic_parent;
    IPluginHostPatientPtr pat_parent;
    CHECK(basic_parent.CreateInstance(CLSID_PluginHostBasic, NULL, CLSCTX_ALL));
    CHECK(pat_parent.CreateInstance(CLSID_PluginHostPatient, NULL, CLSCTX_ALL));
    
    // write DICOM to temporary file
    FILE* file = fopen("dataset.dcm", "wb");
    assert(file);
    size_t bytes_written = fwrite(content.data(), 1, content.size(), file);
    if (bytes_written != content.size())
        throw std::runtime_error("fwrite failed");
    fclose(file);
    
    // load temporary file back & initialize app
    IImageSourcePtr source = pat_parent->GetImageSource();
    LoadFileIntoSource(L"dataset.dcm", *source);
    
    // initialize app
    createAggregate()->Initialize(AppAPI::STARTUP_NORMAL, {}, nullptr, basic_parent, pat_parent);
}
#endif
