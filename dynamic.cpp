static QString settingsValueToText(const QVariant &value)
{
    if (!value.isValid() || value.isNull()) {
        return QString();
    }

    QString text = value.toString().trimmed();

    if (!text.isEmpty()) {
        return text;
    }

    const QStringList list = value.toStringList();

    if (!list.isEmpty()) {
        return list.join(",");
    }

    return QString();
}
