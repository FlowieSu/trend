#ifndef DYNAMIC_TEST_CONTROLLER_H
#define DYNAMIC_TEST_CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QVector>

#include "zlg_can_device.h"

#include "dynamic_protocol.h"
#include "dynamic_product_context.h"
#include "dynamic_result_types.h"
#include "dynamic_test_item_config.h"

/*
 * 动态检测控制器。
 *
 * 职责：
 * 1. 管理 ZlgCanDevice；
 * 2. 发送复位帧和开始检测帧；
 * 3. 接收 CAN 帧并分发给两个 DynamicProductContext；
 * 4. 在 DetectInProgressFlag == 100 时输出产品结果；
 * 5. 两个产品完成后输出总结果。
 */

class DynamicTestController : public QObject
{
    Q_OBJECT

public:
    explicit DynamicTestController(QObject *parent = nullptr);
    ~DynamicTestController() override;

    bool isCanOpen() const;
    bool isRunning() const;

    void setTimeoutMs(int timeoutMs);
    int timeoutMs() const;

    void setItemMetas(const QVector<DynamicTestItemMeta> &metas);

public slots:
    bool openCanDevice(const ZlgCanOpenConfig &config);
    void closeCanDevice();

    /*
     * 默认使用 0x110/0x112 两个通道。
     */
    void startDynamicTest(const QString &productCode1,
                          const QString &productCode2);

    void startDynamicTestWithChannels(const QString &productCode1,
                                      DynamicProtocol::DynamicChannel channel1,
                                      const QString &productCode2,
                                      DynamicProtocol::DynamicChannel channel2);

    /*
     * 新增重载：用于“动态检测参数配置”窗口传入 thresholdFlag。
     * 不改变原动态检测流程，只覆盖开始检测帧 Byte3~Byte4。
     */
    void startDynamicTestWithChannels(const QString &productCode1,
                                      DynamicProtocol::DynamicChannel channel1,
                                      int thresholdFlag1,
                                      const QString &productCode2,
                                      DynamicProtocol::DynamicChannel channel2,
                                      int thresholdFlag2);

    void requestStop();

signals:
    void logMessage(const QString &message);

    void canConnectedChanged(bool connected);

    void productProgressChanged(int productId,
                                int progress,
                                int groupCnt);

    void productFrameUpdated(int productId,
                             int seq,
                             int value,
                             int groupCnt);

    void productFinished(DynamicProductResult result);

    void allFinished(bool allPass,
                     QVector<DynamicProductResult> results);

private slots:
    void onCanConnectedChanged(bool connected);

    void onFrameReceived(quint32 canId,
                         QByteArray data);

    void onSendFinished(quint32 canId,
                        bool ok,
                        QString message);

    void onTimeout();

private:
    void ensureCanDevice();

    void resetTestState();

    int nextSeed();

    bool prepareProduct(DynamicProductContext &context,
                        const QString &productCode,
                        DynamicProtocol::DynamicChannel channel);

    bool prepareProduct(DynamicProductContext &context,
                        const QString &productCode,
                        DynamicProtocol::DynamicChannel channel,
                        int thresholdFlag);

    void sendResetFrames();

    void sendStartFrames();

    void checkProductFinished(DynamicProductContext &context);

    void finishIfAllDone();

    void emitAbortResults(const QString &message);

    QString canIdText(quint32 canId) const;

private:
    ZlgCanDevice *m_canDevice = nullptr;

    bool m_canOpen = false;
    bool m_running = false;
    bool m_acceptFrames = false;

    int m_timeoutMs = 180000;
    int m_nextSeed = 2;

    QTimer *m_timeoutTimer = nullptr;

    QVector<DynamicTestItemMeta> m_itemMetas;

    DynamicProductContext m_product1;
    DynamicProductContext m_product2;

    QVector<DynamicProductResult> m_finishedResults;
};

#endif // DYNAMIC_TEST_CONTROLLER_H
