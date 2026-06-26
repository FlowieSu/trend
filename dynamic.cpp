bool DynamicProductContext::calcProductPass(const QVector<DynamicItemResult> &items) const
{
    qDebug() << "[DynamicPass] productCode =" << m_productCode
             << "finished =" << m_finished
             << "detectProgress =" << m_detectProgress;

    if (!m_finished) {
        qDebug() << "[DynamicPass] Product NG because detect not finished.";
        return false;
    }

    bool productPass = true;

    for (const DynamicTestItemMeta &meta : m_itemMetas) {
        if (!meta.affectsProductPass) {
            continue;
        }

        bool found = false;

        for (const DynamicItemResult &item : items) {
            if (item.itemNo == meta.itemNo) {
                found = true;

                qDebug() << "[DynamicPass]"
                         << "item =" << item.itemName
                         << "pass =" << item.pass
                         << "flagValue =" << item.flagValue
                         << "flagReceived =" << item.flagReceived
                         << "flagCheckOk =" << item.flagCheckOk
                         << "valueReceived =" << item.valueReceived
                         << "valueCheckOk =" << item.valueCheckOk
                         << "message =" << item.message;

                if (!item.pass) {
                    productPass = false;
                }

                break;
            }
        }

        if (!found) {
            qDebug() << "[DynamicPass] item not found, itemNo =" << meta.itemNo
                     << "itemName =" << meta.itemName;
            productPass = false;
        }
    }

    qDebug() << "[DynamicPass] final productPass =" << productPass;

    return productPass;
}
