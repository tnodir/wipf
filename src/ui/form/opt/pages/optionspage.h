#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H

#include "basepage.h"

class OptionsPage : public BasePage
{
    Q_OBJECT

public:
    explicit OptionsPage(OptionsController *ctrl = nullptr, QWidget *parent = nullptr);

protected slots:
    void onAboutToSave() override;
    void onEditResetted() override;

    void onCancelChanges(IniOptions *oldIni) override;

    void onRetranslateUi() override;

private:
    void saveAutoRunMode(int mode);
    void saveService(bool isService);

    void retranslateComboStartMode();
    void retranslateEditPassword();
    void retranslateDriverMessage();

    void setupStartup();

    void setupUi();
    QLayout *setupColumn1();
    void setupStartupBox();
    QLayout *setupStartModeLayout();
    void setupTrafficBox();
    void setupGlobalBox();
    QLayout *setupPasswordLayout();
    void setupEditPassword();
    void setupPasswordLock();
    QLayout *setupLangLayout();
    void setupComboLanguage();
    void setupLogsBox();
    QLayout *setupColumn2();
    void setupDriverBox();
    void setupDriverIcon();
    void setupNewVersionBox();
    void setupNewVersionUpdate();

private:
    qint8 m_currentAutoRunMode = 0;

    QGroupBox *m_gbStartup = nullptr;
    QGroupBox *m_gbTraffic = nullptr;
    QGroupBox *m_gbGlobal = nullptr;
    QGroupBox *m_gbLogs = nullptr;
    QGroupBox *m_gbDriver = nullptr;
    QGroupBox *m_gbNewVersion = nullptr;
    QLabel *m_labelStartMode = nullptr;
    QComboBox *m_comboAutoRun = nullptr;
    QCheckBox *m_cbService = nullptr;
    QCheckBox *m_cbProvBoot = nullptr;
    QCheckBox *m_cbFilterEnabled = nullptr;
    QCheckBox *m_cbFilterLocals = nullptr;
    QCheckBox *m_cbStopTraffic = nullptr;
    QCheckBox *m_cbStopInetTraffic = nullptr;
    QCheckBox *m_cbAllowAllNew = nullptr;
    QCheckBox *m_cbExplorerMenu = nullptr;
    QCheckBox *m_cbHotKeys = nullptr;
    QCheckBox *m_cbPassword = nullptr;
    QLineEdit *m_editPassword = nullptr;
    QPushButton *m_btPasswordLock = nullptr;
    QLabel *m_labelLanguage = nullptr;
    QComboBox *m_comboLanguage = nullptr;
    QCheckBox *m_cbLogDebug = nullptr;
    QCheckBox *m_cbLogConsole = nullptr;
    QLabel *m_iconDriver = nullptr;
    QLabel *m_labelDriverMessage = nullptr;
    QPushButton *m_btInstallDriver = nullptr;
    QPushButton *m_btRemoveDriver = nullptr;
    QLabel *m_labelNewVersion = nullptr;
    QPushButton *m_btNewVersion = nullptr;
};

#endif // OPTIONSPAGE_H
