#pragma once
#include <atomic>
#include <memory>
#include <QImage>
#include <QQmlComponent>
using IPluginAppT = AppAPI::IPluginApp; // IQmlPluginApp compatibility tweak (placed above QmlPluginApp.hpp include)
#include <AppAPI/QmlPluginApp.hpp>
#include "../shared/MovieLoader.hpp"

/** QML-based AppAPI app. Must implement IPluginApp and QML interface, so that the host can initialize it and add it to the host app. */
class QmlTestApp : public QObject, public IQmlPluginApp {
    Q_OBJECT
public:
    QmlTestApp(QQmlEngine *engine, QObject *parent, QQmlContext *context);

    ~QmlTestApp();

    // QmlPluginApp interface
    QQuickItem* GetQuickItem () override;

    //IPluginApp interface
    HRESULT STDMETHODCALLTYPE raw_Initialize(AppAPI::StartupMode /*mode*/, AppAPI::InputMode /*input_mode*/, /*in*/BSTR /*locale*/, AppAPI::IPluginHostBasic *, AppAPI::IPluginHostPatient *) override;

    HRESULT STDMETHODCALLTYPE raw_OnColorMapChanged(AppAPI::ColorMapType /*type*/, int /*mg*/, int /*movie*/) override {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE raw_OnImageSourceChanged() override;

    HRESULT STDMETHODCALLTYPE raw_OnImageSliceViewChanged() override {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE raw_OnCustomViewSettingsChanged() override;

    HRESULT STDMETHODCALLTYPE raw_OnCurrentFrameChanged(int moviegroup, int movie, int stream) override;

    HRESULT STDMETHODCALLTYPE raw_OnUserControlChanged(AppAPI::IControllerBasic * /*ctrl*/, AppAPI::UserEventType /*event*/) override {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE raw_OnDisplayModeChanged(AppAPI::DisplayMode display_mode) override {
        m_current_display_mode = display_mode;
        return S_OK;
    }

#ifdef _WIN32
    ULONG STDMETHODCALLTYPE AddRef() override;

    ULONG STDMETHODCALLTYPE Release() override;
#endif

    HRESULT STDMETHODCALLTYPE QueryInterface(const IID & iid, /*out*/void** ptr) override;

signals:
    void imageAvailable(QImage);
    void viewSettingsChanged(AppAPI::ViewSettings);

private:
    Q_SLOT void UpdateImage(int moviegroup, int movie, int stream);
    Q_SLOT void ChangeDisplayMode();
    Q_SLOT void QuitPlugin();

#ifdef _WIN32
    std::atomic<int>            m_ref_count {0};
#endif
    const unsigned int          DEFAULT_IMAGE_RESOLUTION[2] = {256, 256};

    client::MovieLoader         m_movie_loader;
    QQmlComponent             * m_component = nullptr; ///< weak ptr.
    std::unique_ptr<QQuickItem> m_qml_item;
    AppAPI::IPluginHostPatientPtr m_host_patient;
    AppAPI::IPluginHostBasicPtr m_plugin_host_basic;
    AppAPI::DisplayMode         m_current_display_mode = AppAPI::DISPLAY_SMALL;
    AppAPI::ViewSettings        m_view_cfg{};
};
