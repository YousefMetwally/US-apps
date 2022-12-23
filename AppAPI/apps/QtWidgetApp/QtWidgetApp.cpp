#include "QtWidgetApp.hpp"
#ifdef __EMSCRIPTEN__
  #include <HostSupport/HostSupport_i.c> // for PluginHostBasic CLSID
  #include <QFileDialog>
  #include "../shared/LoadFile.hpp"
#endif

QtWidgetApp::QtWidgetApp (QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::QtWidgetApp)
{
    m_ui->setupUi(this);
#ifdef __EMSCRIPTEN__
    m_ui->pushButtonLoad->setVisible(true);
#endif
}

QtWidgetApp::~QtWidgetApp ()
{
}


#ifdef QT_AXSERVER_LIB
QAxAggregated* QtWidgetApp::createAggregate()
#else
PluginImpl* QtWidgetApp::createAggregate()
#endif
{
    if (m_impl)
        return m_impl;

    m_impl = new PluginImpl;
    // deferred processing of OnCurrentFrameChanged & OnImageSourceChanged events (cannot call back to host immediately, since that might cause a deadlock)
    connect(&m_impl->m_signaler, &PluginImplSignaller::OnCurrentFrameChanged, m_ui->imageHandler, &ImageHandler::OnCurrentFrameChanged, Qt::QueuedConnection);
    connect(&m_impl->m_signaler, &PluginImplSignaller::OnImageSourceChanged, m_ui->imageHandler, &ImageHandler::LoadImageSource, Qt::QueuedConnection);
    // immediate processing of other events
    connect(&m_impl->m_signaler, &PluginImplSignaller::OnColorMapChanged, m_ui->imageHandler, &ImageHandler::InvalidateColormaps);
    connect(&m_impl->m_signaler, &PluginImplSignaller::OnSetViewSettingsChanged, m_ui->imageHandler, &ImageHandler::SetViewSettings);

    m_ui->measurementHandler->SetPluginImpl(m_impl);
    m_ui->patientHandler->SetPluginImpl(m_impl);
    m_ui->logHandler->SetPluginImpl(m_impl);
    m_ui->imageHandler->SetPluginImpl(m_impl);
    return m_impl; // transfer ownership to caller
}

void QtWidgetApp::RequestLoad ()
{
#ifdef __EMSCRIPTEN__
    using namespace std::placeholders;
    QFileDialog::getOpenFileContent("Image files (*.*)", std::bind(&QtWidgetApp::OnFileSelected, this, _1, _2)); // async
#endif
}

#ifdef __EMSCRIPTEN__
void QtWidgetApp::OnFileSelected(const QString & /*filename*/, const QByteArray & content)
{
    AppAPI::IPluginHostBasicPtr basic_parent;
    AppAPI::IPluginHostPatientPtr pat_parent;
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
    AppAPI::IImageSourcePtr source = pat_parent->GetImageSource();
    LoadFileIntoSource(L"dataset.dcm", *source);
    
    // initialize app
    createAggregate()->Initialize(AppAPI::STARTUP_NORMAL, {}, nullptr, basic_parent, pat_parent);
}
#endif

void QtWidgetApp::RequestQuit ()
{
    m_impl->PluginHostBasic().RequestDisplayMode(AppAPI::DISPLAY_HIDDEN);
}
