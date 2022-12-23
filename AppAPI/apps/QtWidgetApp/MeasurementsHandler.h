#pragma once
#include <QWidget>
#include <memory>
#include <AppAPI/AppAPI.tlh>

namespace Ui {
class MeasurementsHandler;
}

class PluginImpl;

class MeasurementsHandler : public QWidget
{
    Q_OBJECT

    public:
        explicit MeasurementsHandler(QWidget *parent = 0);
        ~MeasurementsHandler();

        void SetPluginImpl(PluginImpl*  impl);

    public Q_SLOTS:
        void AddMeas();
        void GetMeas();

    private:
        enum meas_column_idx_t {
            MEAS_INDEX = 0,
            MEAS_NAME,
            MEAS_VALUE,
            MEAS_UNIT
        };

        void InitializeUI();
        AppAPI::MeasurementData GetCurrentMeasurementData();

        std::unique_ptr<Ui::MeasurementsHandler> m_ui;
        PluginImpl*  m_impl;
};
