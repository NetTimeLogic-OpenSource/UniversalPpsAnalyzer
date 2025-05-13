#include <Upa_HelpMenuAbout.h>
#include <ui_Upa_HelpMenuAbout.h>
#include <Upa_UniversalPpsAnalyzer.h>

Upa_HelpMenuAbout::Upa_HelpMenuAbout() : QWidget()
{
    QString temp_version;

    ui = new Ui::Upa_HelpMenuAbout();
    ui->setupUi(this);

    //QPalette pal;
    //pal.setColor(QPalette::Background, Qt::white);
    //setPalette(pal);

    temp_version.append("Version: ");
    temp_version.append(Upa_Version_Nr);
    temp_version.append(" Build time: ");
    temp_version.append(__TIME__);
    temp_version.append(" ");
    temp_version.append(__DATE__);
    ui->AboutVersionValue->setText(temp_version);

    connect(ui->AboutOkButton, SIGNAL(clicked()), this, SLOT(about_ok_button_clicked()));
}

Upa_HelpMenuAbout::~Upa_HelpMenuAbout()
{
    delete ui;
}

void Upa_HelpMenuAbout::about_ok_button_clicked(void)
{
    close();
}
