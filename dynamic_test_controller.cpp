#include "dynamic_test_controller.h"

#include <QMetaType>
#include <QTimer>

DynamicTestController::DynamicTestController(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DynamicItemResult>("DynamicItemResult");
    qRegisterMetaType<DynamicProductResult>("DynamicProductResult");
    qRegisterMetaType<QVector<DynamicProductResult>>("QVector<DynamicProductResult>");

    qRegisterMetaType<DynamicProtocol::DynamicChannel>("DynamicProtocol::DynamicChannel");

    m_itemMetas = createDefaultDynamicTestItemMetas();

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);

    connect(m_timeoutTimer,
            &QTimer::timeout,
            this,
            &DynamicTestController::onTimeout);
}

DynamicTestController::~DynamicTestController()
{
    closeCanDevice();
}

bool DynamicTestController::isCanOpen() const
{
    return m_canOpen;
}

bool DynamicTestController::isRunning() const
{
    return m_running;
}

void DynamicTestController::setTimeoutMs(int timeoutMs)
{
    if (timeoutMs > 0) {
        m_timeoutMs = timeoutMs;
    }
}

int DynamicTestController::timeoutMs() const
{
    return m_timeoutMs;
}

void DynamicTestController::setItemMetas(const QVector<DynamicTestItemMeta> &metas)
{
    if (!metas.isEmpty()) {
        m_itemMetas = metas;
    }
}

bool DynamicTestController::openCanDevice(const ZlgCanOpenConfig &config)
{
    if (m_running) {
        emit logMessage("动态检测运行中，不能重新打开 CAN 设备。");
        return false;
    }

    ensureCanDevice();

    const bool ok = m_canDevice->openDevice(config);
    m_canOpen = ok;

    return ok;
}

void DynamicTestController::closeCanDevice()
{
    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
    }

    if (m_canDevice) {
        m_canDevice->closeDevice();
        m_canDevice->deleteLater();
        m_canDevice = nullptr;
    }

    m_canOpen = false;
    m_running = false;
    m_acceptFrames = false;
}

void DynamicTestController::startDynamicTest(const QString &productCode1,
                                             const QString &productCode2)
{
    startDynamicTestWithChannels(productCode1,
                                 DynamicProtocol::DynamicChannel::Channel110,
                                 productCode2,
                                 DynamicProtocol::DynamicChannel::Channel112);
}

void DynamicTestController::startDynamicTestWithChannels(const QString &productCode1,
                                                         DynamicProtocol::DynamicChannel channel1,
                                                         const QString &productCode2,
                                                         DynamicProtocol::DynamicChannel channel2)
{
    const DynamicProtocol::ChannelConfig cfg1 =
            DynamicProtocol::channelConfig(channel1);

    const DynamicProtocol::ChannelConfig cfg2 =
            DynamicProtocol::channelConfig(channel2);

    startDynamicTestWithChannels(productCode1,
                                 channel1,
                                 cfg1.thresholdFlag,
                                 productCode2,
                                 channel2,
                                 cfg2.thresholdFlag);
}

void DynamicTestController::startDynamicTestWithChannels(const QString &productCode1,
                                                         DynamicProtocol::DynamicChannel channel1,
                                                         int thresholdFlag1,
                                                         const QString &productCode2,
                                                         DynamicProtocol::DynamicChannel channel2,
                                                         int thresholdFlag2)
{
    if (!m_canDevice || !m_canDevice->isOpen()) {
        emit logMessage("CAN 设备未连接，禁止启动动态检测。");
        return;
    }

    if (m_running) {
        emit logMessage("动态检测正在运行，禁止重复启动。");
        return;
    }

    if (productCode1.trimmed().isEmpty() || productCode2.trimmed().isEmpty()) {
        emit logMessage("产品编码为空，禁止启动动态检测。");
        return;
    }

    if (channel1 == channel2) {
        emit logMessage("位置1和位置2动态发送通道重复，禁止启动动态检测。");
        return;
    }

    resetTestState();

    if (!prepareProduct(m_product1, productCode1.trimmed(), channel1, thresholdFlag1)) {
        emit logMessage("位置1动态检测上下文初始化失败。");
        return;
    }

    if (!prepareProduct(m_product2, productCode2.trimmed(), channel2, thresholdFlag2)) {
        emit logMessage("位置2动态检测上下文初始化失败。");
        return;
    }

    m_running = true;
    m_acceptFrames = false;

    emit logMessage(QString("动态检测开始：位置1=%1，通道=%2，threshold=%3；位置2=%4，通道=%5，threshold=%6。")
                    .arg(productCode1.trimmed())
                    .arg(DynamicProtocol::channelName(channel1))
                    .arg(thresholdFlag1)
                    .arg(productCode2.trimmed())
                    .arg(DynamicProtocol::channelName(channel2))
                    .arg(thresholdFlag2));

    if (m_canDevice && !m_canDevice->clearBuffer()) {
        emit logMessage("动态检测开始前清空 CAN 接收缓存失败，继续执行。");
    }

    sendResetFrames();

    /*
     * 复位帧之后稍等再发开始检测帧。
     * 不改变原流程，只把开始检测帧中的 thresholdFlag 改为外部配置。
     */
    QTimer::singleShot(100, this, [this]() {
        if (!m_running) {
            return;
        }

        if (m_canDevice && !m_canDevice->clearBuffer()) {
            emit logMessage("发送开始检测帧前清空 CAN 接收缓存失败，继续执行。");
        }

        m_acceptFrames = true;
        sendStartFrames();

        if (m_timeoutTimer) {
            m_timeoutTimer->start(m_timeoutMs);
        }
    });
}

void DynamicTestController::requestStop()
{
    if (!m_running) {
        return;
    }

    emit logMessage("动态检测被请求停止。");

    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
    }

    emitAbortResults("动态检测被手动停止");
}

void DynamicTestController::onCanConnectedChanged(bool connected)
{
    m_canOpen = connected;
    emit canConnectedChanged(connected);
}

void DynamicTestController::onFrameReceived(quint32 canId,
                                            QByteArray data)
{
    if (!m_running) {
        return;
    }

    if (!m_acceptFrames) {
        return;
    }

    bool handled = false;

    if (m_product1.handleFrame(canId, data)) {
        handled = true;

        emit productProgressChanged(m_product1.productId(),
                                    m_product1.detectProgress(),
                                    m_product1.groupCnt());

        if (data.size() >= 6) {
            emit productFrameUpdated(m_product1.productId(),
                                     static_cast<unsigned char>(data[0]),
                                     static_cast<qint16>(
                                         (static_cast<unsigned char>(data[3]) << 8)
                                         | static_cast<unsigned char>(data[2])),
                                     (static_cast<unsigned char>(data[5]) << 8)
                                     | static_cast<unsigned char>(data[4]));
        }

        checkProductFinished(m_product1);
    }

    if (m_product2.handleFrame(canId, data)) {
        handled = true;

        emit productProgressChanged(m_product2.productId(),
                                    m_product2.detectProgress(),
                                    m_product2.groupCnt());

        if (data.size() >= 6) {
            emit productFrameUpdated(m_product2.productId(),
                                     static_cast<unsigned char>(data[0]),
                                     static_cast<qint16>(
                                         (static_cast<unsigned char>(data[3]) << 8)
                                         | static_cast<unsigned char>(data[2])),
                                     (static_cast<unsigned char>(data[5]) << 8)
                                     | static_cast<unsigned char>(data[4]));
        }

        checkProductFinished(m_product2);
    }

    if (!handled) {
        /*
         * 非动态检测 ID，不打印日志，避免 CAN 总线上其它帧刷屏。
         */
        return;
    }

    finishIfAllDone();
}

void DynamicTestController::onSendFinished(quint32 canId,
                                           bool ok,
                                           QString message)
{
    emit logMessage(QString("发送帧 %1：%2，%3")
                    .arg(canIdText(canId))
                    .arg(ok ? "成功" : "失败")
                    .arg(message));
}

void DynamicTestController::onTimeout()
{
    if (!m_running) {
        return;
    }

    emit logMessage("动态检测超时。");
    emitAbortResults("动态检测超时");
}

void DynamicTestController::ensureCanDevice()
{
    if (m_canDevice) {
        return;
    }

    m_canDevice = new ZlgCanDevice(this);

    connect(m_canDevice,
            &ZlgCanDevice::connectedChanged,
            this,
            &DynamicTestController::onCanConnectedChanged);

    connect(m_canDevice,
            &ZlgCanDevice::frameReceived,
            this,
            &DynamicTestController::onFrameReceived);

    connect(m_canDevice,
            &ZlgCanDevice::sendFinished,
            this,
            &DynamicTestController::onSendFinished);

    connect(m_canDevice,
            &ZlgCanDevice::logMessage,
            this,
            &DynamicTestController::logMessage);
}

void DynamicTestController::resetTestState()
{
    m_finishedResults.clear();
    m_acceptFrames = false;

    m_product1.reset();
    m_product2.reset();

    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
    }
}

int DynamicTestController::nextSeed()
{
    int seed = m_nextSeed & 0xFF;

    if (seed < 2) {
        seed = 2;
    }

    ++m_nextSeed;

    if (m_nextSeed > 255) {
        m_nextSeed = 2;
    }

    return seed;
}

bool DynamicTestController::prepareProduct(DynamicProductContext &context,
                                           const QString &productCode,
                                           DynamicProtocol::DynamicChannel channel)
{
    const DynamicProtocol::ChannelConfig cfg =
            DynamicProtocol::channelConfig(channel);

    return prepareProduct(context,
                          productCode,
                          channel,
                          cfg.thresholdFlag);
}

bool DynamicTestController::prepareProduct(DynamicProductContext &context,
                                           const QString &productCode,
                                           DynamicProtocol::DynamicChannel channel,
                                           int thresholdFlag)
{
    DynamicProtocol::ChannelConfig cfg =
            DynamicProtocol::channelConfig(channel);

    if (cfg.txId == 0 || cfg.rxId == 0) {
        return false;
    }

    cfg.thresholdFlag = thresholdFlag;

    context.setup(cfg,
                  productCode,
                  nextSeed(),
                  m_itemMetas);

    return true;
}

void DynamicTestController::sendResetFrames()
{
    if (!m_canDevice) {
        return;
    }

    const QByteArray resetFrame = DynamicProtocol::buildResetFrame();

    m_canDevice->sendFrame(m_product1.txId(), resetFrame);
    m_canDevice->sendFrame(m_product2.txId(), resetFrame);

    emit logMessage(QString("已发送复位帧：%1、%2。")
                    .arg(canIdText(m_product1.txId()))
                    .arg(canIdText(m_product2.txId())));
}

void DynamicTestController::sendStartFrames()
{
    if (!m_canDevice) {
        return;
    }

    const QByteArray start1 =
            DynamicProtocol::buildStartFrame(m_product1.thresholdFlag(),
                                             m_product1.seed());

    const QByteArray start2 =
            DynamicProtocol::buildStartFrame(m_product2.thresholdFlag(),
                                             m_product2.seed());

    m_canDevice->sendFrame(m_product1.txId(), start1);
    m_canDevice->sendFrame(m_product2.txId(), start2);

    emit logMessage(QString("已发送开始检测帧：%1 DATA=%2；%3 DATA=%4。")
                    .arg(canIdText(m_product1.txId()))
                    .arg(DynamicProtocol::frameDataToString(start1))
                    .arg(canIdText(m_product2.txId()))
                    .arg(DynamicProtocol::frameDataToString(start2)));
}

void DynamicTestController::checkProductFinished(DynamicProductContext &context)
{
    if (!context.isFinished() || context.isReported()) {
        return;
    }

    DynamicProductResult result = context.buildResult();

    context.markReported();

    m_finishedResults.append(result);

    emit logMessage(QString("位置%1动态检测完成：%2，进度=%3，总结果=%4。")
                    .arg(result.productId)
                    .arg(result.productCode)
                    .arg(result.detectProgress)
                    .arg(result.pass ? "PASS" : "NG"));

    emit productFinished(result);
}

void DynamicTestController::finishIfAllDone()
{
    if (!m_running) {
        return;
    }

    if (!m_product1.isFinished() || !m_product2.isFinished()) {
        return;
    }

    if (!m_product1.isReported() || !m_product2.isReported()) {
        return;
    }

    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
    }

    bool allPass = true;

    for (const DynamicProductResult &result : m_finishedResults) {
        if (!result.pass) {
            allPass = false;
            break;
        }
    }

    m_running = false;
    m_acceptFrames = false;

    emit logMessage(QString("双产品动态检测完成，总结果=%1。")
                    .arg(allPass ? "PASS" : "NG"));

    emit allFinished(allPass, m_finishedResults);
}

void DynamicTestController::emitAbortResults(const QString &message)
{
    QVector<DynamicProductResult> results;

    if (m_product1.isConfigured()) {
        results.append(m_product1.buildResult(message));
    }

    if (m_product2.isConfigured()) {
        results.append(m_product2.buildResult(message));
    }

    m_running = false;
    m_acceptFrames = false;

    emit allFinished(false, results);
}

QString DynamicTestController::canIdText(quint32 canId) const
{
    return DynamicProtocol::canIdToString(canId);
}
