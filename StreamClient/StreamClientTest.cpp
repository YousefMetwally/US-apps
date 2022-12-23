#include "stdafx.h"
#include "iostream"

#include "include/StreamClient.hpp"
#include <sstream>
#include <memory>
#include <wrl/client.h>
#include <tuple>
#include <dxgi.h>
#include <cassert>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

int main(int argc, char **argv)
{
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deferredContext;
    Microsoft::WRL::ComPtr<IDXGIFactory> factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;

    // Create a Direct3D device with associated immediate context.
    {
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), &factory);
        assert(SUCCEEDED(hr));

        hr = factory->EnumAdapters(0, &adapter);
        assert(SUCCEEDED(hr));

        UINT32 createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_UNKNOWN;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

        D3D_FEATURE_LEVEL featureLevel;
        hr = D3D11CreateDevice(
            adapter.Get(), driverType, 0, createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION, &device, &featureLevel, &immediateContext);
        assert(SUCCEEDED(hr));

        hr = device->CreateDeferredContext(0, &deferredContext);
        assert(SUCCEEDED(hr));
    }

    // Create StreamClient object
    auto sc = new StreamClient(device.Get(), std::string(argv[1]), 1e-3f);
    std::cout << "StreamClientTest connected" << std::endl;

    while (true)
    {
        sc->Update(immediateContext.Get()); // Get latest data

        std::cout << "StreamClientTest updated" << std::endl;

        const auto mode = sc->GetMode(); //Get the current mode 2D/3D or std::logic_error will be thrown

        StreamClient::Geom geom{};
        if (mode == StreamClient::Mode::LOAD_3D) {
            std::cout << "StreamClientTest in 3D mode" << std::endl;
            auto updateData = sc->Get3dTexture();

            // Skip current empty data.
            if (updateData.empty())
                continue;

            // extract geometry
            geom = std::get<1>(updateData[0]);
        }
        else if (mode == StreamClient::Mode::LOAD_2D) {
            std::cout << "StreamClientTest in 2D mode" << std::endl;
            auto updateData = sc->Get2dTexture();

            // Skip current empty data.
            if (updateData.empty())
                continue;

            // extract geometry
            geom = std::get<1>(updateData[0]);
        }

        {
            std::cout << "Received Geomtery dir1.x: " << geom.dir1[0] << " dir1.y: " << geom.dir1[1] << " dir1.z: " << geom.dir1[2] << std::endl;
            std::cout << "Received Geomtery origin.x:" << geom.origin[0] << " origin.y: " << geom.origin[1] << " origin.z: " << geom.origin[2] << std::endl;

            double ecgFirstTime = 0.0 , ecgLastTime = 0.0;
            auto ecg = sc->GetEcgTrace(ecgFirstTime, ecgLastTime);
            std::cout << "Received ECG samples" << ecg.size() << " first time:" << ecgFirstTime << " last time: " << ecgLastTime << std::endl;

            double auxFirstTime, auxLastTime;
            auto aux = sc->GetAuxTrace(auxFirstTime, auxLastTime);
            std::cout << "Received AUX samples" << aux.size() << " first time:" << auxFirstTime << " last time: " << auxLastTime << std::endl;
        }
    }

    delete sc;
    return 0;
}
