#ifndef UPA_PPSTHRESHOLDSCREEN_H
#define UPA_PPSTHRESHOLDSCREEN_H

#include <QDialog>
#include <ui_Upa_PpsTab.h>

class Upa_PpsTab;

namespace Ui {
class Upa_PpsThresholdScreen;
}

class Upa_PpsThresholdScreen : public QDialog
{
    Q_OBJECT
private:
    // Pps tab
    Upa_PpsTab* pps_tab;
    void pps_save_config(void);
    void pps_load_config(void);

public:
    Upa_PpsThresholdScreen(Upa_PpsTab* parent);
    ~Upa_PpsThresholdScreen();
    void pps_reload(void);

    Ui::Upa_PpsThresholdScreen *ui;

private slots:
    // PPS Threshold screen
    void pps_done_button_clicked(void);
    void pps_change_thresholds_button_clicked(void);
    void pps_analyzer_changed(int index);
    void pps_threshold_changed(void);
};

#endif // UPA_PPSTHRESHOLDSCREEN_H
