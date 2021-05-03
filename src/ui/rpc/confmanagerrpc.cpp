#include "confmanagerrpc.h"

#include <sqlite/sqlitedb.h>

#include "../conf/firewallconf.h"
#include "../fortmanager.h"
#include "../rpc/rpcmanager.h"

ConfManagerRpc::ConfManagerRpc(const QString &filePath, FortManager *fortManager, QObject *parent) :
    ConfManager(filePath, fortManager, parent, SqliteDb::OpenDefaultReadOnly)
{
}

RpcManager *ConfManagerRpc::rpcManager() const
{
    return fortManager()->rpcManager();
}

void ConfManagerRpc::onConfChanged(int confVersion, bool onlyFlags)
{
    if (this->confVersion() == confVersion)
        return;

    if (onlyFlags) {
        loadFlags(*conf());
    } else {
        FirewallConf *conf = createConf();
        if (!load(*conf)) {
            delete conf;
            return;
        }
        setConf(conf);
    }

    setConfVersion(confVersion);

    emit confChanged(onlyFlags);

    fortManager()->reloadOptionsWindow(tr("Settings changed by someone else"));
}

void ConfManagerRpc::setupAppEndTimer() { }

bool ConfManagerRpc::saveToDbIni(FirewallConf &newConf, bool onlyFlags)
{
    rpcManager()->invokeOnServer(Control::Rpc_ConfManager_save,
            { newConf.toVariant(onlyFlags), confVersion(), onlyFlags });

    if (!rpcManager()->waitResult()) {
        showErrorMessage("Save Conf: Service isn't responding.");
        return false;
    }

    if (rpcManager()->resultCommand() != Control::Rpc_Result_Ok) {
        showErrorMessage("Save Conf: Service error.");
        return false;
    }

    return true;
}
