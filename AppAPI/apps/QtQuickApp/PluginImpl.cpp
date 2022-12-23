#include "PluginImpl.hpp"
#include <QUuid>
#include <QMessageBox>
#include <AppAPI/AppAPI.tlh>

using namespace AppAPI;

PluginImpl::PluginImpl(QWidget *main_window) :
    m_main_window(main_window)
{
}

PluginImpl::~PluginImpl()
{
}

#ifdef QT_AXSERVER_LIB
long PluginImpl::queryInterface(const QUuid& iid, void** iface)
{
    *iface = nullptr;
    if(iid == __uuidof(IPluginApp))
        *iface = static_cast<IPluginApp*>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}
#endif

HRESULT PluginImpl::raw_Initialize(StartupMode mode, InputMode /*input_mode*/, /*in*/BSTR /*locale*/, /*in*/IPluginHostBasic * basic_host, /*in*/IPluginHostPatient * pat_host) {
    if (!basic_host)
        return E_INVALIDARG;

    m_basic_host = basic_host;

    IPluginLoggerPtr log;

#if defined _WIN32 && !defined _WINDLL
    try
#endif
    {
        // Get interfaces
        log = basic_host->GetLogReceiver();
        assert(log);

        m_pat_host = pat_host;
        InitializeMovie();
        // Call back to our parent
        emit m_signaller.ParentSet();

        return S_OK;
    } 
#if defined _WIN32 && !defined _WINDLL
    // catch at component boundary when running out-of-process
    catch (const std::exception & e) {
        if (mode != STARTUP_NONINTERACTIVE) {
            QMessageBox::critical(m_main_window, "Unable to open plugin", e.what());
        } else if (log) {
            QString msg = e.what();
            log->Log(MSG_ERROR, msg.toStdWString().c_str());
        }
        return E_FAIL;
    }
#else
    (void)mode;
    (void)m_main_window;
#endif
}

HRESULT PluginImpl::raw_OnColorMapChanged(ColorMapType /*type*/, int /*mg*/, int /*movie*/)
{
    emit m_signaller.ColorMapChanged();
    return S_OK;
}

HRESULT PluginImpl::raw_OnImageSourceChanged()
{
    m_cart_movie_2d = nullptr; // invalidate movie

    emit m_signaller.ImageSourceChanged();
    return S_OK;
}

HRESULT PluginImpl::raw_OnCustomViewSettingsChanged()
{
    // Not implemented yet
    return E_NOTIMPL;
}

HRESULT PluginImpl::raw_OnUserControlChanged(IControllerBasic * /*ctrl*/, UserEventType /*event*/)
{
    // Not implemented yet
    return E_NOTIMPL;
}

HRESULT PluginImpl::raw_OnDisplayModeChanged(DisplayMode /*display_mode*/) {
    return E_NOTIMPL;
}

IImage2dMoviePtr PluginImpl::GetMovie2d()
{
    if (!m_cart_movie_2d)
        InitializeMovie();

    return m_cart_movie_2d; 
}

void PluginImpl::InitializeMovie()
{
    assert(m_pat_host);
    IImageSourcePtr source = m_pat_host->GetImageSource();
    IMovieGroupPtr mg;
    if (SUCCEEDED(source->raw_GetMovieGroup(0, &mg))) { // 1st movie group
        IUnknownPtr movie;
        ImageMovieType type = IMAGE_MOVIE_INVALID;
        HRESULT hr = mg->raw_GetMovie(0, PROCESSING_CARTESIAN, &type, &movie); // 1st movie
        if (SUCCEEDED(hr) && (type == IMAGE_MOVIE_2D)) {
            m_cart_movie_2d = movie;
            assert(m_cart_movie_2d);
        }
    }
}
