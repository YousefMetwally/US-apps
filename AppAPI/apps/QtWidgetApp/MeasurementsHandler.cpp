#include "MeasurementsHandler.h"
#include "ui_MeasurementsHandler.h"
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include "PluginImpl.h"

using namespace AppAPI;

MeasurementsHandler::MeasurementsHandler(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::MeasurementsHandler)
{
    InitializeUI();
}

MeasurementsHandler::~MeasurementsHandler()
{
}

void MeasurementsHandler::SetPluginImpl(PluginImpl* impl)
{
    m_impl = impl;
}

void MeasurementsHandler::AddMeas()
{
    MeasurementData meas = GetCurrentMeasurementData();

    try {
        unsigned int index = m_impl->Meas().Add(&meas);
        static_cast<QLineEdit*>(m_ui->tableWidget_meas->cellWidget(0, MEAS_INDEX))->setText(QString::number(index + 1));
        m_impl->Log(MSG_INFO, "measurement added");
    }
    catch (std::exception & /*e*/) {
        if (m_impl->Interactive())
            QMessageBox::critical(this, "Error", "Failed to add measurement.");
        else
            m_impl->Log(MSG_ERROR, "Failed to add measurement.");
    }
}

void MeasurementsHandler::GetMeas()
{
    try {
        unsigned int index = static_cast<QLineEdit*>(m_ui->tableWidget_meas->cellWidget(0, MEAS_INDEX))->text().toUInt() - 1;
        MeasurementData meas = m_impl->Meas().Get(index);

        std::wstring name = ToWString(meas.disp);
        QComboBox*   unitBox = static_cast<QComboBox*>(m_ui->tableWidget_meas->cellWidget(0, MEAS_UNIT));
        QLineEdit*   valueEdit = static_cast<QLineEdit*>(m_ui->tableWidget_meas->cellWidget(0, MEAS_VALUE));

        // search for unitIndex for a given measurement unit
        int unitIndex = -1;
        for (int i = 0; i < unitBox->count(); ++i) {
            if (static_cast<UnitType>(unitBox->itemData(i).toUInt()) != meas.unit)
                continue;

            unitIndex = i;
            break;
        }

        unitBox->setCurrentIndex(unitIndex);
        if (meas.unit == UNIT_TEXT)
            valueEdit->setText(QString::fromStdWString(ToWString(meas.text_val)));
        else
            valueEdit->setText(QString::number(meas.num_val));

        m_ui->tableWidget_meas->item(0, MEAS_NAME)->setText(QString::fromStdWString(name));
        m_impl->Log(MSG_INFO, "measurement retrieved");
    }
    catch (std::exception & /*e*/) {
        if (m_impl->Interactive())
            QMessageBox::critical(this, "Wrong index", "Failed to get measurement. Measurement with provided index does not exist.");
        else
            m_impl->Log(MSG_ERROR, "Failed to get measurement. Measurement with provided index does not exist.");
    }
}

MeasurementData MeasurementsHandler::GetCurrentMeasurementData()
{
    QString name = m_ui->tableWidget_meas->item(0, MEAS_NAME)->text();
    QString value = static_cast<QLineEdit*>(m_ui->tableWidget_meas->cellWidget(0, MEAS_VALUE))->text();

    if (name.isEmpty() || value.isEmpty()) {
        if (m_impl->Interactive())
            QMessageBox::warning(this, "Missing data", "Either name or value fields are empty. Please fill them in order to continue");
        else
            m_impl->Log(MSG_WARNING, "Either name or value fields are empty. Please fill them in order to continue");
        return MeasurementData();
    }

    QComboBox* unitBox = static_cast<QComboBox*>(m_ui->tableWidget_meas->cellWidget(0, MEAS_UNIT));
    UnitType unit = static_cast<UnitType>(unitBox->currentData().toUInt());

    MeasurementData meas;
    if (unit == UNIT_TEXT)
        meas = MeasurementData(name.toStdWString(), SRCode(name.toStdWString(), L"99TEST", name.toStdWString()), value.toStdWString());
    else
        meas = MeasurementData(name.toStdWString(), SRCode(name.toStdWString(), L"99TEST", name.toStdWString()), locale().toFloat(value), unit);

    meas.image_view = SRCode(L"1234", L"99TEST", L"Apical 4 chamber");

    m_impl->Log(MSG_DEBUG, "current measurement data retrieved");
    return meas;
}

void MeasurementsHandler::InitializeUI()
{
    m_ui->setupUi(this);

    QComboBox* unitBox = new QComboBox(m_ui->tableWidget_meas);
    QLineEdit* indexEdit = new QLineEdit(m_ui->tableWidget_meas);
    QLineEdit* valueEdit = new QLineEdit(m_ui->tableWidget_meas);

    m_ui->tableWidget_meas->setCellWidget(0, MEAS_INDEX, indexEdit);
    indexEdit->setValidator(new QIntValidator(0, 10000, indexEdit));

    m_ui->tableWidget_meas->setCellWidget(0, MEAS_VALUE, valueEdit);
    valueEdit->setValidator(new QDoubleValidator(valueEdit));

    connect(unitBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int /*index*/){
        UnitType unit = static_cast<UnitType>(unitBox->currentData().toUInt());
        if (unit == UNIT_TEXT)
            valueEdit->setValidator(nullptr); // enable free-text content
        else
            valueEdit->setValidator(new QDoubleValidator(valueEdit));
    });

    const UnitType meas_units[] = {UNIT_LENGTH_MM, UNIT_AREA_CM2, UNIT_VOLUME_ML, UNIT_TIME_MS, UNIT_FREQUENCY_BPM, UNIT_MASS_G, UNIT_PRESSURE_MMHG, UNIT_RATIO_PERCENT, UNIT_TEXT};
    for (const UnitType unit : meas_units)
        unitBox->addItem(QString::fromStdWString(ToWString(UnitTypeToSR(unit).value)), QVariant(static_cast<uint>(unit)));

    unitBox->setEditable(false);

    m_ui->tableWidget_meas->setCellWidget(0, MEAS_UNIT, unitBox);
    m_ui->tableWidget_meas->setItem(0, MEAS_NAME, new QTableWidgetItem(""));
}
