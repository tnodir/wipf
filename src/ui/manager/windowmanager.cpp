#include "windowmanager.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleFactory>

#include "../conf/confmanager.h"
#include "../form/controls/mainwindow.h"
#include "../form/dialog/passworddialog.h"
#include "../form/graph/graphwindow.h"
#include "../form/opt/optionswindow.h"
#include "../form/prog/programswindow.h"
#include "../form/stat/statisticswindow.h"
#include "../form/tray/trayicon.h"
#include "../form/zone/zoneswindow.h"
#include "../fortcompat.h"
#include "../fortsettings.h"
#include "../stat/statmanager.h"
#include "nativeeventfilter.h"
#include "util/ioc/ioccontainer.h"

namespace {

void setupAppStyle()
{
    QStyle *style = QStyleFactory::create("Fusion");
    QApplication::setStyle(style);
    QApplication::setPalette(style->standardPalette());
}

}

WindowManager::WindowManager(QObject *parent) : QObject(parent) { }

void WindowManager::setUp()
{
    setupAppStyle(); // Style & Palette

    setupMainWindow();

    connect(qApp, &QCoreApplication::aboutToQuit, this, &WindowManager::closeAll);
}

void WindowManager::tearDown()
{
    closeAll();
}

void WindowManager::setupMainWindow()
{
    m_mainWindow = new MainWindow();

    auto nativeEventFilter = IoC<NativeEventFilter>();

    nativeEventFilter->registerSessionNotification(mainWindow()->winId());

    connect(nativeEventFilter, &NativeEventFilter::sessionLocked, this, [&] {
        IoC<FortSettings>()->resetCheckedPassword(PasswordDialog::UnlockTillSessionLock);
    });
}

void WindowManager::closeMainWindow()
{
    if (!mainWindow())
        return;

    auto nativeEventFilter = IoC<NativeEventFilter>();

    nativeEventFilter->unregisterHotKeys();
    nativeEventFilter->unregisterSessionNotification(mainWindow()->winId());

    m_mainWindow->deleteLater();
    m_mainWindow = nullptr;
}

void WindowManager::setupTrayIcon()
{
    m_trayIcon = new TrayIcon(this);

    connect(m_trayIcon, &TrayIcon::mouseClicked, this, &WindowManager::showProgramsWindow);
    connect(m_trayIcon, &TrayIcon::mouseDoubleClicked, this, &WindowManager::showOptionsWindow);
    connect(m_trayIcon, &TrayIcon::mouseMiddleClicked, this, &WindowManager::showStatisticsWindow);
    connect(m_trayIcon, &TrayIcon::mouseRightClicked, m_trayIcon, &TrayIcon::showTrayMenu);
    connect(m_trayIcon, &QSystemTrayIcon::messageClicked, this,
            &WindowManager::onTrayMessageClicked);

    auto confManager = IoC<ConfManager>();
    connect(confManager, &ConfManager::confChanged, m_trayIcon, &TrayIcon::updateTrayMenu);
    connect(confManager, &ConfManager::iniUserChanged, m_trayIcon, &TrayIcon::updateTrayMenu);
    connect(confManager, &ConfManager::appAlerted, m_trayIcon,
            [&] { m_trayIcon->updateTrayIcon(true); });
}

void WindowManager::setupProgramsWindow()
{
    m_progWindow = new ProgramsWindow();
    m_progWindow->restoreWindowState();

    connect(m_progWindow, &ProgramsWindow::aboutToClose, this, &WindowManager::closeProgramsWindow);
    connect(m_progWindow, &ProgramsWindow::activationChanged, m_trayIcon,
            [&] { m_trayIcon->updateTrayIcon(false); });
}

void WindowManager::setupOptionsWindow()
{
    m_optWindow = new OptionsWindow();
    m_optWindow->restoreWindowState();

    connect(m_optWindow, &OptionsWindow::aboutToClose, this, &WindowManager::closeOptionsWindow);
}

void WindowManager::setupZonesWindow()
{
    m_zoneWindow = new ZonesWindow();
    m_zoneWindow->restoreWindowState();

    connect(m_zoneWindow, &ZonesWindow::aboutToClose, this, &WindowManager::closeZonesWindow);
}

void WindowManager::setupGraphWindow()
{
    m_graphWindow = new GraphWindow();
    m_graphWindow->restoreWindowState();

    connect(m_graphWindow, &GraphWindow::aboutToClose, this, [&] { closeGraphWindow(); });
    connect(m_graphWindow, &GraphWindow::mouseRightClick, this,
            [&](QMouseEvent *event) { m_trayIcon->showTrayMenu(mouseEventGlobalPos(event)); });

    connect(IoC<StatManager>(), &StatManager::trafficAdded, m_graphWindow,
            &GraphWindow::addTraffic);
}

void WindowManager::setupStatisticsWindow()
{
    m_statWindow = new StatisticsWindow();
    m_statWindow->restoreWindowState();

    connect(m_statWindow, &StatisticsWindow::aboutToClose, this,
            &WindowManager::closeStatisticsWindow);
}

void WindowManager::closeAll()
{
    closeGraphWindow(true);
    closeOptionsWindow();
    closeProgramsWindow();
    closeZonesWindow();
    closeStatisticsWindow();
    closeTrayIcon();
    closeMainWindow();
}

void WindowManager::showTrayIcon()
{
    if (!m_trayIcon) {
        setupTrayIcon();
    }

    m_trayIcon->show();
}

void WindowManager::closeTrayIcon()
{
    m_trayIcon->hide();
}

void WindowManager::showTrayMessage(const QString &message, WindowManager::TrayMessageType type)
{
    if (!m_trayIcon)
        return;

    m_lastMessageType = type;
    m_trayIcon->showMessage(QGuiApplication::applicationDisplayName(), message);
}

void WindowManager::showProgramsWindow()
{
    if (!(m_progWindow && m_progWindow->isVisible()) && !checkPassword())
        return;

    if (!m_progWindow) {
        setupProgramsWindow();
    }

    m_progWindow->show();
    m_progWindow->raise();
    m_progWindow->activateWindow();
}

void WindowManager::closeProgramsWindow()
{
    if (!m_progWindow)
        return;

    m_progWindow->saveWindowState();
    m_progWindow->hide();

    m_progWindow->deleteLater();
    m_progWindow = nullptr;
}

bool WindowManager::showProgramEditForm(const QString &appPath)
{
    showProgramsWindow();
    if (!(m_progWindow && m_progWindow->isVisible()))
        return false; // May be not opened due to password checking

    if (!m_progWindow->editProgramByPath(appPath)) {
        showErrorBox(tr("Please close already opened Edit Program window and try again."));
        return false;
    }
    return true;
}

void WindowManager::showOptionsWindow()
{
    if (!(m_optWindow && m_optWindow->isVisible()) && !checkPassword())
        return;

    if (!m_optWindow) {
        setupOptionsWindow();

        emit optWindowChanged(true);
    }

    m_optWindow->show();
    m_optWindow->raise();
    m_optWindow->activateWindow();
}

void WindowManager::closeOptionsWindow()
{
    if (!m_optWindow)
        return;

    m_optWindow->cancelChanges();
    m_optWindow->saveWindowState();
    m_optWindow->hide();

    m_optWindow->deleteLater();
    m_optWindow = nullptr;

    emit optWindowChanged(false);
}

void WindowManager::reloadOptionsWindow(const QString &reason)
{
    if (!m_optWindow)
        return;

    // Unsaved changes are lost
    closeOptionsWindow();
    showOptionsWindow();

    showTrayMessage(reason);
}

void WindowManager::showStatisticsWindow()
{
    if (!(m_statWindow && m_statWindow->isVisible()) && !checkPassword())
        return;

    if (!m_statWindow) {
        setupStatisticsWindow();
    }

    m_statWindow->show();
    m_statWindow->raise();
    m_statWindow->activateWindow();
}

void WindowManager::closeStatisticsWindow()
{
    if (!m_statWindow)
        return;

    m_statWindow->saveWindowState();
    m_statWindow->hide();

    m_statWindow->deleteLater();
    m_statWindow = nullptr;
}

void WindowManager::showZonesWindow()
{
    if (!(m_zoneWindow && m_zoneWindow->isVisible()) && !checkPassword())
        return;

    if (!m_zoneWindow) {
        setupZonesWindow();
    }

    m_zoneWindow->show();
    m_zoneWindow->raise();
    m_zoneWindow->activateWindow();
}

void WindowManager::closeZonesWindow()
{
    if (!m_zoneWindow)
        return;

    m_zoneWindow->saveWindowState();
    m_zoneWindow->hide();

    m_zoneWindow->deleteLater();
    m_zoneWindow = nullptr;
}

void WindowManager::showGraphWindow()
{
    if (!m_graphWindow) {
        setupGraphWindow();

        emit graphWindowChanged(true);
    }

    m_graphWindow->show();
}

void WindowManager::closeGraphWindow(bool wasVisible)
{
    if (!m_graphWindow)
        return;

    m_graphWindow->saveWindowState(wasVisible);
    m_graphWindow->hide();

    m_graphWindow->deleteLater();
    m_graphWindow = nullptr;

    emit graphWindowChanged(false);
}

void WindowManager::switchGraphWindow()
{
    if (!m_graphWindow)
        showGraphWindow();
    else
        closeGraphWindow();
}

void WindowManager::quitByCheckPassword()
{
    if (!checkPassword())
        return;

    closeAll();

    qDebug() << "Quit due user request";

    QCoreApplication::quit();
}

bool WindowManager::checkPassword()
{
    static bool g_passwordDialogOpened = false;

    const auto settings = IoC<FortSettings>();

    if (!settings->isPasswordRequired())
        return true;

    if (g_passwordDialogOpened) {
        activateModalWidget();
        return false;
    }

    g_passwordDialogOpened = true;

    QString password;
    PasswordDialog::UnlockType unlockType = PasswordDialog::UnlockDisabled;
    const bool ok = PasswordDialog::getPassword(password, unlockType, mainWindow());

    g_passwordDialogOpened = false;

    const bool checked = ok && !password.isEmpty() && IoC<ConfManager>()->checkPassword(password);

    settings->setPasswordChecked(checked, unlockType);

    return checked;
}

void WindowManager::showErrorBox(const QString &text, const QString &title)
{
    QMessageBox::warning(focusWidget(), title, text);
}

void WindowManager::showInfoBox(const QString &text, const QString &title)
{
    QMessageBox::information(focusWidget(), title, text);
}

bool WindowManager::showQuestionBox(const QString &text, const QString &title)
{
    return QMessageBox::question(focusWidget(), title, text) == QMessageBox::Yes;
}

bool WindowManager::showYesNoBox(
        const QString &text, const QString &yesText, const QString &noText, const QString &title)
{
    QMessageBox box(QMessageBox::Information, title, text, QMessageBox::NoButton, focusWidget());
    box.addButton(noText, QMessageBox::NoRole);
    box.addButton(yesText, QMessageBox::YesRole);

    return box.exec() == 1;
}

void WindowManager::onTrayMessageClicked()
{
    switch (m_lastMessageType) {
    case MessageZones:
        showZonesWindow();
        break;
    default:
        showOptionsWindow();
    }
}

QWidget *WindowManager::focusWidget() const
{
    auto w = QApplication::focusWidget();
    return w ? w : mainWindow();
}

void WindowManager::activateModalWidget()
{
    auto w = qApp->activeModalWidget();
    if (w) {
        w->activateWindow();
    }
}
