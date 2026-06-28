#include "dynamic_product_context.h"

#include <QDebug>

DynamicProductContext::DynamicProductContext()
{
    resetCache();
}

void DynamicProductContext::setup(const DynamicProtocol::ChannelConfig &channelConfig,
                                  const QString &productCode,
                                  int seed,
                                  const QVector<DynamicTestItemMeta> &itemMetas)
{
    reset();

    m_channelConfig = channelConfig;
    m_productCode = productCode;
    m_seed = seed & 0xFF;
    m_itemMetas = itemMetas;
    m_startTime = currentTimeText();
    m_configured = true;
}

void DynamicProductContext::reset()
{
    m_configured = false;
    m_channelConfig = DynamicProtocol::ChannelConfig();
    m_productCode.clear();

    m_seed = 0;
    m_groupCnt = 0;
    m_detectProgress = 0;

    m_hasActiveGroup = false;
    m_activeGroupCnt = 0;

    m_finished = false;
    m_reported = false;
    m_finalDataMode = false;

    m_groupCheckReceived = false;
    m_groupCheckPassed = false;

    clearActiveGroupDebugFrames();

    m_startTime.clear();
    m_finishTime.clear();

    resetCache();
}

bool DynamicProductContext::handleFrame(quint32 canId,
                                        const QByteArray &data)
{
    if (!m_configured) {
        return false;
    }

    if (canId != m_channelConfig.rxId) {
        return false;
    }

    if (m_reported) {
        /*
         * 测试板在开始检测后会持续轮询发送数据。
         * 结果已上报后继续收到同 ID 数据，不再更新最终结果。
         */
        return true;
    }

    DynamicProtocol::ParsedFrame frame =
            DynamicProtocol::parseFrame(canId, data, m_seed);

    if (!frame.valid) {
        return true;
    }

    if (!frame.seedOk) {
        return true;
    }

    /*
     * 结果已经锁定后，不再接收后续轮询组数据。
     * 避免 DetectInProgressFlag=100 后，下一轮组的数据把最终结果覆盖。
     */
    if (m_finished) {
        return true;
    }

    if (DynamicProtocol::isNormalDataSeq(frame.seq)) {
        /*
         * 普通数据帧必须归属于当前轮询组。
         * 如果 groupCnt 变化，说明进入了新一轮轮询组，
         * 旧组未完成的数据直接丢弃，重新开始缓存新组。
         */
        if (!ensureActiveGroup(frame)) {
            return true;
        }

        recordActiveGroupFrame(frame, data, false);
        applyNormalFrame(frame);
        return true;
    }

    if (DynamicProtocol::isGroupCheckSeq(frame.seq)) {
        /*
         * 组校验帧只允许校验当前正在缓存的 groupCnt。
         * 如果 groupCnt 不一致，不能拿它去校验当前缓存，直接忽略。
         */
        if (!m_hasActiveGroup || frame.groupCnt != m_activeGroupCnt) {
            return true;
        }

        recordActiveGroupFrame(frame, data, true);
        applyGroupCheckFrame(frame);
        return true;
    }

    return true;
}

bool DynamicProductContext::isConfigured() const
{
    return m_configured;
}

bool DynamicProductContext::isFinished() const
{
    return m_finished;
}

bool DynamicProductContext::isReported() const
{
    return m_reported;
}

void DynamicProductContext::markReported()
{
    m_reported = true;
}

int DynamicProductContext::productId() const
{
    return m_channelConfig.productId;
}

QString DynamicProductContext::productCode() const
{
    return m_productCode;
}

quint32 DynamicProductContext::txId() const
{
    return m_channelConfig.txId;
}

quint32 DynamicProductContext::rxId() const
{
    return m_channelConfig.rxId;
}

int DynamicProductContext::seed() const
{
    return m_seed;
}

int DynamicProductContext::thresholdFlag() const
{
    return m_channelConfig.thresholdFlag;
}

int DynamicProductContext::groupCnt() const
{
    return m_groupCnt;
}

int DynamicProductContext::detectProgress() const
{
    return m_detectProgress;
}

int DynamicProductContext::receivedCount() const
{
    int count = 0;
    for (int i = 0; i <= 60; ++i) {
        if (m_received[i]) {
            ++count;
        }
    }
    return count;
}

int DynamicProductContext::checkOkCount() const
{
    int count = 0;
    for (int i = 0; i <= 60; ++i) {
        if (m_checkOk[i]) {
            ++count;
        }
    }
    return count;
}

DynamicProductResult DynamicProductContext::buildResult(const QString &message) const
{
    DynamicProductResult result;

    result.productId = m_channelConfig.productId;
    result.productCode = m_productCode;
    result.channelName = m_channelConfig.name;

    result.txId = m_channelConfig.txId;
    result.rxId = m_channelConfig.rxId;

    result.seed = m_seed;
    result.groupCnt = m_groupCnt;
    result.detectProgress = m_detectProgress;

    result.receivedCount = receivedCount();
    result.checkOkCount = checkOkCount();

    result.finished = m_finished;

    result.groupCheckReceived = m_groupCheckReceived;
    result.groupCheckPassed = m_groupCheckPassed;

    result.startTime = m_startTime;
    result.finishTime = m_finishTime.isEmpty() ? currentTimeText() : m_finishTime;

    for (const DynamicTestItemMeta &meta : m_itemMetas) {
        result.itemResults.append(buildItemResult(meta));
    }

    result.pass = calcProductPass(result.itemResults);

    /*
     * 最小化调试：
     * 产品 NG 时，输出：
     *   1. 不合格 item 对应的 flag 原始报文；
     *   2. 该报文解析后的结果；
     *   3. 当前界面显示结果所属轮询组的所有报文。
     */
    if (!result.pass) {
        debugDumpProductNg(result.itemResults);
    }

    if (!message.isEmpty()) {
        result.message = message;
    } else if (!result.finished) {
        result.message = "检测未完成";
    } else if (result.pass) {
        result.message = "动态检测合格";
    } else {
        result.message = "动态检测不合格";
    }

    return result;
}

void DynamicProductContext::resetCache()
{
    for (int i = 0; i <= 60; ++i) {
        m_values[i] = 0;
        m_received[i] = false;
        m_checkOk[i] = false;
    }

    /*
     * 协议：合格标志位默认 1，1 代表不合格。
     * 这里按 Excel 的 1~31 标志位初始化。
     * 如果现场确实存在 seq=0 标志位，也不会影响接收，收到后会覆盖。
     */
    for (int i = 1; i <= 31; ++i) {
        m_values[i] = 1;
    }
}

void DynamicProductContext::startNewGroup(int groupCnt)
{
    /*
     * 每一个 groupCnt 都是一组独立轮询数据。
     *
     * 关键原则：
     *   m_values / m_received / m_checkOk 只允许保存同一组 groupCnt 的数据。
     *   一旦 groupCnt 变化，旧组未完成数据全部清空，避免 flag 和 rec 混组。
     */
    resetCache();
    clearActiveGroupDebugFrames();

    m_hasActiveGroup = true;
    m_activeGroupCnt = groupCnt;

    m_groupCnt = groupCnt;
    m_detectProgress = m_finalDataMode ? DynamicProtocol::kDetectFinishedValue : 0;

    m_groupCheckReceived = false;
    m_groupCheckPassed = false;
}


void DynamicProductContext::lockActiveGroupAsFinished(const QString &reason)
{
    if (m_finished) {
        return;
    }

    m_finished = true;

    if (m_finishTime.isEmpty()) {
        m_finishTime = currentTimeText();
    }

    qDebug().noquote()
            << QString("[DynamicFinish] product=%1 channel=%2 activeGroup=%3 detectProgress=%4 reason=%5 groupCheckReceived=%6 groupCheckPassed=%7")
               .arg(m_productCode)
               .arg(m_channelConfig.name)
               .arg(m_activeGroupCnt)
               .arg(m_detectProgress)
               .arg(reason)
               .arg(m_groupCheckReceived ? "true" : "false")
               .arg(m_groupCheckPassed ? "true" : "false");
}

bool DynamicProductContext::ensureActiveGroup(const DynamicProtocol::ParsedFrame &frame)
{
    if (!m_hasActiveGroup) {
        startNewGroup(frame.groupCnt);
        return true;
    }

    if (frame.groupCnt != m_activeGroupCnt) {
        /*
         * 关键修正：
         *
         * 上一版要求 DetectInProgressFlag==100 后，还必须等 seq=255 组校验帧，
         * 但现场测试表现为进度已经到 100，流程仍不停止。
         *
         * 说明检测板可能没有发送组校验帧，或者当前工程没有收到/识别到组校验帧。
         *
         * 现在改成：
         *   如果上一组的 DetectInProgressFlag 已经到 100，
         *   当检测到下一组 groupCnt 开始时，说明上一组轮询已经结束，
         *   此时锁定上一组作为最终结果。
         *
         * 这样仍然保证最终显示的 item 数据来自同一个 groupCnt，
         * 并且不会把新组第一帧塞进上一组结果。
         */
        if (m_detectProgress == DynamicProtocol::kDetectFinishedValue &&
                hasRequiredFrames()) {
            lockActiveGroupAsFinished("DetectInProgressFlag=100 and next group started");
            return false;
        }

        /*
         * 上一组没有完成，丢弃上一组未完成缓存，重新开始新组。
         */
        startNewGroup(frame.groupCnt);
        return true;
    }

    return true;
}

void DynamicProductContext::applyNormalFrame(const DynamicProtocol::ParsedFrame &frame)
{
    if (!frame.checkOk) {
        if (frame.seq >= 0 && frame.seq <= 60) {
            m_checkOk[frame.seq] = false;
        }
        return;
    }

    m_groupCnt = frame.groupCnt;

    if (DynamicProtocol::isDetectInProgressSeq(frame.seq)) {
        /*
         * 这里只记录当前轮询组的进度值。
         * 不能在收到 DetectInProgressFlag==100 的瞬间就完成并上报，
         * 因为同一组后续 rec 数据可能还没接收完。
         *
         * 最终完成在 applyGroupCheckFrame() 中锁定：
         *   当前组 DetectInProgressFlag == 100
         *   并且当前组组校验通过
         */
        m_detectProgress = frame.value;
        if (frame.value == DynamicProtocol::kDetectFinishedValue) {
            m_finalDataMode = true;
        }

        m_values[frame.seq] = frame.value;
        m_received[frame.seq] = true;
        m_checkOk[frame.seq] = true;
        return;
    }

    if (!m_finalDataMode) {
        /*
         * Match the legacy dynamic tester: before DetectInProgressFlag reaches
         * 100, normal flag/record frames are treated as intermediate data and
         * are not cached into the final result group.
         */
        return;
    }

    if (frame.seq >= 0 && frame.seq <= 60) {
        m_values[frame.seq] = frame.value;
        m_received[frame.seq] = true;
        m_checkOk[frame.seq] = true;
    }
}

void DynamicProductContext::applyGroupCheckFrame(const DynamicProtocol::ParsedFrame &frame)
{
    m_groupCheckReceived = true;
    m_groupCnt = frame.groupCnt;

    const int calc = DynamicProtocol::calcGroupCheck(m_values,
                                                     m_seed,
                                                     frame.groupCnt);

    m_groupCheckPassed = (calc == frame.checkValue);

    /*
     * 只有当前同一轮询组同时满足：
     *   1. DetectInProgressFlag == 100
     *   2. 当前组组校验通过
     *
     * 才允许锁定结果。
     *
     * 这样可以保证：
     *   每个 item 的 flag 和 rec 来自同一个 groupCnt；
     *   不会把上一组 flag 和下一组 rec 拼到同一个 item；
     *   不会在进度刚到 100、但同组后续数据还没收齐时提前上报。
     */
    if (m_detectProgress == DynamicProtocol::kDetectFinishedValue &&
            m_groupCheckPassed &&
            hasRequiredFrames()) {
        lockActiveGroupAsFinished("DetectInProgressFlag=100 and group check passed");
    }
}

bool DynamicProductContext::hasRequiredFrames() const
{
    for (const DynamicTestItemMeta &meta : m_itemMetas) {
        if (meta.flagSeq < 0 || meta.flagSeq > 60 || !m_received[meta.flagSeq]) {
            return false;
        }

        if (meta.valueSeq >= 0 &&
                (meta.valueSeq > 60 || !m_received[meta.valueSeq])) {
            return false;
        }
    }

    return true;
}

DynamicItemResult DynamicProductContext::buildItemResult(const DynamicTestItemMeta &meta) const
{
    DynamicItemResult item;

    item.itemNo = meta.itemNo;
    item.itemName = meta.itemName;
    item.standardText = meta.standardText;
    item.unit = meta.unit;

    item.flagSeq = meta.flagSeq;
    item.valueSeq = meta.valueSeq;

    if (meta.flagSeq >= 0 && meta.flagSeq <= 60) {
        item.flagValue = m_values[meta.flagSeq];
        item.flagReceived = m_received[meta.flagSeq];
        item.flagCheckOk = m_checkOk[meta.flagSeq];
    }

    if (meta.valueSeq >= 0 && meta.valueSeq <= 60) {
        item.rawValue = m_values[meta.valueSeq];
        item.valueReceived = m_received[meta.valueSeq];
        item.valueCheckOk = m_checkOk[meta.valueSeq];
        item.displayValue = static_cast<double>(item.rawValue) * meta.scale;
    }

    if (meta.isProgressItem) {
        item.pass = item.flagReceived
                    && item.flagCheckOk
                    && item.flagValue == meta.progressFinishedValue;

        if (!item.flagReceived) {
            item.message = "进度数据未收到";
        } else if (!item.flagCheckOk) {
            item.message = "进度数据校验失败";
        } else if (item.flagValue != meta.progressFinishedValue) {
            item.message = QString("检测未完成，当前进度=%1").arg(item.flagValue);
        } else {
            item.message = "检测完成";
        }

        item.rawValue = item.flagValue;
        item.displayValue = item.flagValue;
        return item;
    }

    const bool flagOk = item.flagReceived
                        && item.flagCheckOk
                        && item.flagValue == 0;

    bool valueOk = true;

    if (meta.valueSeq >= 0) {
        valueOk = item.valueReceived && item.valueCheckOk;
    }

    item.pass = flagOk && valueOk;

    if (!item.flagReceived) {
        item.message = "合格标志未收到";
    } else if (!item.flagCheckOk) {
        item.message = "合格标志校验失败";
    } else if (item.flagValue != 0) {
        item.message = "检测项不合格";
    } else if (meta.valueSeq >= 0 && !item.valueReceived) {
        item.message = "检测值未收到";
    } else if (meta.valueSeq >= 0 && !item.valueCheckOk) {
        item.message = "检测值校验失败";
    } else {
        item.message = "OK";
    }

    return item;
}

bool DynamicProductContext::calcProductPass(const QVector<DynamicItemResult> &items) const
{
    if (!m_finished) {
        return false;
    }

    for (const DynamicTestItemMeta &meta : m_itemMetas) {
        if (!meta.affectsProductPass) {
            continue;
        }

        for (const DynamicItemResult &item : items) {
            if (item.itemNo == meta.itemNo) {
                if (!item.pass) {
                    return false;
                }
                break;
            }
        }
    }

    return true;
}


void DynamicProductContext::clearActiveGroupDebugFrames()
{
    m_activeGroupDebugFrames.clear();
}

void DynamicProductContext::recordActiveGroupFrame(const DynamicProtocol::ParsedFrame &frame,
                                                   const QByteArray &rawData,
                                                   bool groupCheckFrame)
{
    DebugFrameRecord record;
    record.frame = frame;
    record.rawData = rawData;
    record.groupCheckFrame = groupCheckFrame;

    m_activeGroupDebugFrames.append(record);
}

const DynamicProductContext::DebugFrameRecord *
DynamicProductContext::findLastDebugFrameBySeq(int seq) const
{
    for (int i = m_activeGroupDebugFrames.size() - 1; i >= 0; --i) {
        if (m_activeGroupDebugFrames.at(i).frame.seq == seq) {
            return &m_activeGroupDebugFrames.at(i);
        }
    }

    return nullptr;
}

QString DynamicProductContext::debugFrameLine(const DebugFrameRecord &record) const
{
    const DynamicProtocol::ParsedFrame &f = record.frame;

    return QString("type=%1 canId=%2 raw=[%3] parsed={seq=%4, seed=%5, value=%6, groupCnt=%7, checkValue=%8, valid=%9, seedOk=%10, checkOk=%11}")
            .arg(record.groupCheckFrame ? "GROUP_CHECK" : "NORMAL")
            .arg(DynamicProtocol::canIdToString(f.canId))
            .arg(DynamicProtocol::frameDataToString(record.rawData))
            .arg(f.seq)
            .arg(f.seed)
            .arg(f.value)
            .arg(f.groupCnt)
            .arg(f.checkValue)
            .arg(f.valid ? "true" : "false")
            .arg(f.seedOk ? "true" : "false")
            .arg(f.checkOk ? "true" : "false");
}

void DynamicProductContext::debugDumpNgItem(const DynamicItemResult &item) const
{
    qDebug().noquote()
            << QString("[DynamicNG][Item] product=%1 channel=%2 activeGroup=%3 itemNo=%4 itemName=%5 pass=%6 message=%7")
               .arg(m_productCode)
               .arg(m_channelConfig.name)
               .arg(m_activeGroupCnt)
               .arg(item.itemNo)
               .arg(item.itemName)
               .arg(item.pass ? "true" : "false")
               .arg(item.message);

    qDebug().noquote()
            << QString("[DynamicNG][Flag] item=%1 flagSeq=%2 flagValue=%3 flagReceived=%4 flagCheckOk=%5")
               .arg(item.itemName)
               .arg(item.flagSeq)
               .arg(item.flagValue)
               .arg(item.flagReceived ? "true" : "false")
               .arg(item.flagCheckOk ? "true" : "false");

    const DebugFrameRecord *flagRecord = findLastDebugFrameBySeq(item.flagSeq);

    if (flagRecord) {
        qDebug().noquote()
                << QString("[DynamicNG][FlagRaw] item=%1 %2")
                   .arg(item.itemName)
                   .arg(debugFrameLine(*flagRecord));
    } else {
        qDebug().noquote()
                << QString("[DynamicNG][FlagRaw] item=%1 flagSeq=%2 当前轮询组未找到对应原始报文")
                   .arg(item.itemName)
                   .arg(item.flagSeq);
    }

    if (item.valueSeq >= 0) {
        qDebug().noquote()
                << QString("[DynamicNG][Value] item=%1 valueSeq=%2 rawValue=%3 displayValue=%4 valueReceived=%5 valueCheckOk=%6 unit=%7 standard=%8")
                   .arg(item.itemName)
                   .arg(item.valueSeq)
                   .arg(item.rawValue)
                   .arg(item.displayValue)
                   .arg(item.valueReceived ? "true" : "false")
                   .arg(item.valueCheckOk ? "true" : "false")
                   .arg(item.unit)
                   .arg(item.standardText);

        const DebugFrameRecord *valueRecord = findLastDebugFrameBySeq(item.valueSeq);

        if (valueRecord) {
            qDebug().noquote()
                    << QString("[DynamicNG][ValueRaw] item=%1 %2")
                       .arg(item.itemName)
                       .arg(debugFrameLine(*valueRecord));
        } else {
            qDebug().noquote()
                    << QString("[DynamicNG][ValueRaw] item=%1 valueSeq=%2 当前轮询组未找到对应原始报文")
                       .arg(item.itemName)
                       .arg(item.valueSeq);
        }
    }
}

void DynamicProductContext::debugDumpCurrentGroupFrames(const QString &reason) const
{
    qDebug().noquote()
            << QString("[DynamicNG][GroupDumpBegin] reason=%1 product=%2 channel=%3 activeGroup=%4 frameCount=%5 detectProgress=%6 groupCheckReceived=%7 groupCheckPassed=%8")
               .arg(reason)
               .arg(m_productCode)
               .arg(m_channelConfig.name)
               .arg(m_activeGroupCnt)
               .arg(m_activeGroupDebugFrames.size())
               .arg(m_detectProgress)
               .arg(m_groupCheckReceived ? "true" : "false")
               .arg(m_groupCheckPassed ? "true" : "false");

    for (int i = 0; i < m_activeGroupDebugFrames.size(); ++i) {
        qDebug().noquote()
                << QString("[DynamicNG][GroupFrame][%1] %2")
                   .arg(i)
                   .arg(debugFrameLine(m_activeGroupDebugFrames.at(i)));
    }

    qDebug().noquote()
            << QString("[DynamicNG][GroupDumpEnd] product=%1 activeGroup=%2")
               .arg(m_productCode)
               .arg(m_activeGroupCnt);
}

void DynamicProductContext::debugDumpProductNg(const QVector<DynamicItemResult> &items) const
{
    qDebug().noquote()
            << QString("[DynamicNG][Product] product=%1 channel=%2 finished=%3 detectProgress=%4 activeGroup=%5 groupCheckReceived=%6 groupCheckPassed=%7")
               .arg(m_productCode)
               .arg(m_channelConfig.name)
               .arg(m_finished ? "true" : "false")
               .arg(m_detectProgress)
               .arg(m_activeGroupCnt)
               .arg(m_groupCheckReceived ? "true" : "false")
               .arg(m_groupCheckPassed ? "true" : "false");

    bool dumpedAnyItem = false;

    for (const DynamicItemResult &item : items) {
        if (!item.pass) {
            dumpedAnyItem = true;
            debugDumpNgItem(item);
        }
    }

    if (!dumpedAnyItem) {
        qDebug().noquote()
                << QString("[DynamicNG][Product] 未发现 item.pass=false，但产品总结果为 NG。请检查 m_finished 或 calcProductPass 逻辑。");
    }

    debugDumpCurrentGroupFrames("product NG");
}


QString DynamicProductContext::currentTimeText() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
}
