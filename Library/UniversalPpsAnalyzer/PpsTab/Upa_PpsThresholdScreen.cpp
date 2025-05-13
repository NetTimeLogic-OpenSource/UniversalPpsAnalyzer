#include <Upa_PpsThresholdScreen.h>
#include <Upa_PpsTab.h>
#include <ui_Upa_PpsThresholdScreen.h>

Upa_PpsThresholdScreen::Upa_PpsThresholdScreen(Upa_PpsTab *parent) : QDialog()
{
    pps_tab = parent;

    ui = new Ui::Upa_PpsThresholdScreen();
    ui->setupUi(this);

    //QPalette pal;
    //pal.setColor(QPalette::Background, Qt::white);
    //setPalette(pal);

    connect(ui->PpsDoneButton, SIGNAL(clicked()), this, SLOT(pps_done_button_clicked()));
    connect(ui->PpsChangeThresholdsButton, SIGNAL(clicked()), this, SLOT(pps_change_thresholds_button_clicked()));
    connect(ui->PpsAnalyzerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pps_analyzer_changed(int)));
    connect(ui->PpsThresholdRefCheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));
    connect(ui->PpsThreshold1CheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));
    connect(ui->PpsThreshold2CheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));
    connect(ui->PpsThreshold3CheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));
    connect(ui->PpsThreshold4CheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));
    connect(ui->PpsThreshold5CheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));
    connect(ui->PpsThreshold6CheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));
    connect(ui->PpsThreshold7CheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));
    connect(ui->PpsThreshold8CheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_threshold_changed()));

    // add the boards found to the drop down
    for (int i = 0; i < pps_tab->pps_boards.size(); i++)
    {
        ui->PpsAnalyzerComboBox->addItem(pps_tab->pps_boards.at(i)->com_port);
    }

    pps_load_config();

    pps_analyzer_changed(0);

}

Upa_PpsThresholdScreen::~Upa_PpsThresholdScreen()
{
    pps_save_config();
    delete ui;
}

void Upa_PpsThresholdScreen::pps_done_button_clicked(void)
{
    close();
}

void Upa_PpsThresholdScreen::pps_reload(void)
{
    pps_analyzer_changed(0);
}

void Upa_PpsThresholdScreen::pps_change_thresholds_button_clicked(void)
{
    for (int i = 0; i < pps_tab->pps_boards.size(); i++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == pps_tab->pps_boards.at(i)->com_port)
        {
            pps_tab->pps_boards.at(i)->pps_ctrl[0].pps_threshold_high = ui->PpsThresholdPpsHighRefValue->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[1].pps_threshold_high = ui->PpsThresholdPpsHigh1Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[2].pps_threshold_high = ui->PpsThresholdPpsHigh2Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[3].pps_threshold_high = ui->PpsThresholdPpsHigh3Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[4].pps_threshold_high = ui->PpsThresholdPpsHigh4Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[5].pps_threshold_high = ui->PpsThresholdPpsHigh5Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[6].pps_threshold_high = ui->PpsThresholdPpsHigh6Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[7].pps_threshold_high = ui->PpsThresholdPpsHigh7Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[8].pps_threshold_high = ui->PpsThresholdPpsHigh8Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[0].pps_threshold_low = ui->PpsThresholdPpsLowRefValue->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[1].pps_threshold_low = ui->PpsThresholdPpsLow1Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[2].pps_threshold_low = ui->PpsThresholdPpsLow2Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[3].pps_threshold_low = ui->PpsThresholdPpsLow3Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[4].pps_threshold_low = ui->PpsThresholdPpsLow4Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[5].pps_threshold_low = ui->PpsThresholdPpsLow5Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[6].pps_threshold_low = ui->PpsThresholdPpsLow6Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[7].pps_threshold_low = ui->PpsThresholdPpsLow7Value->text().toInt(nullptr, 10);
            pps_tab->pps_boards.at(i)->pps_ctrl[8].pps_threshold_low = ui->PpsThresholdPpsLow8Value->text().toInt(nullptr, 10);
        }
    }

    pps_save_config();
}

void Upa_PpsThresholdScreen::pps_analyzer_changed(int index)
{
    ui->PpsThresholdPpsHighRefValue->setEnabled(false);
    ui->PpsThresholdPpsHigh1Value->setEnabled(false);
    ui->PpsThresholdPpsHigh2Value->setEnabled(false);
    ui->PpsThresholdPpsHigh3Value->setEnabled(false);
    ui->PpsThresholdPpsHigh4Value->setEnabled(false);
    ui->PpsThresholdPpsHigh5Value->setEnabled(false);
    ui->PpsThresholdPpsHigh6Value->setEnabled(false);
    ui->PpsThresholdPpsHigh7Value->setEnabled(false);
    ui->PpsThresholdPpsHigh8Value->setEnabled(false);
    ui->PpsThresholdPpsLowRefValue->setEnabled(false);
    ui->PpsThresholdPpsLow1Value->setEnabled(false);
    ui->PpsThresholdPpsLow2Value->setEnabled(false);
    ui->PpsThresholdPpsLow3Value->setEnabled(false);
    ui->PpsThresholdPpsLow4Value->setEnabled(false);
    ui->PpsThresholdPpsLow5Value->setEnabled(false);
    ui->PpsThresholdPpsLow6Value->setEnabled(false);
    ui->PpsThresholdPpsLow7Value->setEnabled(false);
    ui->PpsThresholdPpsLow8Value->setEnabled(false);
    ui->PpsThresholdRefCheckBox->setEnabled(false);
    ui->PpsThreshold1CheckBox->setEnabled(false);
    ui->PpsThreshold2CheckBox->setEnabled(false);
    ui->PpsThreshold3CheckBox->setEnabled(false);
    ui->PpsThreshold4CheckBox->setEnabled(false);
    ui->PpsThreshold5CheckBox->setEnabled(false);
    ui->PpsThreshold6CheckBox->setEnabled(false);
    ui->PpsThreshold7CheckBox->setEnabled(false);
    ui->PpsThreshold8CheckBox->setEnabled(false);
    ui->PpsThresholdRefCheckBox->setChecked(false);
    ui->PpsThreshold1CheckBox->setChecked(false);
    ui->PpsThreshold2CheckBox->setChecked(false);
    ui->PpsThreshold3CheckBox->setChecked(false);
    ui->PpsThreshold4CheckBox->setChecked(false);
    ui->PpsThreshold5CheckBox->setChecked(false);
    ui->PpsThreshold6CheckBox->setChecked(false);
    ui->PpsThreshold7CheckBox->setChecked(false);
    ui->PpsThreshold8CheckBox->setChecked(false);

    for (int i = 0; i < pps_tab->pps_boards.size(); i++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == pps_tab->pps_boards.at(i)->com_port)
        {
            ui->PpsThresholdPpsHighRefValue->setEnabled(true);
            ui->PpsThresholdPpsHigh1Value->setEnabled(true);
            ui->PpsThresholdPpsHigh2Value->setEnabled(true);
            ui->PpsThresholdPpsHigh3Value->setEnabled(true);
            ui->PpsThresholdPpsHigh4Value->setEnabled(true);
            ui->PpsThresholdPpsHigh5Value->setEnabled(true);
            ui->PpsThresholdPpsHigh6Value->setEnabled(true);
            ui->PpsThresholdPpsHigh7Value->setEnabled(true);
            ui->PpsThresholdPpsHigh8Value->setEnabled(true);
            ui->PpsThresholdPpsHighRefValue->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[0].pps_threshold_high));
            ui->PpsThresholdPpsHigh1Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[1].pps_threshold_high));
            ui->PpsThresholdPpsHigh2Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[2].pps_threshold_high));
            ui->PpsThresholdPpsHigh3Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[3].pps_threshold_high));
            ui->PpsThresholdPpsHigh4Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[4].pps_threshold_high));
            ui->PpsThresholdPpsHigh5Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[5].pps_threshold_high));
            ui->PpsThresholdPpsHigh6Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[6].pps_threshold_high));
            ui->PpsThresholdPpsHigh7Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[7].pps_threshold_high));
            ui->PpsThresholdPpsHigh8Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[8].pps_threshold_high));

            ui->PpsThresholdPpsLowRefValue->setEnabled(true);
            ui->PpsThresholdPpsLow1Value->setEnabled(true);
            ui->PpsThresholdPpsLow2Value->setEnabled(true);
            ui->PpsThresholdPpsLow3Value->setEnabled(true);
            ui->PpsThresholdPpsLow4Value->setEnabled(true);
            ui->PpsThresholdPpsLow5Value->setEnabled(true);
            ui->PpsThresholdPpsLow6Value->setEnabled(true);
            ui->PpsThresholdPpsLow7Value->setEnabled(true);
            ui->PpsThresholdPpsLow8Value->setEnabled(true);
            ui->PpsThresholdPpsLowRefValue->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[0].pps_threshold_low));
            ui->PpsThresholdPpsLow1Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[1].pps_threshold_low));
            ui->PpsThresholdPpsLow2Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[2].pps_threshold_low));
            ui->PpsThresholdPpsLow3Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[3].pps_threshold_low));
            ui->PpsThresholdPpsLow4Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[4].pps_threshold_low));
            ui->PpsThresholdPpsLow5Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[5].pps_threshold_low));
            ui->PpsThresholdPpsLow6Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[6].pps_threshold_low));
            ui->PpsThresholdPpsLow7Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[7].pps_threshold_low));
            ui->PpsThresholdPpsLow8Value->setText(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[8].pps_threshold_low));

            ui->PpsThresholdRefCheckBox->setEnabled(true);
            ui->PpsThreshold1CheckBox->setEnabled(true);
            ui->PpsThreshold2CheckBox->setEnabled(true);
            ui->PpsThreshold3CheckBox->setEnabled(true);
            ui->PpsThreshold4CheckBox->setEnabled(true);
            ui->PpsThreshold5CheckBox->setEnabled(true);
            ui->PpsThreshold6CheckBox->setEnabled(true);
            ui->PpsThreshold7CheckBox->setEnabled(true);
            ui->PpsThreshold8CheckBox->setEnabled(true);
            if (pps_tab->pps_boards.at(i)->pps_ctrl[0].pps_threshold_enabled != 0)
            {
                ui->PpsThresholdRefCheckBox->setChecked(true);
            }
            if (pps_tab->pps_boards.at(i)->pps_ctrl[1].pps_threshold_enabled != 0)
            {
                ui->PpsThreshold1CheckBox->setChecked(true);
            }
            if (pps_tab->pps_boards.at(i)->pps_ctrl[2].pps_threshold_enabled != 0)
            {
                ui->PpsThreshold2CheckBox->setChecked(true);
            }
            if (pps_tab->pps_boards.at(i)->pps_ctrl[3].pps_threshold_enabled != 0)
            {
                ui->PpsThreshold3CheckBox->setChecked(true);
            }
            if (pps_tab->pps_boards.at(i)->pps_ctrl[4].pps_threshold_enabled != 0)
            {
                ui->PpsThreshold4CheckBox->setChecked(true);
            }
            if (pps_tab->pps_boards.at(i)->pps_ctrl[5].pps_threshold_enabled != 0)
            {
                ui->PpsThreshold5CheckBox->setChecked(true);
            }
            if (pps_tab->pps_boards.at(i)->pps_ctrl[6].pps_threshold_enabled != 0)
            {
                ui->PpsThreshold6CheckBox->setChecked(true);
            }
            if (pps_tab->pps_boards.at(i)->pps_ctrl[7].pps_threshold_enabled != 0)
            {
                ui->PpsThreshold7CheckBox->setChecked(true);
            }
            if (pps_tab->pps_boards.at(i)->pps_ctrl[8].pps_threshold_enabled != 0)
            {
                ui->PpsThreshold8CheckBox->setChecked(true);
            }
        }
    }
}

void Upa_PpsThresholdScreen::pps_threshold_changed(void)
{
    for (int i = 0; i < pps_tab->pps_boards.size(); i++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == pps_tab->pps_boards.at(i)->com_port)
        {
            if (ui->PpsThresholdRefCheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[0].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[0].pps_threshold_enabled = 0;
            }
            if (ui->PpsThreshold1CheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[1].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[1].pps_threshold_enabled = 0;
            }
            if (ui->PpsThreshold2CheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[2].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[2].pps_threshold_enabled = 0;
            }
            if (ui->PpsThreshold3CheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[3].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[3].pps_threshold_enabled = 0;
            }
            if (ui->PpsThreshold4CheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[4].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[4].pps_threshold_enabled = 0;
            }
            if (ui->PpsThreshold5CheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[5].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[5].pps_threshold_enabled = 0;
            }
            if (ui->PpsThreshold6CheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[6].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[6].pps_threshold_enabled = 0;
            }
            if (ui->PpsThreshold7CheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[7].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[7].pps_threshold_enabled = 0;
            }
            if (ui->PpsThreshold8CheckBox->isChecked() == true)
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[8].pps_threshold_enabled = 1;
            }
            else
            {
                pps_tab->pps_boards.at(i)->pps_ctrl[8].pps_threshold_enabled = 0;
            }
        }
    }
}

void Upa_PpsThresholdScreen::pps_load_config(void)
{
    QFile temp_file("./UpaThresholdConfig.cfg");

    if (false == temp_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    while (false == temp_file.atEnd())
    {
        QByteArray temp_line = temp_file.readLine();
        QString temp_string = QString::fromUtf8(temp_line.data());

        QRegularExpression sep("\\s+");
        for (int i = 0; i < pps_tab->pps_boards.size(); i++)
        {
            if (true == temp_string.startsWith(pps_tab->pps_boards.at(i)->com_port))
            {
                for (int j = 0; j < Upa_PpsPerBoard; j++)
                {
                    if (pps_tab->pps_boards.at(i)->pps_ts[j].ts_core_config->core_instance_nr == temp_string.section(sep, 1, 1).toUInt(nullptr, 10))
                    {
                        pps_tab->pps_boards.at(i)->pps_ctrl[j].pps_threshold_high = temp_string.section(sep, 2, 2).toInt(nullptr, 10);
                        pps_tab->pps_boards.at(i)->pps_ctrl[j].pps_threshold_low = temp_string.section(sep, 3, 3).toInt(nullptr, 10);
                        pps_tab->pps_boards.at(i)->pps_ctrl[j].pps_threshold_enabled = temp_string.section(sep, 4, 4).toInt(nullptr, 10);
                    }
                }
            }
        }
    }

    temp_file.close();
}

void Upa_PpsThresholdScreen::pps_save_config(void)
{
    QString temp_string;

    for (int i = 0; i < pps_tab->pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            temp_string.append(pps_tab->pps_boards.at(i)->com_port);
            temp_string.append(" ");
            temp_string.append(QString::number(pps_tab->pps_boards.at(i)->pps_ts[j].ts_core_config->core_instance_nr));
            temp_string.append(" ");
            temp_string.append(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[j].pps_threshold_high));
            temp_string.append(" ");
            temp_string.append(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[j].pps_threshold_low));
            temp_string.append(" ");
            temp_string.append(QString::number(pps_tab->pps_boards.at(i)->pps_ctrl[j].pps_threshold_enabled));
            temp_string.append("\n");
        }
    }

    QFile temp_file("./UpaThresholdConfig.cfg");
    if (false == temp_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream temp_stream(&temp_file);
    temp_stream << temp_string << "\n"; //endl;

    temp_file.close();
}
