import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "../../controls"
import com.fortfirewall 1.0

ButtonPopup {

    icon.source: !((appGroup.limitInEnabled && appGroup.speedLimitIn)
                   || (appGroup.limitOutEnabled && appGroup.speedLimitOut))
                 ? "qrc:/images/flag_green.png"
                 : "qrc:/images/flag_yellow.png"
    text: (translationManager.trTrigger
          && qsTranslate("qml", "Speed Limit: "))
          + speedLimitsText

    readonly property var speedLimitValues: [
        10, 0, 20, 30, 50, 75, 100, 150, 200, 300, 500, 900,
        1024, 1.5 * 1024, 2 * 1024, 3 * 1024, 5 * 1024, 7.5 * 1024,
        10 * 1024, 15 * 1024, 20 * 1024, 30 * 1024, 50 * 1024
    ]

    readonly property var speedLimitNames: {
        var list = translationManager.trTrigger
                && [qsTranslate("qml", "Custom"),
                    qsTranslate("qml", "Disabled")];

        const n = speedLimitValues.length;
        for (var i = list.length; i < n; ++i) {
            list.push(formatSpeed(speedLimitValues[i]));
        }
        return list;
    }

    readonly property var speedLimitDisabledText: speedLimitNames[1]

    readonly property string speedLimitsText: {
        const limitIn = appGroup.limitInEnabled
                ? appGroup.speedLimitIn : 0;
        const limitOut = appGroup.limitOutEnabled
                ? appGroup.speedLimitOut : 0;

        if (!(limitIn || limitOut))
            return speedLimitDisabledText;

        var text = "";
        if (limitIn) {
            text = "DL " + formatSpeed(limitIn);
        }
        if (limitOut) {
            text += (text ? "; " : "")
                    + "UL " + formatSpeed(limitOut);
        }
        return text;
    }

    function formatSpeed(kbytes) {
        return netUtil.formatSpeed(kbytes * 1024);
    }

    ColumnLayout {
        SpinComboRow {
            names: speedLimitNames
            values: speedLimitValues
            checkBox {
                text: translationManager.trTrigger
                      && qsTranslate("qml", "Download speed limit, KiB/s:")
                checked: appGroup.limitInEnabled
                onToggled: {
                    appGroup.limitInEnabled = checkBox.checked;

                    setConfEdited();
                }
            }
            field {
                from: 0
                to: 99999
                defaultValue: appGroup.speedLimitIn
                onValueEdited: {
                    appGroup.speedLimitIn = field.value;

                    setConfEdited();
                }
            }
        }

        SpinComboRow {
            names: speedLimitNames
            values: speedLimitValues
            checkBox {
                text: translationManager.trTrigger
                      && qsTranslate("qml", "Upload speed limit, KiB/s:")
                checked: appGroup.limitOutEnabled
                onToggled: {
                    appGroup.limitOutEnabled = checkBox.checked;

                    setConfEdited();
                }
            }
            field {
                from: 0
                to: 99999
                defaultValue: appGroup.speedLimitOut
                onValueEdited: {
                    appGroup.speedLimitOut = field.value;

                    setConfEdited();
                }
            }
        }
    }
}
