#include "PatientHandler.h"
#include "ui_PatientHandler.h"
#include <AppAPI/AppAPI.tlh>
#include "PluginImpl.h"

using namespace AppAPI;

PatientHandler::PatientHandler(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::PatientHandler)
{
    InitializeUI();
}

PatientHandler::~PatientHandler()
{
}

void PatientHandler::SetPluginImpl(PluginImpl* impl)
{
    m_impl = impl;
}

void PatientHandler::GetPatientData()
{
    int param = m_ui->comboBox_patientParam->currentData().toUInt();
    QString val;

    m_ui->lineEdit_patientParam->clear();

    try {
        if ((param > PAT_STR_INVALID) && (param < PAT_STR_END)) {
            _bstr_t ws = m_impl->Patient().GetParamStr(static_cast<PatientParamStr>(param));
            val = QString::fromWCharArray(ws);
        }
        else if ((param > PAT_INT_INVALID) && (param < PAT_INT_END)) {
            int tmp = m_impl->Patient().GetParamInt(static_cast<PatientParamInt>(param));
            val = QString::number(tmp);
        }
        else if ((param > PAT_FLOAT_INVALID) && (param < PAT_FLOAT_END)) {
            double tmp = m_impl->Patient().GetParamFloat(static_cast<PatientParamFloat>(param));
            val = QString::number(tmp);
        }
        else {
            throw std::logic_error("unknown patient param type");
        }

        m_ui->lineEdit_patientParam->setText(val);
        m_impl->Log(MSG_INFO, "patient data retrieved");
    }
    catch (std::exception & /*e*/) {
        m_impl->Log(MSG_ERROR, "Failed to retrieve patient data");
    }
}

void PatientHandler::InitializeUI()
{
    m_ui->setupUi(this);

    m_ui->comboBox_patientParam->addItem("ID", QVariant(static_cast<uint>(PAT_ID)));
    m_ui->comboBox_patientParam->addItem("First name", QVariant(static_cast<uint>(PAT_NAME_FIRST)));
    m_ui->comboBox_patientParam->addItem("Middle name", QVariant(static_cast<uint>(PAT_NAME_MIDDLE)));
    m_ui->comboBox_patientParam->addItem("Last name", QVariant(static_cast<uint>(PAT_NAME_LAST)));
    m_ui->comboBox_patientParam->addItem("Height", QVariant(static_cast<uint>(PAT_HEIGHT)));
    m_ui->comboBox_patientParam->addItem("Weight", QVariant(static_cast<uint>(PAT_WEIGHT)));
    m_ui->comboBox_patientParam->addItem("Gender", QVariant(static_cast<uint>(PAT_GENDER)));
    m_ui->comboBox_patientParam->addItem("Birth Year", QVariant(static_cast<uint>(PAT_BIRTH_YEAR)));
    m_ui->comboBox_patientParam->addItem("Birth month", QVariant(static_cast<uint>(PAT_BIRTH_MONTH)));
    m_ui->comboBox_patientParam->addItem("Birth day", QVariant(static_cast<uint>(PAT_BIRTH_DAY)));
    m_ui->comboBox_patientParam->addItem("Systolic pressure", QVariant(static_cast<uint>(PAT_BLOOD_PRESSURE_SYS)));
    m_ui->comboBox_patientParam->addItem("Diastolic pressure", QVariant(static_cast<uint>(PAT_BLOOD_PRESSURE_DIA)));
}
