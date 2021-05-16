#include "statisticspage.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTabBar>
#include <QTableView>
#include <QTimeEdit>
#include <QVBoxLayout>

#include "../../../appinfo/appinfocache.h"
#include "../../../conf/firewallconf.h"
#include "../../../fortmanager.h"
#include "../../../fortsettings.h"
#include "../../../model/appstatmodel.h"
#include "../../../model/traflistmodel.h"
#include "../../../user/iniuser.h"
#include "../../../util/iconcache.h"
#include "../../../util/net/netutil.h"
#include "../../controls/appinforow.h"
#include "../../controls/checktimeperiod.h"
#include "../../controls/controlutil.h"
#include "../../controls/labelcolor.h"
#include "../../controls/labelspin.h"
#include "../../controls/labelspincombo.h"
#include "../../controls/listview.h"
#include "../optionscontroller.h"

namespace {

const ValuesList trafKeepDayValues = { 60, -1, 90, 180, 365, 365 * 3, 365 * 5, 365 * 10 };
const ValuesList trafKeepMonthValues = { 2, -1, 3, 6, 12, 12 * 3, 12 * 5, 12 * 10 };
const ValuesList logIpKeepCountValues = { 3000, 1000, 5000, 10000, 50000, 100000, 500000, 1000000,
    5000000, 10000000 };
const ValuesList quotaValues = { 10, 0, 100, 500, 1024, 8 * 1024, 10 * 1024, 30 * 1024, 50 * 1024,
    100 * 1024 };

}

StatisticsPage::StatisticsPage(OptionsController *ctrl, QWidget *parent) :
    BasePage(ctrl, parent), m_isPageUpdating(false)
{
    setupTrafListModel();

    setupUi();
    updatePage();
}

AppStatModel *StatisticsPage::appStatModel() const
{
    return fortManager()->appStatModel();
}

AppInfoCache *StatisticsPage::appInfoCache() const
{
    return appStatModel()->appInfoCache();
}

void StatisticsPage::setIniEdited()
{
    if (!m_isPageUpdating) {
        ctrl()->setIniEdited();
    }
}

void StatisticsPage::onSaveWindowState(IniUser *ini)
{
    ini->setOptWindowStatSplit(m_splitter->saveState());
}

void StatisticsPage::onRestoreWindowState(IniUser *ini)
{
    m_splitter->restoreState(ini->optWindowStatSplit());
}

void StatisticsPage::onRetranslateUi()
{
    m_btRefresh->setText(tr("Refresh"));
    m_btClear->setText(tr("Clear"));

    m_actRemoveApp->setText(tr("Remove Application"));
    m_actResetTotal->setText(tr("Reset Total"));
    m_actClearAll->setText(tr("Clear All"));

    m_btTrafOptions->setText(tr("Options"));
    m_cbLogStat->setText(tr("Collect Traffic Statistics"));
    m_cbLogStatNoFilter->setText(tr("Collect Traffic, when Filter Disabled"));
    m_ctpActivePeriod->checkBox()->setText(tr("Active time period:"));
    m_lscMonthStart->label()->setText(tr("Month starts on:"));

    m_lscTrafHourKeepDays->label()->setText(tr("Keep data for 'Hourly':"));
    m_lscTrafHourKeepDays->spinBox()->setSuffix(tr(" day(s)"));
    m_lscTrafDayKeepDays->label()->setText(tr("Keep data for 'Daily':"));
    m_lscTrafDayKeepDays->spinBox()->setSuffix(tr(" day(s)"));
    m_lscTrafMonthKeepMonths->label()->setText(tr("Keep data for 'Monthly':"));
    m_lscTrafMonthKeepMonths->spinBox()->setSuffix(tr(" month(s)"));

    m_lscQuotaDayMb->label()->setText(tr("Day's Quota:"));
    m_lscQuotaMonthMb->label()->setText(tr("Month's Quota:"));

    m_lscAllowedIpKeepCount->label()->setText(tr("Keep count for 'Allowed connections':"));
    m_lscBlockedIpKeepCount->label()->setText(tr("Keep count for 'Blocked connections':"));

    retranslateTrafKeepDayNames();
    retranslateTrafKeepMonthNames();
    retranslateQuotaNames();
    retranslateIpKeepCountNames();

    m_btGraphOptions->setText(tr("Graph"));
    m_cbGraphAlwaysOnTop->setText(tr("Always on top"));
    m_cbGraphFrameless->setText(tr("Frameless"));
    m_cbGraphClickThrough->setText(tr("Click through"));
    m_cbGraphHideOnHover->setText(tr("Hide on hover"));
    m_graphOpacity->label()->setText(tr("Opacity:"));
    m_graphHoverOpacity->label()->setText(tr("Hover opacity:"));
    m_graphMaxSeconds->label()->setText(tr("Max seconds:"));
    m_graphColor->label()->setText(tr("Background:"));
    m_graphColorIn->label()->setText(tr("Download:"));
    m_graphColorOut->label()->setText(tr("Upload:"));
    m_graphAxisColor->label()->setText(tr("Axis:"));
    m_graphTickLabelColor->label()->setText(tr("Tick label:"));
    m_graphLabelColor->label()->setText(tr("Label:"));
    m_graphGridColor->label()->setText(tr("Grid:"));

    m_traphUnits->setText(tr("Units:"));
    retranslateTrafUnitNames();

    retranslateTabBar();

    m_appInfoRow->retranslateUi();
}

void StatisticsPage::retranslateTrafKeepDayNames()
{
    const QStringList list = { tr("Custom"), tr("Forever"), tr("3 months"), tr("6 months"),
        tr("1 year"), tr("3 years"), tr("5 years"), tr("10 years") };

    m_lscTrafHourKeepDays->setNames(list);
    m_lscTrafDayKeepDays->setNames(list);
}

void StatisticsPage::retranslateTrafKeepMonthNames()
{
    const QStringList list = { tr("Custom"), tr("Forever"), tr("3 months"), tr("6 months"),
        tr("1 year"), tr("3 years"), tr("5 years"), tr("10 years") };

    m_lscTrafMonthKeepMonths->setNames(list);
}

void StatisticsPage::retranslateQuotaNames()
{
    QStringList list;

    list.append(tr("Custom"));
    list.append(tr("Disabled"));

    int index = 0;
    for (const int v : quotaValues) {
        if (++index > 2) {
            list.append(formatQuota(v));
        }
    }

    m_lscQuotaDayMb->setNames(list);
    m_lscQuotaMonthMb->setNames(list);
}

void StatisticsPage::retranslateIpKeepCountNames()
{
    const QStringList list = { tr("Custom"), "1K", "5K", "10K", "50K", "100K", "500K", "1M", "5M",
        "10M" };

    m_lscAllowedIpKeepCount->setNames(list);
    m_lscBlockedIpKeepCount->setNames(list);
}

void StatisticsPage::retranslateTrafUnitNames()
{
    const QStringList list = { tr("Adaptive"), tr("Bytes"), "KiB", "MiB", "GiB", "TiB" };

    m_comboTrafUnit->clear();
    m_comboTrafUnit->addItems(list);

    updateTrafUnit();
}

void StatisticsPage::retranslateTabBar()
{
    const QStringList list = { tr("Hourly"), tr("Daily"), tr("Monthly"), tr("Total") };

    int index = 0;
    for (const auto &v : list) {
        m_tabBar->setTabText(index++, v);
    }
}

void StatisticsPage::setupTrafListModel()
{
    m_trafListModel = appStatModel()->trafListModel();
}

void StatisticsPage::setupUi()
{
    auto layout = new QVBoxLayout();

    // Header
    auto header = setupHeader();
    layout->addLayout(header);

    // Content
    m_splitter = new QSplitter();

    setupAppListView();
    m_splitter->addWidget(m_appListView);

    // Tab Bar
    auto trafLayout = new QVBoxLayout();
    trafLayout->setContentsMargins(0, 0, 0, 0);

    setupTabBar();
    trafLayout->addWidget(m_tabBar);

    // Traf Table
    setupTableTraf();
    setupTableTrafHeader();
    trafLayout->addWidget(m_tableTraf);

    auto trafWidget = new QWidget();
    trafWidget->setLayout(trafLayout);
    m_splitter->addWidget(trafWidget);

    layout->addWidget(m_splitter, 1);

    // App Info Row
    setupAppInfoRow();
    layout->addWidget(m_appInfoRow);

    // Actions on app list view's current changed
    setupAppListViewChanged();

    this->setLayout(layout);
}

QLayout *StatisticsPage::setupHeader()
{
    auto layout = new QHBoxLayout();

    m_btRefresh =
            ControlUtil::createButton(":/icons/sign-sync.png", [&] { trafListModel()->reset(); });

    setupClearMenu();
    setupTrafUnits();
    setupGraphOptionsMenu();
    setupTrafOptionsMenu();

    layout->addWidget(m_btRefresh);
    layout->addWidget(m_btClear);
    layout->addWidget(ControlUtil::createSeparator(Qt::Vertical));
    layout->addWidget(m_traphUnits);
    layout->addWidget(m_comboTrafUnit);
    layout->addStretch();
    layout->addWidget(m_btGraphOptions);
    layout->addWidget(m_btTrafOptions);

    return layout;
}

void StatisticsPage::setupClearMenu()
{
    auto menu = new QMenu(this);

    m_actRemoveApp = menu->addAction(IconCache::icon(":/icons/sign-delete.png"), QString());
    m_actRemoveApp->setShortcut(Qt::Key_Delete);

    m_actResetTotal = menu->addAction(QString());
    m_actClearAll = menu->addAction(QString());

    connect(m_actRemoveApp, &QAction::triggered, this, [&] {
        if (!fortManager()->showQuestionBox(
                    tr("Are you sure to remove statistics for selected application?")))
            return;

        appStatModel()->remove(appListCurrentIndex());
    });
    connect(m_actResetTotal, &QAction::triggered, this, [&] {
        if (!fortManager()->showQuestionBox(tr("Are you sure to reset total statistics?")))
            return;

        trafListModel()->resetAppTotals();
    });
    connect(m_actClearAll, &QAction::triggered, this, [&] {
        if (!fortManager()->showQuestionBox(tr("Are you sure to clear all statistics?")))
            return;

        m_appListView->clearSelection();
        appStatModel()->clear();
    });

    m_btClear = ControlUtil::createButton(":/icons/trashcan-full.png");
    m_btClear->setMenu(menu);
}

void StatisticsPage::setupTrafUnits()
{
    m_traphUnits = ControlUtil::createLabel();

    m_comboTrafUnit = ControlUtil::createComboBox(QStringList(), [&](int index) {
        if (ini()->trafUnit() == index)
            return;

        ini()->setTrafUnit(index);
        updateTableTrafUnit();

        setIniEdited();
    });
}

void StatisticsPage::setupGraphOptionsMenu()
{
    setupGraphCheckboxes();
    setupGraphOptions();
    setupGraphColors();

    // Menu
    auto colLayout1 = ControlUtil::createLayoutByWidgets({ m_cbGraphAlwaysOnTop, m_cbGraphFrameless,
            m_cbGraphClickThrough, m_cbGraphHideOnHover, ControlUtil::createSeparator(),
            m_graphOpacity, m_graphHoverOpacity, m_graphMaxSeconds, nullptr });
    auto colLayout2 =
            ControlUtil::createLayoutByWidgets({ m_graphColor, m_graphColorIn, m_graphColorOut,
                    m_graphAxisColor, m_graphTickLabelColor, m_graphLabelColor, m_graphGridColor });
    auto layout = new QHBoxLayout();
    layout->addLayout(colLayout1);
    layout->addWidget(ControlUtil::createSeparator(Qt::Vertical));
    layout->addLayout(colLayout2);

    auto menu = ControlUtil::createMenuByLayout(layout, this);

    m_btGraphOptions = ControlUtil::createButton(":/icons/line-graph.png");
    m_btGraphOptions->setMenu(menu);
}

void StatisticsPage::setupGraphCheckboxes()
{
    m_cbGraphAlwaysOnTop = ControlUtil::createCheckBox(false, [&](bool checked) {
        if (ini()->graphWindowAlwaysOnTop() != checked) {
            ini()->setGraphWindowAlwaysOnTop(checked);
            setIniEdited();
        }
    });
    m_cbGraphFrameless = ControlUtil::createCheckBox(false, [&](bool checked) {
        if (ini()->graphWindowFrameless() != checked) {
            ini()->setGraphWindowFrameless(checked);
            setIniEdited();
        }
    });
    m_cbGraphClickThrough = ControlUtil::createCheckBox(false, [&](bool checked) {
        if (ini()->graphWindowClickThrough() != checked) {
            ini()->setGraphWindowClickThrough(checked);
            setIniEdited();
        }
    });
    m_cbGraphHideOnHover = ControlUtil::createCheckBox(false, [&](bool checked) {
        if (ini()->graphWindowHideOnHover() != checked) {
            ini()->setGraphWindowHideOnHover(checked);
            setIniEdited();
        }
    });
}

void StatisticsPage::setupGraphOptions()
{
    m_graphOpacity = createSpin(0, 100, " %");
    m_graphHoverOpacity = createSpin(0, 100, " %");
    m_graphMaxSeconds = createSpin(0, 9999);

    connect(m_graphOpacity->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int v) {
                if (ini()->graphWindowOpacity() != v) {
                    ini()->setGraphWindowOpacity(v);
                    setIniEdited();
                }
            });
    connect(m_graphHoverOpacity->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int v) {
                if (ini()->graphWindowHoverOpacity() != v) {
                    ini()->setGraphWindowHoverOpacity(v);
                    setIniEdited();
                }
            });
    connect(m_graphMaxSeconds->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int v) {
                if (ini()->graphWindowMaxSeconds() != v) {
                    ini()->setGraphWindowMaxSeconds(v);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupGraphColors()
{
    m_graphColor = new LabelColor();
    m_graphColorIn = new LabelColor();
    m_graphColorOut = new LabelColor();
    m_graphAxisColor = new LabelColor();
    m_graphTickLabelColor = new LabelColor();
    m_graphLabelColor = new LabelColor();
    m_graphGridColor = new LabelColor();

    connect(m_graphColor, &LabelColor::colorChanged, this, [&](const QColor &v) {
        if (ini()->graphWindowColor() != v) {
            ini()->setGraphWindowColor(v);
            setIniEdited();
        }
    });
    connect(m_graphColorIn, &LabelColor::colorChanged, this, [&](const QColor &v) {
        if (ini()->graphWindowColorIn() != v) {
            ini()->setGraphWindowColorIn(v);
            setIniEdited();
        }
    });
    connect(m_graphColorOut, &LabelColor::colorChanged, this, [&](const QColor &v) {
        if (ini()->graphWindowColorOut() != v) {
            ini()->setGraphWindowColorOut(v);
            setIniEdited();
        }
    });
    connect(m_graphAxisColor, &LabelColor::colorChanged, this, [&](const QColor &v) {
        if (ini()->graphWindowAxisColor() != v) {
            ini()->setGraphWindowAxisColor(v);
            setIniEdited();
        }
    });
    connect(m_graphTickLabelColor, &LabelColor::colorChanged, this, [&](const QColor &v) {
        if (ini()->graphWindowTickLabelColor() != v) {
            ini()->setGraphWindowTickLabelColor(v);
            setIniEdited();
        }
    });
    connect(m_graphLabelColor, &LabelColor::colorChanged, this, [&](const QColor &v) {
        if (ini()->graphWindowLabelColor() != v) {
            ini()->setGraphWindowLabelColor(v);
            setIniEdited();
        }
    });
    connect(m_graphGridColor, &LabelColor::colorChanged, this, [&](const QColor &v) {
        if (ini()->graphWindowGridColor() != v) {
            ini()->setGraphWindowGridColor(v);
            setIniEdited();
        }
    });
}

void StatisticsPage::setupTrafOptionsMenu()
{
    setupLogStat();
    setupLogStatNoFilter();
    setupActivePeriod();
    setupMonthStart();
    setupTrafHourKeepDays();
    setupTrafDayKeepDays();
    setupTrafMonthKeepMonths();
    setupQuotaDayMb();
    setupQuotaMonthMb();
    setupAllowedIpKeepCount();
    setupBlockedIpKeepCount();

    // Menu
    const QList<QWidget *> menuWidgets = { m_cbLogStat, m_cbLogStatNoFilter, m_ctpActivePeriod,
        m_lscMonthStart, ControlUtil::createSeparator(), m_lscTrafHourKeepDays,
        m_lscTrafDayKeepDays, m_lscTrafMonthKeepMonths, ControlUtil::createSeparator(),
        m_lscQuotaDayMb, m_lscQuotaMonthMb, ControlUtil::createSeparator(), m_lscAllowedIpKeepCount,
        m_lscBlockedIpKeepCount };
    auto layout = ControlUtil::createLayoutByWidgets(menuWidgets);

    auto menu = ControlUtil::createMenuByLayout(layout, this);

    m_btTrafOptions = ControlUtil::createButton(":/icons/wrench.png");
    m_btTrafOptions->setMenu(menu);
}

void StatisticsPage::setupLogStat()
{
    m_cbLogStat = ControlUtil::createCheckBox(false, [&](bool checked) {
        if (conf()->logStat() == checked)
            return;

        conf()->setLogStat(checked);

        ctrl()->setFlagsEdited();
    });

    m_cbLogStat->setFont(ControlUtil::fontDemiBold());
}

void StatisticsPage::setupLogStatNoFilter()
{
    m_cbLogStatNoFilter = ControlUtil::createCheckBox(false, [&](bool checked) {
        if (conf()->logStatNoFilter() == checked)
            return;

        conf()->setLogStatNoFilter(checked);

        ctrl()->setFlagsEdited();
    });
}

void StatisticsPage::setupActivePeriod()
{
    m_ctpActivePeriod = new CheckTimePeriod();

    connect(m_ctpActivePeriod->checkBox(), &QCheckBox::toggled, this, [&](bool checked) {
        if (conf()->activePeriodEnabled() == checked)
            return;

        conf()->setActivePeriodEnabled(checked);

        ctrl()->setFlagsEdited();
    });
    connect(m_ctpActivePeriod->timeEdit1(), &QTimeEdit::userTimeChanged, this,
            [&](const QTime &time) {
                const auto timeStr = CheckTimePeriod::fromTime(time);

                if (conf()->activePeriodFrom() == timeStr)
                    return;

                conf()->setActivePeriodFrom(timeStr);

                ctrl()->setFlagsEdited();
            });
    connect(m_ctpActivePeriod->timeEdit2(), &QTimeEdit::userTimeChanged, this,
            [&](const QTime &time) {
                const auto timeStr = CheckTimePeriod::fromTime(time);

                if (conf()->activePeriodTo() == timeStr)
                    return;

                conf()->setActivePeriodTo(timeStr);

                ctrl()->setFlagsEdited();
            });
}

void StatisticsPage::setupMonthStart()
{
    m_lscMonthStart = createSpinCombo(1, 31);

    // Days list
    {
        ValuesList dayValues;
        dayValues.reserve(31);
        for (int i = 1; i <= 31; ++i) {
            dayValues.append(i);
        }
        m_lscMonthStart->setValues(dayValues);
        m_lscMonthStart->setNamesByValues();
    }

    connect(m_lscMonthStart->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int value) {
                if (ini()->monthStart() != value) {
                    ini()->setMonthStart(value);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupTrafHourKeepDays()
{
    m_lscTrafHourKeepDays = createSpinCombo(-1, 9999);
    m_lscTrafHourKeepDays->setValues(trafKeepDayValues);

    connect(m_lscTrafHourKeepDays->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int value) {
                if (ini()->trafHourKeepDays() != value) {
                    ini()->setTrafHourKeepDays(value);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupTrafDayKeepDays()
{
    m_lscTrafDayKeepDays = createSpinCombo(-1, 9999);
    m_lscTrafDayKeepDays->setValues(trafKeepDayValues);

    connect(m_lscTrafDayKeepDays->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int value) {
                if (ini()->trafDayKeepDays() != value) {
                    ini()->setTrafDayKeepDays(value);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupTrafMonthKeepMonths()
{
    m_lscTrafMonthKeepMonths = createSpinCombo(-1, 9999);
    m_lscTrafMonthKeepMonths->setValues(trafKeepMonthValues);

    connect(m_lscTrafMonthKeepMonths->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int value) {
                if (ini()->trafMonthKeepMonths() != value) {
                    ini()->setTrafMonthKeepMonths(value);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupQuotaDayMb()
{
    m_lscQuotaDayMb = createSpinCombo(0, 1024 * 1024, " MiB");
    m_lscQuotaDayMb->setValues(quotaValues);

    connect(m_lscQuotaDayMb->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int value) {
                const quint32 mbytes = quint32(value);
                if (ini()->quotaDayMb() != mbytes) {
                    ini()->setQuotaDayMb(mbytes);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupQuotaMonthMb()
{
    m_lscQuotaMonthMb = createSpinCombo(0, 1024 * 1024, " MiB");
    m_lscQuotaMonthMb->setValues(quotaValues);

    connect(m_lscQuotaMonthMb->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int value) {
                const quint32 mbytes = quint32(value);
                if (ini()->quotaMonthMb() != mbytes) {
                    ini()->setQuotaMonthMb(mbytes);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupAllowedIpKeepCount()
{
    m_lscAllowedIpKeepCount = new LabelSpinCombo();
    m_lscAllowedIpKeepCount->spinBox()->setRange(0, 999999999);
    m_lscAllowedIpKeepCount->setValues(logIpKeepCountValues);

    connect(m_lscAllowedIpKeepCount->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int value) {
                if (ini()->allowedIpKeepCount() != value) {
                    ini()->setAllowedIpKeepCount(value);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupBlockedIpKeepCount()
{
    m_lscBlockedIpKeepCount = new LabelSpinCombo();
    m_lscBlockedIpKeepCount->spinBox()->setRange(0, 999999999);
    m_lscBlockedIpKeepCount->setValues(logIpKeepCountValues);

    connect(m_lscBlockedIpKeepCount->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [&](int value) {
                if (ini()->blockedIpKeepCount() != value) {
                    ini()->setBlockedIpKeepCount(value);
                    setIniEdited();
                }
            });
}

void StatisticsPage::setupAppListView()
{
    m_appListView = new ListView();
    m_appListView->setFlow(QListView::TopToBottom);
    m_appListView->setViewMode(QListView::ListMode);
    m_appListView->setIconSize(QSize(24, 24));
    m_appListView->setUniformItemSizes(true);
    m_appListView->setAlternatingRowColors(true);

    m_appListView->setModel(appStatModel());
}

void StatisticsPage::setupTabBar()
{
    m_tabBar = new QTabBar();
    m_tabBar->setShape(QTabBar::TriangularNorth);

    for (int n = 4; --n >= 0;) {
        m_tabBar->addTab(QString());
    }
}

void StatisticsPage::setupTableTraf()
{
    m_tableTraf = new QTableView();
    m_tableTraf->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableTraf->setSelectionBehavior(QAbstractItemView::SelectItems);

    m_tableTraf->setModel(trafListModel());

    const auto resetTableTraf = [&] {
        trafListModel()->setType(static_cast<TrafListModel::TrafType>(m_tabBar->currentIndex()));
        trafListModel()->setAppId(appStatModel()->appIdByRow(appListCurrentIndex()));
        trafListModel()->resetTraf();
    };

    resetTableTraf();

    connect(m_tabBar, &QTabBar::currentChanged, this, resetTableTraf);
    connect(m_appListView, &ListView::currentIndexChanged, this, resetTableTraf);
}

void StatisticsPage::setupTableTrafHeader()
{
    auto header = m_tableTraf->horizontalHeader();

    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::Stretch);

    const auto refreshTableTrafHeader = [&] {
        auto hh = m_tableTraf->horizontalHeader();
        hh->resizeSection(0, qBound(150, qRound(hh->width() * 0.3), 180));
    };

    refreshTableTrafHeader();

    connect(header, &QHeaderView::geometriesChanged, this, refreshTableTrafHeader);
}

void StatisticsPage::setupAppInfoRow()
{
    m_appInfoRow = new AppInfoRow();

    const auto refreshAppInfoVersion = [&] {
        m_appInfoRow->refreshAppInfoVersion(appListCurrentPath(), appInfoCache());
    };

    refreshAppInfoVersion();

    connect(m_appListView, &ListView::currentIndexChanged, this, refreshAppInfoVersion);
    connect(appInfoCache(), &AppInfoCache::cacheChanged, this, refreshAppInfoVersion);
}

void StatisticsPage::setupAppListViewChanged()
{
    const auto refreshAppListViewChanged = [&] {
        const bool appSelected = (appListCurrentIndex() > 0);
        m_actRemoveApp->setEnabled(appSelected);
        m_appInfoRow->setVisible(appSelected);
    };

    refreshAppListViewChanged();

    connect(m_appListView, &ListView::currentIndexChanged, this, refreshAppListViewChanged);
}

void StatisticsPage::updatePage()
{
    m_isPageUpdating = true;

    m_cbLogStat->setChecked(conf()->logStat());
    m_cbLogStatNoFilter->setChecked(conf()->logStatNoFilter());

    m_ctpActivePeriod->checkBox()->setChecked(conf()->activePeriodEnabled());
    m_ctpActivePeriod->timeEdit1()->setTime(CheckTimePeriod::toTime(conf()->activePeriodFrom()));
    m_ctpActivePeriod->timeEdit2()->setTime(CheckTimePeriod::toTime(conf()->activePeriodTo()));

    m_lscMonthStart->spinBox()->setValue(ini()->monthStart());
    m_lscTrafHourKeepDays->spinBox()->setValue(ini()->trafHourKeepDays());
    m_lscTrafDayKeepDays->spinBox()->setValue(ini()->trafDayKeepDays());
    m_lscTrafMonthKeepMonths->spinBox()->setValue(ini()->trafMonthKeepMonths());

    m_lscQuotaDayMb->spinBox()->setValue(int(ini()->quotaDayMb()));
    m_lscQuotaMonthMb->spinBox()->setValue(int(ini()->quotaMonthMb()));

    m_lscAllowedIpKeepCount->spinBox()->setValue(ini()->allowedIpKeepCount());
    m_lscBlockedIpKeepCount->spinBox()->setValue(ini()->blockedIpKeepCount());

    m_cbGraphAlwaysOnTop->setChecked(ini()->graphWindowAlwaysOnTop());
    m_cbGraphFrameless->setChecked(ini()->graphWindowFrameless());
    m_cbGraphClickThrough->setChecked(ini()->graphWindowClickThrough());
    m_cbGraphHideOnHover->setChecked(ini()->graphWindowHideOnHover());

    m_graphOpacity->spinBox()->setValue(ini()->graphWindowOpacity());
    m_graphHoverOpacity->spinBox()->setValue(ini()->graphWindowHoverOpacity());
    m_graphMaxSeconds->spinBox()->setValue(ini()->graphWindowMaxSeconds());

    m_graphColor->setColor(ini()->graphWindowColor());
    m_graphColorIn->setColor(ini()->graphWindowColorIn());
    m_graphColorOut->setColor(ini()->graphWindowColorOut());
    m_graphAxisColor->setColor(ini()->graphWindowAxisColor());
    m_graphTickLabelColor->setColor(ini()->graphWindowTickLabelColor());
    m_graphLabelColor->setColor(ini()->graphWindowLabelColor());
    m_graphGridColor->setColor(ini()->graphWindowGridColor());

    updateTrafUnit();
    updateTableTrafUnit();

    m_isPageUpdating = false;
}

void StatisticsPage::updateTrafUnit()
{
    m_comboTrafUnit->setCurrentIndex(ini()->trafUnit());
}

void StatisticsPage::updateTableTrafUnit()
{
    const auto trafUnit = static_cast<TrafListModel::TrafUnit>(ini()->trafUnit());

    if (trafListModel()->unit() != trafUnit) {
        trafListModel()->setUnit(trafUnit);
        trafListModel()->refresh();
    }
}

int StatisticsPage::appListCurrentIndex() const
{
    return m_appListView->currentRow();
}

QString StatisticsPage::appListCurrentPath() const
{
    return appStatModel()->appPathByRow(appListCurrentIndex());
}

LabelSpinCombo *StatisticsPage::createSpinCombo(int min, int max, const QString &suffix)
{
    auto c = new LabelSpinCombo();
    c->spinBox()->setRange(min, max);
    c->spinBox()->setSuffix(suffix);
    return c;
}

LabelSpin *StatisticsPage::createSpin(int min, int max, const QString &suffix)
{
    auto c = new LabelSpin();
    c->spinBox()->setRange(min, max);
    c->spinBox()->setSuffix(suffix);
    return c;
}

QString StatisticsPage::formatQuota(int mbytes)
{
    return NetUtil::formatDataSize1(qint64(mbytes) * 1024 * 1024);
}
