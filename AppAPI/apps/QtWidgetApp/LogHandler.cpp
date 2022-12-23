#include "LogHandler.h"
#include "ui_LogHandler.h"
#include "PluginImpl.h"
#include <QMessageBox>

LogHandler::LogHandler(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::LogHandler)
{
    InitializeUI();
}

LogHandler::~LogHandler()
{
}

void LogHandler::SetPluginImpl(PluginImpl* impl)
{
    m_impl = impl;
}

void LogHandler::AddLog()
{
    QString message = m_ui->textEdit_log->toPlainText();
    if (message.isEmpty()) {
        if (m_impl->Interactive())
            QMessageBox::critical(this, "Empty message", "It is not allowed to add empty log message.");
        else
            m_impl->Log(AppAPI::MSG_ERROR, "It is not allowed to add empty log message.");
        return;
    }

    m_impl->Log(static_cast<AppAPI::MessageSeverity>(m_ui->comboBox_logType->currentData().toUInt()), message);
}

void LogHandler::InitializeUI()
{
    m_ui->setupUi(this);

    m_ui->comboBox_logType->addItem("Debug", QVariant(static_cast<uint>(AppAPI::MSG_DEBUG)));
    m_ui->comboBox_logType->addItem("Info", QVariant(static_cast<uint>(AppAPI::MSG_INFO)));
    m_ui->comboBox_logType->addItem("Warning", QVariant(static_cast<uint>(AppAPI::MSG_WARNING)));
    m_ui->comboBox_logType->addItem("Error", QVariant(static_cast<uint>(AppAPI::MSG_ERROR)));
    m_ui->comboBox_logType->addItem("Security", QVariant(static_cast<uint>(AppAPI::MSG_SECURITY)));
}
