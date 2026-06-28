#ifndef DYNAMIC_PRODUCT_CONTEXT_H
#define DYNAMIC_PRODUCT_CONTEXT_H

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QByteArray>

#include "dynamic_protocol.h"
#include "dynamic_result_types.h"
#include "dynamic_test_item_config.h"

/*
 * 单个产品的动态检测数据上下文。
 *
 * 一个 DynamicProductContext 对应一个产品：
 *   例如 txId=0x110，rxId=0x201；
 *   或 txId=0x112，rxId=0x301。
 *
 * 它只负责：
 * 1. 缓存 seq 0~60 数据；
 * 2. 做 seed 和单帧校验；
 * 3. 根据 DetectInProgressFlag == 100 判断检测完成；
 * 4. 生成 DynamicProductResult。
 */

class DynamicProductContext
{
public:
    DynamicProductContext();

    void setup(const DynamicProtocol::ChannelConfig &channelConfig,
               const QString &productCode,
               int seed,
               const QVector<DynamicTestItemMeta> &itemMetas);

    void reset();

    bool handleFrame(quint32 canId,
                     const QByteArray &data);

    bool isConfigured() const;
    bool isFinished() const;
    bool isReported() const;
    void markReported();

    int productId() const;
    QString productCode() const;

    quint32 txId() const;
    quint32 rxId() const;

    int seed() const;
    int thresholdFlag() const;

    int groupCnt() const;
    int detectProgress() const;

    int receivedCount() const;
    int checkOkCount() const;

    DynamicProductResult buildResult(const QString &message = QString()) const;

private:
    void resetCache();

    /*
     * 新增：
     * 按 groupCnt 管理一整组轮询数据。
     * 只有同一个 groupCnt 的 flag 和 rec 才允许组成最终结果。
     */
    void startNewGroup(int groupCnt);

    /*
     * 新增：
     * 当上一组 DetectInProgressFlag 已经到 100，
     * 且检测到下一组 groupCnt 开始时，锁定上一组作为最终结果。
     *
     * 这样不依赖 seq=255 组校验帧。
     */
    void lockActiveGroupAsFinished(const QString &reason);

    bool ensureActiveGroup(const DynamicProtocol::ParsedFrame &frame);

    void applyNormalFrame(const DynamicProtocol::ParsedFrame &frame);
    void applyGroupCheckFrame(const DynamicProtocol::ParsedFrame &frame);
    bool hasRequiredFrames() const;

    DynamicItemResult buildItemResult(const DynamicTestItemMeta &meta) const;
    bool calcProductPass(const QVector<DynamicItemResult> &items) const;

    QString currentTimeText() const;


private:
    struct DebugFrameRecord
    {
        DynamicProtocol::ParsedFrame frame;
        QByteArray rawData;
        bool groupCheckFrame = false;
    };

    void clearActiveGroupDebugFrames();

    void recordActiveGroupFrame(const DynamicProtocol::ParsedFrame &frame,
                                const QByteArray &rawData,
                                bool groupCheckFrame);

    const DebugFrameRecord *findLastDebugFrameBySeq(int seq) const;

    void debugDumpProductNg(const QVector<DynamicItemResult> &items) const;
    void debugDumpNgItem(const DynamicItemResult &item) const;
    void debugDumpCurrentGroupFrames(const QString &reason) const;

    QString debugFrameLine(const DebugFrameRecord &record) const;

private:
    bool m_configured = false;

    DynamicProtocol::ChannelConfig m_channelConfig;

    QString m_productCode;

    int m_seed = 0;
    int m_groupCnt = 0;
    int m_detectProgress = 0;

    /*
     * 新增：
     * 当前正在缓存的轮询组。
     * m_values / m_received / m_checkOk 只代表这一组的数据。
     */
    bool m_hasActiveGroup = false;
    int m_activeGroupCnt = 0;

    bool m_finished = false;
    bool m_reported = false;
    bool m_finalDataMode = false;

    bool m_groupCheckReceived = false;
    bool m_groupCheckPassed = false;

    QString m_startTime;
    QString m_finishTime;

    int m_values[61];
    bool m_received[61];
    bool m_checkOk[61];

    
    QVector<DynamicTestItemMeta> m_itemMetas;

    /*
     * 调试用：
     * 保存当前 active groupCnt 对应的所有原始报文和解析结果。
     * 当产品 NG 时，qDebug 输出这些报文，便于确认界面显示的 item 是否混组。
     */
    QVector<DebugFrameRecord> m_activeGroupDebugFrames;
};


#endif // DYNAMIC_PRODUCT_CONTEXT_H
