#pragma once

#include <QWidget>
#include <memory>

namespace Ui {
class PatientHandler;
}

class PluginImpl;

class PatientHandler : public QWidget
{
    Q_OBJECT

    public:
        explicit PatientHandler(QWidget *parent = 0);
        ~PatientHandler();

        void SetPluginImpl(PluginImpl*  impl);

    public Q_SLOTS:
        void GetPatientData();

    private:
        void InitializeUI();

        std::unique_ptr<Ui::PatientHandler> m_ui;
        PluginImpl*  m_impl;
};
