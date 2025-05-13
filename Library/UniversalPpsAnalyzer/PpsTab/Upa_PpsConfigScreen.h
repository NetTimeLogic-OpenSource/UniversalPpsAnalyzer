#ifndef UPA_PPSCONFIGSCREEN_H
#define UPA_PPSCONFIGSCREEN_H

#include <QDialog>
#include <ui_Upa_PpsTab.h>

class Upa_PpsTab;

namespace Ui {
class Upa_PpsConfigScreen;
}

class Upa_PpsConfigScreen : public QDialog
{
    Q_OBJECT
private:
    // Pps tab
    Upa_PpsTab* pps_tab;
    void pps_change_delays(void);
    void pps_change_names(void);
    void pps_save_config(void);
    void pps_load_config(void);

public:
    Upa_PpsConfigScreen(Upa_PpsTab* parent);
    ~Upa_PpsConfigScreen();
    void pps_reload(void);

    Ui::Upa_PpsConfigScreen *ui;

private slots:
    // PPS Delay screen
    void pps_done_button_clicked(void);
    void pps_change_delays_and_names_button_clicked(void);
    void pps_change_reference_button_clicked(void);
    void pps_analyzer_changed(int index);
    void pps_graph_changed(void);
    void pps_calc_variance_changed(void);
};

#endif // UPA_PPSCONFIGSCREEN_H
