#include "statisticspage.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTimeEdit>
#include <QVBoxLayout>

#include "../../../conf/firewallconf.h"
#include "../../../util/iconcache.h"
#include "../../../util/net/netutil.h"
#include "../../controls/checktimeperiod.h"
#include "../../controls/controlutil.h"
#include "../../controls/labelcolor.h"
#include "../../controls/labelspin.h"
#include "../../controls/labelspincombo.h"
#include "../optionscontroller.h"

namespace {

const ValuesList dayValues = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };
const ValuesList trafKeepDayValues = { 60, -1, 90, 180, 365, 365 * 3, 365 * 5, 365 * 10 };
const ValuesList trafKeepMonthValues = { 2, -1, 3, 6, 12, 12 * 3, 12 * 5, 12 * 10 };
const ValuesList logIpKeepCountValues = { 3000, 1000, 5000, 10000, 50000, 100000, 500000, 1000000,
    5000000, 10000000 };
const ValuesList quotaValues = { 10, 0, 100, 500, 1024, 8 * 1024, 10 * 1024, 30 * 1024, 50 * 1024,
    100 * 1024 };

LabelSpinCombo *createSpinCombo(int v, int min, int max, const ValuesList &values,
        const QString &suffix, const std::function<void(int value)> &onValueChanged)
{
    auto c = new LabelSpinCombo();
    c->spinBox()->setValue(v);
    c->spinBox()->setRange(min, max);
    c->spinBox()->setSuffix(suffix);
    c->setValues(values);

    c->connect(c->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), onValueChanged);

    return c;
}

LabelSpin *createSpin(int v, int min, int max, const QString &suffix,
        const std::function<void(int value)> &onValueChanged)
{
    auto c = new LabelSpin();
    c->spinBox()->setValue(v);
    c->spinBox()->setRange(min, max);
    c->spinBox()->setSuffix(suffix);

    c->connect(c->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), onValueChanged);

    return c;
}

LabelColor *createLabelColor(
        const QColor &v, const std::function<void(const QColor &color)> &onColorChanged)
{
    auto c = new LabelColor();
    c->setColor(v);

    c->connect(c, &LabelColor::colorChanged, onColorChanged);

    return c;
}

QString formatQuota(int mbytes)
{
    return NetUtil::formatDataSize1(qint64(mbytes) * 1024 * 1024);
}

}

StatisticsPage::StatisticsPage(OptionsController *ctrl, QWidget *parent) : OptBasePage(ctrl, parent)
{
    setupUi();
}

void StatisticsPage::onRetranslateUi()
{
    m_gbTraffic->setTitle(tr("Traffic"));
    m_gbGraph->setTitle(tr("Graph"));

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

void StatisticsPage::setupUi()
{
    // Column #1
    auto colLayout1 = setupColumn1();

    // Column #2
    auto colLayout2 = setupColumn2();

    // Main layout
    auto layout = new QHBoxLayout();
    layout->addLayout(colLayout1);
    layout->addStretch();
    layout->addLayout(colLayout2);
    layout->addStretch();

    this->setLayout(layout);
}

QLayout *StatisticsPage::setupColumn1()
{
    auto layout = new QVBoxLayout();
    layout->setSpacing(10);

    // Traffic Group Box
    setupTrafficBox();
    layout->addWidget(m_gbTraffic);

    layout->addStretch();

    return layout;
}

void StatisticsPage::setupTrafficBox()
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

    // Layout
    auto layout = ControlUtil::createLayoutByWidgets({ m_cbLogStat, m_cbLogStatNoFilter,
            m_ctpActivePeriod, m_lscMonthStart, ControlUtil::createSeparator(),
            m_lscTrafHourKeepDays, m_lscTrafDayKeepDays, m_lscTrafMonthKeepMonths,
            ControlUtil::createSeparator(), m_lscQuotaDayMb, m_lscQuotaMonthMb,
            ControlUtil::createSeparator(), m_lscAllowedIpKeepCount, m_lscBlockedIpKeepCount });

    m_gbTraffic = new QGroupBox(this);
    m_gbTraffic->setLayout(layout);
}

void StatisticsPage::setupLogStat()
{
    m_cbLogStat = ControlUtil::createCheckBox(conf()->logStat(), [&](bool checked) {
        if (conf()->logStat() != checked) {
            conf()->setLogStat(checked);
            ctrl()->setFlagsEdited();
        }
    });

    m_cbLogStat->setFont(ControlUtil::fontDemiBold());
}

void StatisticsPage::setupLogStatNoFilter()
{
    m_cbLogStatNoFilter = ControlUtil::createCheckBox(conf()->logStatNoFilter(), [&](bool checked) {
        if (conf()->logStatNoFilter() != checked) {
            conf()->setLogStatNoFilter(checked);
            ctrl()->setFlagsEdited();
        }
    });
}

void StatisticsPage::setupActivePeriod()
{
    m_ctpActivePeriod = new CheckTimePeriod();
    m_ctpActivePeriod->checkBox()->setChecked(conf()->activePeriodEnabled());
    m_ctpActivePeriod->timeEdit1()->setTime(CheckTimePeriod::toTime(conf()->activePeriodFrom()));
    m_ctpActivePeriod->timeEdit2()->setTime(CheckTimePeriod::toTime(conf()->activePeriodTo()));

    connect(m_ctpActivePeriod->checkBox(), &QCheckBox::toggled, this, [&](bool checked) {
        if (conf()->activePeriodEnabled() != checked) {
            conf()->setActivePeriodEnabled(checked);
            ctrl()->setFlagsEdited();
        }
    });
    connect(m_ctpActivePeriod->timeEdit1(), &QTimeEdit::userTimeChanged, this,
            [&](const QTime &time) {
                const auto timeStr = CheckTimePeriod::fromTime(time);

                if (conf()->activePeriodFrom() != timeStr) {
                    conf()->setActivePeriodFrom(timeStr);
                    ctrl()->setFlagsEdited();
                }
            });
    connect(m_ctpActivePeriod->timeEdit2(), &QTimeEdit::userTimeChanged, this,
            [&](const QTime &time) {
                const auto timeStr = CheckTimePeriod::fromTime(time);

                if (conf()->activePeriodTo() != timeStr) {
                    conf()->setActivePeriodTo(timeStr);
                    ctrl()->setFlagsEdited();
                }
            });
}

void StatisticsPage::setupMonthStart()
{
    m_lscMonthStart = createSpinCombo(ini()->monthStart(), 1, 31, dayValues, {}, [&](int value) {
        if (ini()->monthStart() != value) {
            ini()->setMonthStart(value);
            ctrl()->setIniEdited();
        }
    });
    m_lscMonthStart->setNamesByValues();
}

void StatisticsPage::setupTrafHourKeepDays()
{
    m_lscTrafHourKeepDays = createSpinCombo(
            ini()->trafHourKeepDays(), -1, 9999, trafKeepDayValues, {}, [&](int value) {
                if (ini()->trafHourKeepDays() != value) {
                    ini()->setTrafHourKeepDays(value);
                    ctrl()->setIniEdited();
                }
            });
}

void StatisticsPage::setupTrafDayKeepDays()
{
    m_lscTrafDayKeepDays = createSpinCombo(
            ini()->trafDayKeepDays(), -1, 9999, trafKeepDayValues, {}, [&](int value) {
                if (ini()->trafDayKeepDays() != value) {
                    ini()->setTrafDayKeepDays(value);
                    ctrl()->setIniEdited();
                }
            });
}

void StatisticsPage::setupTrafMonthKeepMonths()
{
    m_lscTrafMonthKeepMonths = createSpinCombo(
            ini()->trafMonthKeepMonths(), -1, 9999, trafKeepMonthValues, {}, [&](int value) {
                if (ini()->trafMonthKeepMonths() != value) {
                    ini()->setTrafMonthKeepMonths(value);
                    ctrl()->setIniEdited();
                }
            });
}

void StatisticsPage::setupQuotaDayMb()
{
    m_lscQuotaDayMb = createSpinCombo(
            int(ini()->quotaDayMb()), 0, 1024 * 1024, quotaValues, " MiB", [&](int value) {
                if (ini()->quotaDayMb() != value) {
                    ini()->setQuotaDayMb(value);
                    ctrl()->setIniEdited();
                }
            });
}

void StatisticsPage::setupQuotaMonthMb()
{
    m_lscQuotaMonthMb = createSpinCombo(
            int(ini()->quotaMonthMb()), 0, 1024 * 1024, quotaValues, " MiB", [&](int value) {
                if (ini()->quotaMonthMb() != value) {
                    ini()->setQuotaMonthMb(value);
                    ctrl()->setIniEdited();
                }
            });
}

void StatisticsPage::setupAllowedIpKeepCount()
{
    m_lscAllowedIpKeepCount = createSpinCombo(
            ini()->allowedIpKeepCount(), 0, 999999999, logIpKeepCountValues, {}, [&](int value) {
                if (ini()->allowedIpKeepCount() != value) {
                    ini()->setAllowedIpKeepCount(value);
                    ctrl()->setIniEdited();
                }
            });
}

void StatisticsPage::setupBlockedIpKeepCount()
{
    m_lscBlockedIpKeepCount = createSpinCombo(
            ini()->blockedIpKeepCount(), 0, 999999999, logIpKeepCountValues, {}, [&](int value) {
                if (ini()->blockedIpKeepCount() != value) {
                    ini()->setBlockedIpKeepCount(value);
                    ctrl()->setIniEdited();
                }
            });
}

QLayout *StatisticsPage::setupColumn2()
{
    auto layout = new QVBoxLayout();
    layout->setSpacing(10);

    // Graph Group Box
    setupGraphBox();
    layout->addWidget(m_gbGraph);

    layout->addStretch();

    return layout;
}

void StatisticsPage::setupGraphBox()
{
    setupGraphCheckboxes();
    setupGraphOptions();
    setupGraphColors();

    // Layout
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

    m_gbGraph = new QGroupBox(this);
    m_gbGraph->setLayout(layout);
}

void StatisticsPage::setupGraphCheckboxes()
{
    m_cbGraphAlwaysOnTop =
            ControlUtil::createCheckBox(ini()->graphWindowAlwaysOnTop(), [&](bool checked) {
                if (ini()->graphWindowAlwaysOnTop() != checked) {
                    ini()->setGraphWindowAlwaysOnTop(checked);
                    ctrl()->setIniEdited();
                }
            });
    m_cbGraphFrameless =
            ControlUtil::createCheckBox(ini()->graphWindowFrameless(), [&](bool checked) {
                if (ini()->graphWindowFrameless() != checked) {
                    ini()->setGraphWindowFrameless(checked);
                    ctrl()->setIniEdited();
                }
            });
    m_cbGraphClickThrough =
            ControlUtil::createCheckBox(ini()->graphWindowClickThrough(), [&](bool checked) {
                if (ini()->graphWindowClickThrough() != checked) {
                    ini()->setGraphWindowClickThrough(checked);
                    ctrl()->setIniEdited();
                }
            });
    m_cbGraphHideOnHover =
            ControlUtil::createCheckBox(ini()->graphWindowHideOnHover(), [&](bool checked) {
                if (ini()->graphWindowHideOnHover() != checked) {
                    ini()->setGraphWindowHideOnHover(checked);
                    ctrl()->setIniEdited();
                }
            });
}

void StatisticsPage::setupGraphOptions()
{
    m_graphOpacity = createSpin(ini()->graphWindowOpacity(), 0, 100, " %", [&](int v) {
        if (ini()->graphWindowOpacity() != v) {
            ini()->setGraphWindowOpacity(v);
            ctrl()->setIniEdited();
        }
    });

    m_graphHoverOpacity = createSpin(ini()->graphWindowHoverOpacity(), 0, 100, " %", [&](int v) {
        if (ini()->graphWindowHoverOpacity() != v) {
            ini()->setGraphWindowHoverOpacity(v);
            ctrl()->setIniEdited();
        }
    });

    m_graphMaxSeconds = createSpin(ini()->graphWindowMaxSeconds(), 0, 9999, {}, [&](int v) {
        if (ini()->graphWindowMaxSeconds() != v) {
            ini()->setGraphWindowMaxSeconds(v);
            ctrl()->setIniEdited();
        }
    });
}

void StatisticsPage::setupGraphColors()
{
    m_graphColor = createLabelColor(ini()->graphWindowColor(), [&](const QColor &v) {
        if (ini()->graphWindowColor() != v) {
            ini()->setGraphWindowColor(v);
            ctrl()->setIniEdited();
        }
    });
    m_graphColorIn = createLabelColor(ini()->graphWindowColorIn(), [&](const QColor &v) {
        if (ini()->graphWindowColorIn() != v) {
            ini()->setGraphWindowColorIn(v);
            ctrl()->setIniEdited();
        }
    });
    m_graphColorOut = createLabelColor(ini()->graphWindowColorOut(), [&](const QColor &v) {
        if (ini()->graphWindowColorOut() != v) {
            ini()->setGraphWindowColorOut(v);
            ctrl()->setIniEdited();
        }
    });
    m_graphAxisColor = createLabelColor(ini()->graphWindowAxisColor(), [&](const QColor &v) {
        if (ini()->graphWindowAxisColor() != v) {
            ini()->setGraphWindowAxisColor(v);
            ctrl()->setIniEdited();
        }
    });
    m_graphTickLabelColor =
            createLabelColor(ini()->graphWindowTickLabelColor(), [&](const QColor &v) {
                if (ini()->graphWindowTickLabelColor() != v) {
                    ini()->setGraphWindowTickLabelColor(v);
                    ctrl()->setIniEdited();
                }
            });
    m_graphLabelColor = createLabelColor(ini()->graphWindowLabelColor(), [&](const QColor &v) {
        if (ini()->graphWindowLabelColor() != v) {
            ini()->setGraphWindowLabelColor(v);
            ctrl()->setIniEdited();
        }
    });
    m_graphGridColor = createLabelColor(ini()->graphWindowGridColor(), [&](const QColor &v) {
        if (ini()->graphWindowGridColor() != v) {
            ini()->setGraphWindowGridColor(v);
            ctrl()->setIniEdited();
        }
    });
}
