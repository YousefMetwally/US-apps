#include "PluginImpl.h"
#ifndef Q_MOC_RUN
#include <AppAPI/AppAPI.tlh>
#endif
#include <QUuid>

using namespace AppAPI;

PluginImpl::PluginImpl() : m_interactive_mode(true)
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

HRESULT PluginImpl::raw_Initialize(StartupMode mode, InputMode /*input_mode*/, /*in*/BSTR /*locale*/, IPluginHostBasic * basic_host, IPluginHostPatient * pat_host) {
    if (!basic_host || !pat_host)
        return E_INVALIDARG;

    m_basic_host = basic_host;

    m_logger = basic_host->GetLogReceiver();
    m_interactive_mode = (mode != STARTUP_NONINTERACTIVE);

    m_php = pat_host;

    // retrieve initial view settings from host
    OnCustomViewSettingsChanged();

    m_source = pat_host->GetImageSource();


    SRCode app_code = {L"QtWidgetApp", L"99TEST", L"QtWidgetApp sample application"};
    IMeasurementSeriesPtr meas = pat_host->CreateMeasSeries(&app_code);

    SRCode finding_site = {L"T-32600", L"SRT", L"Left Ventricle"};
    m_meas_findings = meas->GetFindings(&finding_site); // collection of LV measurements

    m_patient = pat_host->GetPatientDataSource();

    // query host for whitelisted file paths 
    CComSafeArray<BSTR> file_paths;
    {
        SAFEARRAY * tmp = pat_host->GetFilePaths();
        file_paths.Attach(tmp);
        tmp = nullptr;
    }
    for (size_t i = 0; i < file_paths.GetCount(); ++i) {
        CComBSTR elm = file_paths.GetAt(static_cast<LONG>(i));
        m_file_paths.push_back(elm.m_str);
    }

    return S_OK;
}


void PluginImpl::Log (MessageSeverity type, QString message)
{
    if (!m_logger)
        return; // no log receiver connected

    m_logger->Log(type, message.toStdWString().c_str());
}

HRESULT PluginImpl::raw_OnColorMapChanged(ColorMapType /*type*/, int /*mg*/, int /*movie*/)
{
    emit m_signaler.OnColorMapChanged();
    return S_OK;
}

HRESULT PluginImpl::raw_OnImageSourceChanged() {
    emit m_signaler.OnImageSourceChanged();
    return S_OK;
}

HRESULT PluginImpl::raw_OnCustomViewSettingsChanged()
{
    assert(m_php);

    ViewSettings view = {};
    SAFEARRAY * sa_tmp = nullptr;
    HRESULT hr = m_php->raw_GetCustomViewSettings(&view, &sa_tmp);
    if (FAILED(hr))
        return hr;

    CComSafeArray<BYTE> data;
    data.Attach(sa_tmp);

    emit m_signaler.OnSetViewSettingsChanged(view);
    return S_OK;
}

HRESULT PluginImpl::raw_OnUserControlChanged(IControllerBasic * /*ctrl*/, UserEventType /*event*/)
{
    // Not implemented yet
    return E_NOTIMPL;
}

HRESULT PluginImpl::raw_OnDisplayModeChanged(DisplayMode /*display_mode*/)
{
    return E_NOTIMPL;
}