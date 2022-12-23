#pragma once
#ifndef Q_MOC_RUN
#include <AppAPI/AppAPI.tlh>
#endif
#include <QWidget>
#ifdef QT_AXSERVER_LIB
#  include <QAxAggregated>
#endif


/** Helper class to enable signals from PluginImpl */
class PluginImplSignaller : public QObject {
    Q_OBJECT

signals:
    void OnCurrentFrameChanged();
    void OnImageSourceChanged();
    void OnColorMapChanged();
    void OnSetViewSettingsChanged(AppAPI::ViewSettings view);
};


class PluginImpl :
#ifdef QT_AXSERVER_LIB
    public QAxAggregated,
#endif
    public AppAPI::IPluginApp
{
    public:
        PluginImpl();
        ~PluginImpl();

#ifdef QT_AXSERVER_LIB
        long queryInterface(const QUuid& iid, void** iface) override;

        QAXAGG_IUNKNOWN
#else
        BEGIN_COM_MAP(PluginImpl)
            COM_INTERFACE_ENTRY(IPluginApp)
        END_COM_MAP()
#endif

        //IPluginApp
        HRESULT STDMETHODCALLTYPE raw_Initialize(AppAPI::StartupMode mode, AppAPI::InputMode /*input_mode*/, /*in*/BSTR locale, AppAPI::IPluginHostBasic *, AppAPI::IPluginHostPatient *) override;
        HRESULT STDMETHODCALLTYPE raw_OnColorMapChanged(AppAPI::ColorMapType type, int mg, int movie) override;
        HRESULT STDMETHODCALLTYPE raw_OnImageSourceChanged() override;
        HRESULT STDMETHODCALLTYPE raw_OnImageSliceViewChanged() override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE raw_OnCustomViewSettingsChanged() override;

        HRESULT STDMETHODCALLTYPE raw_OnCurrentFrameChanged(int /*moviegroup*/, int /*movie*/, int /*stream*/) override {
            // must return quickly from this event to avoid blocking the host
            emit m_signaler.OnCurrentFrameChanged();
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE raw_OnUserControlChanged(AppAPI::IControllerBasic * ctrl, AppAPI::UserEventType event) override;
        HRESULT STDMETHODCALLTYPE raw_OnDisplayModeChanged(AppAPI::DisplayMode display_mode) override;

        AppAPI::IMeasurementFindings & Meas () const {
            return *m_meas_findings;
        }

        AppAPI::IPatientData & Patient () const {
            return *m_patient;
        }

        AppAPI::IPluginHostBasic& PluginHostBasic() const {
            return *m_basic_host;
        }
        
        AppAPI::IImageSourcePtr  GetSource() { return m_source; }
        AppAPI::IPluginLoggerPtr GetLogger() { return m_logger; }

        void Log (AppAPI::MessageSeverity type, QString message);

        bool Interactive () const { return m_interactive_mode; }

        PluginImplSignaller             m_signaler;
    private:
        bool                            m_interactive_mode;
        AppAPI::IPluginHostPatientPtr   m_php;
        AppAPI::IPluginLoggerPtr        m_logger;
        AppAPI::IMeasurementFindingsPtr m_meas_findings;
        AppAPI::IImageSourcePtr         m_source;
        AppAPI::IPatientDataPtr         m_patient;
        AppAPI::IPluginHostBasicPtr     m_basic_host;
        std::vector<std::wstring>       m_file_paths; ///< list of paths where app is allowed to read/write
};
