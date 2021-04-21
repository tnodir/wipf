#ifndef PROGRAMSWINDOW_H
#define PROGRAMSWINDOW_H

#include "../../util/window/widgetwindow.h"

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QDateTimeEdit)
QT_FORWARD_DECLARE_CLASS(QDialog)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QRadioButton)

class AppInfoCache;
class AppInfoRow;
class AppListModel;
class CheckSpinCombo;
class ConfManager;
class FirewallConf;
class FortManager;
class FortSettings;
class ProgramsController;
class TableView;
class WidgetWindowStateWatcher;

struct AppRow;

class ProgramsWindow : public WidgetWindow
{
    Q_OBJECT

public:
    explicit ProgramsWindow(FortManager *fortManager, QWidget *parent = nullptr);

    ProgramsController *ctrl() const { return m_ctrl; }
    FortManager *fortManager() const;
    FortSettings *settings() const;
    ConfManager *confManager() const;
    FirewallConf *conf() const;
    AppListModel *appListModel() const;
    AppInfoCache *appInfoCache() const;

    void saveWindowState();
    void restoreWindowState();

    bool openAppEditFormByPath(const QString &appPath);

protected slots:
    void onRetranslateUi();

private:
    void setupController();
    void setupStateWatcher();

    void retranslateAppBlockInHours();

    void setupUi();
    void setupAppEditForm();
    QLayout *setupAppEditFormAppLayout();
    QLayout *setupAppEditFormAppPathLayout();
    QLayout *setupAppEditFormAppNameLayout();
    void setupComboAppGroups();
    QLayout *setupAppEditFormAllowLayout();
    QLayout *setupCheckDateTimeEdit();
    void setupAllowEclusiveGroup();
    QLayout *setupHeader();
    void setupLogOptions();
    void setupLogBlocked();
    void setupTableApps();
    void setupTableAppsHeader();
    void setupAppInfoRow();
    void setupTableAppsChanged();

    void updateAppEditForm(bool editCurrentApp);
    void openAppEditFormByRow(const AppRow &appRow, bool editCurrentApp, bool isSingleSelection);
    void activateAppEditForm();

    bool saveAppEditForm();
    bool saveAppEditFormMulti(const QString &appPath, const QString &appName,
            const QDateTime &endTime, int groupIndex, bool useGroupPerm, bool blocked);
    bool saveAppEditFormCheckEdited(const QString &appPath, const QString &appName,
            const QDateTime &endTime, int groupIndex, bool useGroupPerm, bool blocked, bool &ok);

    void updateApp(int row, bool blocked);
    void deleteApp(int row);

    void updateSelectedApps(bool blocked);
    void deleteSelectedApps();

    int appListCurrentIndex() const;
    QString appListCurrentPath() const;

private:
    bool m_formAppForSelected = false;

    ProgramsController *m_ctrl = nullptr;
    WidgetWindowStateWatcher *m_stateWatcher = nullptr;

    QPushButton *m_btEdit = nullptr;
    QAction *m_actAllowApp = nullptr;
    QAction *m_actBlockApp = nullptr;
    QAction *m_actAddApp = nullptr;
    QAction *m_actEditApp = nullptr;
    QAction *m_actRemoveApp = nullptr;
    QAction *m_actPurgeApps = nullptr;
    QPushButton *m_btAllowApp = nullptr;
    QPushButton *m_btBlockApp = nullptr;
    QPushButton *m_btRemoveApp = nullptr;
    QLabel *m_labelEditPath = nullptr;
    QLineEdit *m_editPath = nullptr;
    QPushButton *m_btSelectFile = nullptr;
    QLabel *m_labelEditName = nullptr;
    QLineEdit *m_editName = nullptr;
    QPushButton *m_btGetName = nullptr;
    QLabel *m_labelAppGroup = nullptr;
    QComboBox *m_comboAppGroup = nullptr;
    QCheckBox *m_cbUseGroupPerm = nullptr;
    QRadioButton *m_rbAllowApp = nullptr;
    QRadioButton *m_rbBlockApp = nullptr;
    CheckSpinCombo *m_cscBlockAppIn = nullptr;
    QCheckBox *m_cbBlockAppAt = nullptr;
    QDateTimeEdit *m_dteBlockAppAt = nullptr;
    QCheckBox *m_cbBlockAppNone = nullptr;
    QPushButton *m_btEditOk = nullptr;
    QPushButton *m_btEditCancel = nullptr;
    QDialog *m_formAppEdit = nullptr;
    QPushButton *m_btLogOptions = nullptr;
    QCheckBox *m_cbLogBlocked = nullptr;
    TableView *m_appListView = nullptr;
    AppInfoRow *m_appInfoRow = nullptr;
};

#endif // PROGRAMSWINDOW_H
