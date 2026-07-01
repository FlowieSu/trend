qDebug() << "====== Static INI Debug ======";
qDebug() << "file =" << settings.fileName();
qDebug() << "group =" << groupName;
qDebug() << "childKeys =" << settings.childKeys();

const QString redRaw = settings.value("redBlackMPoints").toString();
const QString blackRaw = settings.value("blackRedMPoints").toString();

qDebug() << "name raw =" << settings.value("name").toString();
qDebug() << "redBlackMPoints raw =" << redRaw;
qDebug() << "blackRedMPoints raw =" << blackRaw;
qDebug() << "==============================";
