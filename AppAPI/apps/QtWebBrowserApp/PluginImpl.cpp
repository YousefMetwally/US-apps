#include "PluginImpl.hpp"
#include <QUuid>
#include <QMessageBox>

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

HRESULT PluginImpl::raw_Initialize(StartupMode mode, InputMode /*input_mode*/, /*in*/BSTR /*locale*/, /*in*/IPluginHostBasic * basic_host, /*in*/IPluginHostPatient * /*pat_host*/) {
    if (!basic_host)
        return E_INVALIDARG;

    IPluginLoggerPtr log;

#if defined _WIN32 && !defined _WINDLL
    try
#endif
    {
        // Get interfaces
        log = basic_host->GetLogReceiver();
        assert(log);

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
#endif
}
