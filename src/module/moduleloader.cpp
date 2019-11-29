#include "moduleloader.h"
#include <QDebug>

namespace Stardust {

ModuleLoader::ModuleLoader(Paths *paths, QQmlEngine *engine) : QObject(nullptr) {
    this->paths = paths;
    this->qmlEngine = engine;

    systemConfigReader.setSource("/etc/stardust.json.conf");

    getModuleList();
    loadAllModules();
}

void ModuleLoader::loadModuleList() {
    QByteArray moduleListJsonBytes = systemConfigReader.read().toLocal8Bit();
    QJsonDocument moduleListJsonDocument = QJsonDocument::fromJson(moduleListJsonBytes);

    moduleJSON = QJsonObject(moduleListJsonDocument.object()["modules"].toObject());
    modulesFolder.setPath(moduleListJsonDocument.object()["modules_path"].toString());
}

void ModuleLoader::loadAllModules() {
    QStringList moduleIDs = moduleJSON.keys();

    foreach(QString moduleID, moduleIDs) {
        loadModule(moduleID);
    }
}

void ModuleLoader::loadModule(QString id) {
    if(!moduleJSON.contains(id))
        qErrnoWarning(("Module "+id+" does not exist.").toStdString().c_str());

    Module *moduleToLoad = getModuleById(id);

    if(moduleToLoad != nullptr) {
        moduleList.removeAll(moduleToLoad);
        delete moduleToLoad;
    }

    QString modulesFolderPath = modulesFolder.absolutePath();

    modulesFolder.cd(moduleJSON[id].toObject()["path"].toString());
    moduleToLoad = new Module(this, modulesFolder.absolutePath());

    modulesFolder.setPath(modulesFolderPath);

    moduleToLoad->reloadModuleInfo();
    moduleList.push_back(moduleToLoad);
}

Module *ModuleLoader::getModuleById(QString id) {
    foreach(Module *module, moduleList)
        if(module->property("id").toString() == id)
            return module;

    return nullptr;
}

QVariantMap ModuleLoader::getModuleList() {
    return moduleJSON.toVariantMap();
}

}
