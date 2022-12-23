#include <AppAPI/AppAPI.tlh> // for IPluginApp
#include "QmlTestApp.hpp"
#include "ImageItem.hpp"
#include "../shared/QImageSupport.hpp"

using namespace AppAPI;


QmlTestApp::QmlTestApp(QQmlEngine *engine, QObject *parent, QQmlContext *context)
    : m_movie_loader() {

    qRegisterMetaType<ViewSettings>("ViewSettings");

    m_component = new QQmlComponent(engine, parent);
    m_component->loadUrl(QUrl("qrc:/app.qml"));
    if (m_component->isError()) {
        qWarning() << m_component->errors();
        abort();
    }

    m_qml_item.reset(qobject_cast<QQuickItem*>(m_component->create(context)));

    QQuickItem *change_displaymode_quickitem = m_qml_item->findChild<QQuickItem*>("changedisplaymodeobject");
    QObject::connect(change_displaymode_quickitem, SIGNAL(changeDisplayModeSignal()), this, SLOT(ChangeDisplayMode()));

    QQuickItem *quit_plugin_quickitem = m_qml_item->findChild<QQuickItem*>("quitpluginbuttonobject");
    QObject::connect(quit_plugin_quickitem, SIGNAL(quitPluginSignal()), this, SLOT(QuitPlugin()));

    QQuickItem* image_item = m_qml_item->findChild<ImageItem *>("imageitemobject");
    assert(image_item);

    connect(this, SIGNAL(imageAvailable(QImage)), image_item, SLOT(onImageAvailable(QImage)), Qt::QueuedConnection);
    connect(this, SIGNAL(viewSettingsChanged(ViewSettings)), image_item, SLOT(onViewSettingsChanged(ViewSettings)), Qt::QueuedConnection);
}

QmlTestApp::~QmlTestApp() {
}

// QmlPluginApp interface
QQuickItem* QmlTestApp::GetQuickItem () {
    return m_qml_item.get();
}

HRESULT QmlTestApp::raw_Initialize(StartupMode /*mode*/, InputMode /*input_mode*/, /*in*/BSTR /*locale*/, IPluginHostBasic * plugin_host_basic, IPluginHostPatient * pat) {
    if (!pat)
        return E_INVALIDARG;

    m_host_patient = pat;
    m_plugin_host_basic = plugin_host_basic;

    // retrieve flip settings
    OnCustomViewSettingsChanged();

    return S_OK;
}

HRESULT QmlTestApp::raw_OnImageSourceChanged() {
    m_movie_loader.InvalidateStream();
    return S_OK;
}

HRESULT QmlTestApp::raw_OnCustomViewSettingsChanged() {
    SAFEARRAY* sa_tmp = nullptr;
    HRESULT hr = m_host_patient->raw_GetCustomViewSettings(&m_view_cfg, &sa_tmp);
    if (FAILED(hr))
        return hr;

    CComSafeArray<BYTE> data;
    data.Attach(sa_tmp); // take ownership of buffer (for automatic release)

    emit viewSettingsChanged(m_view_cfg);

    return hr;
}

HRESULT QmlTestApp::raw_OnCurrentFrameChanged(int moviegroup, int movie, int stream) {
    // NOTE: Image loading currently requires the OpenGL context that is available
    // in the thread that calls this callback function
    UpdateImage(moviegroup, movie, stream);
    return S_OK;
}

void QmlTestApp::UpdateImage(int /*moviegroup*/, int /*movie*/, int /*stream*/) {
    if (!m_movie_loader.HasMovie()) {
        IImageSourcePtr source = m_host_patient->GetImageSource();
        m_movie_loader.LoadImageSource(source);
        m_movie_loader.SetOutputSize(DEFAULT_IMAGE_RESOLUTION[0], DEFAULT_IMAGE_RESOLUTION[1]);
        m_movie_loader.LoadStreams();
    }

    assert(!m_movie_loader.Is3d());
    IImage2dFramePtr t_frame, cf_frame;
    std::tie(t_frame, cf_frame) = m_movie_loader.GetFrame2d(0);

    QImage image = client::QImageConverter::Frame2d(t_frame, cf_frame, m_movie_loader.GetColorMaps());

    client::PosStream & pos_stream = m_movie_loader.GetPosStream();
    if (pos_stream.stream) {
        // add position-checkerboard grid to image (debugging aid)
        AppAPI::Image2d t_image = t_frame->GetData();

        if (image.format() == QImage::Format_Indexed8) {
            PosSample pos_sample = pos_stream.ClosestSample(t_image.time);
            if (pos_sample.time != 0)
                client::AddCheckerboardPattern(image, m_movie_loader.Get2dGeom(), pos_sample);
        }
    }

    emit imageAvailable(image);
}


#ifdef _WIN32
ULONG QmlTestApp::AddRef() {
    return ++m_ref_count;
}

ULONG QmlTestApp::Release() {
    ULONG ref_cnt = --m_ref_count;

    if (!ref_cnt)
        delete this;

    return ref_cnt;
}
#endif

HRESULT QmlTestApp::QueryInterface(const IID & iid, /*out*/void** ptr) {
    if (!ptr)
        return E_POINTER;

    *ptr = nullptr;

    if (iid == __uuidof(IUnknown))
        *ptr = static_cast<IUnknown*>(this);
    else if (iid == __uuidof(IPluginApp))
        *ptr = static_cast<IPluginApp*>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

void QmlTestApp::ChangeDisplayMode() {
    if (m_current_display_mode == DisplayMode::DISPLAY_LARGE) {
        m_plugin_host_basic->RequestDisplayMode(DisplayMode::DISPLAY_SMALL);
    } else if (m_current_display_mode == DisplayMode::DISPLAY_SMALL) {
        m_plugin_host_basic->RequestDisplayMode(DisplayMode::DISPLAY_TINY);
    } else if (m_current_display_mode == DisplayMode::DISPLAY_TINY) {
        m_plugin_host_basic->RequestDisplayMode(DisplayMode::DISPLAY_LARGE);
    }
}

void QmlTestApp::QuitPlugin() {
    m_plugin_host_basic->RequestDisplayMode(DisplayMode::DISPLAY_QUIT);
}

/** Factory function. */
CComPtr<IQmlPluginApp> CreateQmlTestApp (QQmlEngine *engine, QObject *parent, QQmlContext *context) {
    Q_INIT_RESOURCE(app); // force-link to app.qrc
    qmlRegisterType<ImageItem>("QmlTestApp", 1, 0, "ImageItem");

    return CComPtr<IQmlPluginApp>(new QmlTestApp(engine, parent, context));
}
