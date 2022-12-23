#pragma once
#include <QObject>
#ifdef QT_AXSERVER_LIB
  #include <QAxAggregated>
#endif
#ifndef Q_MOC_RUN
#include <AppAPI/AppAPI.tlh>
#endif

#include <atomic>


class PluginImplSignaller : public QObject {
    Q_OBJECT

signals:
    void ParentSet();
    void ColorMapChanged();
    void ImageSourceChanged();
};

class PluginImpl :
#ifdef QT_AXSERVER_LIB
    public QAxAggregated,
#endif
    public AppAPI::IPluginApp
{
public:
    PluginImpl(QWidget *main_window);
    ~PluginImpl() override;

#ifdef QT_AXSERVER_LIB
    long queryInterface(const QUuid& iid, void** iface) override;

    QAXAGG_IUNKNOWN
#else
    BEGIN_COM_MAP(PluginImpl)
        COM_INTERFACE_ENTRY(IPluginApp)
    END_COM_MAP()
#endif

    //IPluginApp
    HRESULT STDMETHODCALLTYPE raw_Initialize(AppAPI::StartupMode mode, AppAPI::InputMode /*input_mode*/, /*in*/BSTR locale, /*in*/AppAPI::IPluginHostBasic *, /*in*/AppAPI::IPluginHostPatient *) override;
    HRESULT STDMETHODCALLTYPE raw_OnColorMapChanged(AppAPI::ColorMapType type, int mg, int movie) override;
    HRESULT STDMETHODCALLTYPE raw_OnImageSourceChanged() override;
    HRESULT STDMETHODCALLTYPE raw_OnImageSliceViewChanged() override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE raw_OnCustomViewSettingsChanged() override;
    HRESULT STDMETHODCALLTYPE raw_OnCurrentFrameChanged(int /*moviegroup*/, int /*movie*/, int /*stream*/) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE raw_OnUserControlChanged(AppAPI::IControllerBasic * ctrl, AppAPI::UserEventType event) override;
    HRESULT STDMETHODCALLTYPE raw_OnDisplayModeChanged(AppAPI::DisplayMode display_mode) override;

    PluginImplSignaller*   GetSignaller() { return &m_signaller; }
    AppAPI::IImage2dMoviePtr GetMovie2d();

    AppAPI::IPluginHostBasic& PluginHostBasic() const {
        return *m_basic_host;
    }

private:
    void InitializeMovie();

    QWidget                     * m_main_window = nullptr; ///< Weak pointer, used to show message box.
    AppAPI::IPluginHostPatientPtr m_pat_host;
    AppAPI::IPluginHostBasicPtr   m_basic_host;
    PluginImplSignaller           m_signaller;
    AppAPI::IImage2dMoviePtr      m_cart_movie_2d;
};
