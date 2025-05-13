#ifndef UPA_HELPMENUABOUT_H
#define UPA_HELPMENUABOUT_H

#include <QWidget>

namespace Ui {
class Upa_HelpMenuAbout;
}

class Upa_HelpMenuAbout : public QWidget
{
    Q_OBJECT
public:
    Upa_HelpMenuAbout();
    ~Upa_HelpMenuAbout();

    Ui::Upa_HelpMenuAbout *ui;

private slots:
    // About screen
    void about_ok_button_clicked(void);
};

#endif // UPA_HELPMENUABOUT_H
