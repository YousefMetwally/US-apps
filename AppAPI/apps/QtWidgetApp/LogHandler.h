#pragma once
#include <QWidget>
#include <memory>

namespace Ui {
class LogHandler;
}

class PluginImpl;

class LogHandler : public QWidget
{
    Q_OBJECT

    public:
        explicit LogHandler(QWidget *parent = 0);
        ~LogHandler();

        void SetPluginImpl(PluginImpl* impl);

    public Q_SLOTS:
        void AddLog();

    private:
        void InitializeUI();

        std::unique_ptr<Ui::LogHandler> m_ui;
        PluginImpl* m_impl;
};
