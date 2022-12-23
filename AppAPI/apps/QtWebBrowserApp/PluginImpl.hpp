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
    HRESULT STDMETHODCALLTYPE raw_OnColorMapChanged(AppAPI::ColorMapType /*type*/, int /*mg*/, int /*movie*/) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE raw_OnImageSourceChanged() override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE raw_OnImageSliceViewChanged() override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE raw_OnCustomViewSettingsChanged() override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE raw_OnCurrentFrameChanged(int /*moviegroup*/, int /*movie*/, int /*stream*/) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE raw_OnUserControlChanged(AppAPI::IControllerBasic * /*ctrl*/, AppAPI::UserEventType /*event*/) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE raw_OnDisplayModeChanged(AppAPI::DisplayMode /*display_mode*/) override { return E_NOTIMPL; }

    PluginImplSignaller*   GetSignaller() { return &m_signaller; }

private:
    QWidget*                    m_main_window = nullptr; ///< Weak pointer, used to show message box.
    PluginImplSignaller         m_signaller;
};
