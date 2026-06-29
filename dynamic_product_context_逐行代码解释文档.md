# DynamicProductContext 逐行代码解释文档

> 说明：本文基于你上传的 `dynamic_product_context.h` 和 `dynamic_product_context.cpp` 生成。  
> 按你的要求，`qDebug` 调试输出内容和调试辅助函数不展开逐行解释，只保留与业务流程有关的代码解释。

---

## 1. 文件职责总览

`DynamicProductContext` 是**单个产品**的动态检测上下文。  
一个对象对应一个产品和一个动态检测通道，例如：

```text
位置1：txId = 0x110，rxId = 0x201
位置2：txId = 0x112，rxId = 0x301
```

它的主要职责是：

```text
1. 保存当前产品条码、通道、seed、thresholdFlag；
2. 接收并解析当前产品对应 rxId 的 CAN 报文；
3. 按 groupCnt 缓存同一轮询组的数据；
4. 只在 DetectInProgressFlag 到 100 后缓存最终 flag/rec 数据；
5. 在组校验通过或下一组开始时锁定最终结果；
6. 根据 DynamicTestItemMeta 生成 DynamicItemResult；
7. 生成 DynamicProductResult，供 Controller 上报给 MainWindow。
```

---

## 2. 当前动态数据流

```text
ZlgCanDevice 收到 CAN 帧
        ↓
DynamicTestController::onFrameReceived()
        ↓
m_product1.handleFrame(canId, data)
m_product2.handleFrame(canId, data)
        ↓
DynamicProductContext::handleFrame()
        ↓
parseFrame() 解析 seq / seed / value / groupCnt / check
        ↓
ensureActiveGroup() 判断是否同一轮 groupCnt
        ↓
applyNormalFrame() 或 applyGroupCheckFrame()
        ↓
m_values / m_received / m_checkOk 缓存最终数据
        ↓
buildResult()
        ↓
DynamicProductResult
```

---

## 3. `dynamic_product_context.h` 逐行解释

| 行号 | 代码 | 解释 |
|---:|---|---|
| 1 | `#ifndef DYNAMIC_PRODUCT_CONTEXT_H` | 头文件防重复包含保护：如果宏未定义，继续展开本头文件。 |
| 2 | `#define DYNAMIC_PRODUCT_CONTEXT_H` | 定义头文件保护宏，防止同一个头文件被重复包含。 |
| 4 | `#include <QString>` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |
| 5 | `#include <QVector>` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |
| 6 | `#include <QDateTime>` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |
| 7 | `#include <QByteArray>` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |
| 9 | `#include "dynamic_protocol.h"` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |
| 10 | `#include "dynamic_result_types.h"` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |
| 11 | `#include "dynamic_test_item_config.h"` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |
| 13 | `/*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 14 | ` * 单个产品的动态检测数据上下文。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 15 | ` *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 16 | ` * 一个 DynamicProductContext 对应一个产品：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 17 | ` *   例如 txId=0x110，rxId=0x201；` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 18 | ` *   或 txId=0x112，rxId=0x301。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 19 | ` *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 20 | ` * 它只负责：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 21 | ` * 1. 缓存 seq 0~60 数据；` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 22 | ` * 2. 做 seed 和单帧校验；` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 23 | ` * 3. 根据 DetectInProgressFlag == 100 判断检测完成；` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 24 | ` * 4. 生成 DynamicProductResult。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 25 | ` */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 27 | `class DynamicProductContext` | 声明 DynamicProductContext 类，表示单个产品的一次动态检测上下文。 |
| 28 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 29 | `public:` | 访问控制标记：后续成员属于 `public` 区域。 |
| 30 | `    DynamicProductContext();` | 成员函数声明，具体实现在 cpp 文件中。 |
| 32 | `    void setup(const DynamicProtocol::ChannelConfig &channelConfig,` | 上一条语句的续行，继续传递参数或表达式。 |
| 33 | `               const QString &productCode,` | 上一条语句的续行，继续传递参数或表达式。 |
| 34 | `               int seed,` | 上一条语句的续行，继续传递参数或表达式。 |
| 35 | `               const QVector<DynamicTestItemMeta> &itemMetas);` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 37 | `    void reset();` | 声明复位接口，用于清空上下文状态。 |
| 39 | `    bool handleFrame(quint32 canId,` | 上一条语句的续行，继续传递参数或表达式。 |
| 40 | `                     const QByteArray &data);` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 42 | `    bool isConfigured() const;` | 声明状态查询接口：判断上下文是否已经配置。 |
| 43 | `    bool isFinished() const;` | 声明状态查询接口：判断产品是否已经完成动态检测。 |
| 44 | `    bool isReported() const;` | 声明状态查询接口：判断结果是否已经上报。 |
| 45 | `    void markReported();` | 声明标记接口：把当前产品设为已上报。 |
| 47 | `    int productId() const;` | 声明读取产品位置编号接口。 |
| 48 | `    QString productCode() const;` | 声明读取产品条码接口。 |
| 50 | `    quint32 txId() const;` | 声明读取当前产品发送 CAN ID 接口。 |
| 51 | `    quint32 rxId() const;` | 声明读取当前产品接收 CAN ID 接口。 |
| 53 | `    int seed() const;` | 声明读取当前 seed 接口。 |
| 54 | `    int thresholdFlag() const;` | 声明读取当前 thresholdFlag 接口，用于开始检测帧。 |
| 56 | `    int groupCnt() const;` | 声明读取当前 groupCnt 接口。 |
| 57 | `    int detectProgress() const;` | 声明读取当前检测进度接口。 |
| 59 | `    int receivedCount() const;` | 声明统计已接收 seq 数量的接口。 |
| 60 | `    int checkOkCount() const;` | 声明统计校验通过 seq 数量的接口。 |
| 62 | `    DynamicProductResult buildResult(const QString &message = QString()) const;` | 声明构建产品动态检测最终结果接口。 |
| 64 | `private:` | 访问控制标记：后续成员属于 `private` 区域。 |
| 65 | `    void resetCache();` | 声明内部缓存清空函数。 |
| 67 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 68 | `     * 新增：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 69 | `     * 按 groupCnt 管理一整组轮询数据。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 70 | `     * 只有同一个 groupCnt 的 flag 和 rec 才允许组成最终结果。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 71 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 72 | `    void startNewGroup(int groupCnt);` | 声明读取当前 groupCnt 接口。 |
| 74 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 75 | `     * 新增：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 76 | `     * 当上一组 DetectInProgressFlag 已经到 100，` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 77 | `     * 且检测到下一组 groupCnt 开始时，锁定上一组作为最终结果。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 78 | `     *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 79 | `     * 这样不依赖 seq=255 组校验帧。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 80 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 81 | `    void lockActiveGroupAsFinished(const QString &reason);` | 声明锁定当前 active group 为最终结果的函数。 |
| 83 | `    bool ensureActiveGroup(const DynamicProtocol::ParsedFrame &frame);` | 声明确保当前报文属于 active group 的函数。 |
| 85 | `    void applyNormalFrame(const DynamicProtocol::ParsedFrame &frame);` | 声明普通数据帧落库函数。 |
| 86 | `    void applyGroupCheckFrame(const DynamicProtocol::ParsedFrame &frame);` | 声明组校验帧处理函数。 |
| 87 | `    bool hasRequiredFrames() const;` | 声明检查必要 flag/rec 是否齐全的函数。 |
| 89 | `    DynamicItemResult buildItemResult(const DynamicTestItemMeta &meta) const;` | 声明按配置生成单个检测项结果的函数。 |
| 90 | `    bool calcProductPass(const QVector<DynamicItemResult> &items) const;` | 声明产品总判定函数。 |
| 92 | `    QString currentTimeText() const;` | 声明当前时间字符串生成函数。 |
| 95 | `private:` | 访问控制标记：后续成员属于 `private` 区域。 |
| 96 | `    struct DebugFrameRecord` | 声明调试报文记录结构体，用于保存一帧原始数据和解析结果。 |
| 97 | `    {` | 代码块边界，用于开始或结束当前作用域。 |
| 98 | `        DynamicProtocol::ParsedFrame frame;` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 99 | `        QByteArray rawData;` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 100 | `        bool groupCheckFrame = false;` | 赋值语句，将右侧结果保存到左侧变量或成员中。 |
| 101 | `    };` | 代码块边界，用于开始或结束当前作用域。 |
| 103 | `    void clearActiveGroupDebugFrames();` | 成员函数声明，具体实现在 cpp 文件中。 |
| 105 | `    void recordActiveGroupFrame(const DynamicProtocol::ParsedFrame &frame,` | 上一条语句的续行，继续传递参数或表达式。 |
| 106 | `                                const QByteArray &rawData,` | 上一条语句的续行，继续传递参数或表达式。 |
| 107 | `                                bool groupCheckFrame);` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 109 | `    const DebugFrameRecord *findLastDebugFrameBySeq(int seq) const;` | 成员函数声明，具体实现在 cpp 文件中。 |
| 111 | `    void debugDumpProductNg(const QVector<DynamicItemResult> &items) const;` | 成员函数声明，具体实现在 cpp 文件中。 |
| 112 | `    void debugDumpNgItem(const DynamicItemResult &item) const;` | 成员函数声明，具体实现在 cpp 文件中。 |
| 113 | `    void debugDumpCurrentGroupFrames(const QString &reason) const;` | 成员函数声明，具体实现在 cpp 文件中。 |
| 115 | `    QString debugFrameLine(const DebugFrameRecord &record) const;` | 成员函数声明，具体实现在 cpp 文件中。 |
| 117 | `private:` | 访问控制标记：后续成员属于 `private` 区域。 |
| 118 | `    bool m_configured = false;` | 记录当前上下文是否已经 setup，未配置时不处理 CAN 帧。 |
| 120 | `    DynamicProtocol::ChannelConfig m_channelConfig;` | 保存当前产品使用的动态通道配置，包括 txId、rxId、productId、thresholdFlag 等。 |
| 122 | `    QString m_productCode;` | 保存当前产品条码。 |
| 124 | `    int m_seed = 0;` | 保存开始检测帧使用的 seed，后续接收帧必须匹配该 seed。 |
| 125 | `    int m_groupCnt = 0;` | 保存当前缓存数据所属的轮询组编号。 |
| 126 | `    int m_detectProgress = 0;` | 保存 DetectInProgressFlag 的当前进度值。 |
| 128 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 129 | `     * 新增：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 130 | `     * 当前正在缓存的轮询组。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 131 | `     * m_values / m_received / m_checkOk 只代表这一组的数据。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 132 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 133 | `    bool m_hasActiveGroup = false;` | 标记是否已经建立当前 active group。 |
| 134 | `    int m_activeGroupCnt = 0;` | 保存当前 active group 的 groupCnt。 |
| 136 | `    bool m_finished = false;` | 标记该产品动态检测是否已经锁定完成。 |
| 137 | `    bool m_reported = false;` | 标记该产品结果是否已经上报给 controller，避免重复上报。 |
| 138 | `    bool m_finalDataMode = false;` | 标记是否已经进入最终数据阶段；进度到 100 后才缓存普通 flag/rec 数据。 |
| 140 | `    bool m_groupCheckReceived = false;` | 标记当前组是否收到 seq=255 组校验帧。 |
| 141 | `    bool m_groupCheckPassed = false;` | 标记当前组校验是否通过。 |
| 143 | `    QString m_startTime;` | 保存该产品动态检测开始时间。 |
| 144 | `    QString m_finishTime;` | 保存该产品动态检测完成时间。 |
| 146 | `    int m_values[61];` | 按 seq 下标缓存数据值，范围 0~60。 |
| 147 | `    bool m_received[61];` | 按 seq 下标记录对应数据是否收到且被缓存。 |
| 148 | `    bool m_checkOk[61];` | 按 seq 下标记录单帧校验是否通过。 |
| 151 | `    QVector<DynamicTestItemMeta> m_itemMetas;` | 保存动态检测项配置，用于生成每个 DynamicItemResult。 |
| 153 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 154 | `     * 调试用：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 155 | `     * 保存当前 active groupCnt 对应的所有原始报文和解析结果。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 156 | `     * 当产品 NG 时，qDebug 输出这些报文，便于确认界面显示的 item 是否混组。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 157 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 158 | `    QVector<DebugFrameRecord> m_activeGroupDebugFrames;` | 调试用缓存：保存当前 active group 的原始报文记录。 |
| 159 | `};` | 代码块边界，用于开始或结束当前作用域。 |
| 162 | `#endif // DYNAMIC_PRODUCT_CONTEXT_H` | 结束头文件防重复包含保护。 |

---

## 4. `dynamic_product_context.cpp` 逐行解释

### 文件头部 include

| 行号 | 代码 | 解释 |
|---:|---|---|
| 1 | `#include "dynamic_product_context.h"` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |
| 3 | `#include <QDebug>` | 引入依赖头文件，使当前文件可以使用其中声明的类型或函数。 |

### DynamicProductContext::DynamicProductContext()

| 行号 | 代码 | 解释 |
|---:|---|---|
| 5 | `DynamicProductContext::DynamicProductContext()` | 构造函数定义，创建对象时执行。 |
| 6 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 7 | `    resetCache();` | 声明内部缓存清空函数。 |
| 8 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### void DynamicProductContext::setup(const DynamicProtocol::ChannelConfig &channelConfig,

| 行号 | 代码 | 解释 |
|---:|---|---|
| 10 | `void DynamicProductContext::setup(const DynamicProtocol::ChannelConfig &channelConfig,` | setup 函数定义，用于初始化单产品上下文。 |
| 11 | `                                  const QString &productCode,` | 上一条语句的续行，继续传递参数或表达式。 |
| 12 | `                                  int seed,` | 上一条语句的续行，继续传递参数或表达式。 |
| 13 | `                                  const QVector<DynamicTestItemMeta> &itemMetas)` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 14 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 15 | `    reset();` | 声明复位接口，用于清空上下文状态。 |
| 17 | `    m_channelConfig = channelConfig;` | 保存当前产品通道配置。 |
| 18 | `    m_productCode = productCode;` | 保存产品条码。 |
| 19 | `    m_seed = seed & 0xFF;` | 只保留 seed 的低 8 位，保证与协议 Byte 范围一致。 |
| 20 | `    m_itemMetas = itemMetas;` | 保存检测项配置表。 |
| 21 | `    m_startTime = currentTimeText();` | 声明当前时间字符串生成函数。 |
| 22 | `    m_configured = true;` | 标记上下文已配置完成，后续允许处理 CAN 帧。 |
| 23 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### void DynamicProductContext::reset()

| 行号 | 代码 | 解释 |
|---:|---|---|
| 25 | `void DynamicProductContext::reset()` | reset 函数定义，用于恢复到未配置、未检测状态。 |
| 26 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 27 | `    m_configured = false;` | 清除配置完成标志。 |
| 28 | `    m_channelConfig = DynamicProtocol::ChannelConfig();` | 成员函数声明，具体实现在 cpp 文件中。 |
| 29 | `    m_productCode.clear();` | 声明读取产品条码接口。 |
| 31 | `    m_seed = 0;` | 清空 seed。 |
| 32 | `    m_groupCnt = 0;` | 清空当前 groupCnt。 |
| 33 | `    m_detectProgress = 0;` | 清空检测进度。 |
| 35 | `    m_hasActiveGroup = false;` | 标记当前没有 active group。 |
| 36 | `    m_activeGroupCnt = 0;` | 清空 active group 编号。 |
| 38 | `    m_finished = false;` | 标记产品未完成。 |
| 39 | `    m_reported = false;` | 标记产品未上报。 |
| 40 | `    m_finalDataMode = false;` | 退出最终数据缓存阶段。 |
| 42 | `    m_groupCheckReceived = false;` | 清除组校验帧接收标志。 |
| 43 | `    m_groupCheckPassed = false;` | 清除组校验通过标志。 |
| 45 | `    clearActiveGroupDebugFrames();` | 成员函数声明，具体实现在 cpp 文件中。 |
| 47 | `    m_startTime.clear();` | 成员函数声明，具体实现在 cpp 文件中。 |
| 48 | `    m_finishTime.clear();` | 成员函数声明，具体实现在 cpp 文件中。 |
| 50 | `    resetCache();` | 声明内部缓存清空函数。 |
| 51 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### bool DynamicProductContext::handleFrame(quint32 canId,

| 行号 | 代码 | 解释 |
|---:|---|---|
| 53 | `bool DynamicProductContext::handleFrame(quint32 canId,` | handleFrame 函数定义，用于处理一帧 CAN 数据。 |
| 54 | `                                        const QByteArray &data)` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 55 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 56 | `    if (!m_configured) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 57 | `        return false;` | 返回 false，表示条件不满足、处理失败或结果不通过。 |
| 58 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 60 | `    if (canId != m_channelConfig.rxId) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 61 | `        return false;` | 返回 false，表示条件不满足、处理失败或结果不通过。 |
| 62 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 64 | `    if (m_reported) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 65 | `        /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 66 | `         * 测试板在开始检测后会持续轮询发送数据。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 67 | `         * 结果已上报后继续收到同 ID 数据，不再更新最终结果。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 68 | `         */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 69 | `        return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 70 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 72 | `    DynamicProtocol::ParsedFrame frame =` | 定义解析后的动态协议帧对象。 |
| 73 | `            DynamicProtocol::parseFrame(canId, data, m_seed);` | 声明读取当前 seed 接口。 |
| 75 | `    if (!frame.valid) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 76 | `        return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 77 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 79 | `    if (!frame.seedOk) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 80 | `        return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 81 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 83 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 84 | `     * 结果已经锁定后，不再接收后续轮询组数据。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 85 | `     * 避免 DetectInProgressFlag=100 后，下一轮组的数据把最终结果覆盖。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 86 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 87 | `    if (m_finished) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 88 | `        return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 89 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 91 | `    if (DynamicProtocol::isNormalDataSeq(frame.seq)) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 92 | `        /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 93 | `         * 普通数据帧必须归属于当前轮询组。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 94 | `         * 如果 groupCnt 变化，说明进入了新一轮轮询组，` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 95 | `         * 旧组未完成的数据直接丢弃，重新开始缓存新组。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 96 | `         */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 97 | `        if (!ensureActiveGroup(frame)) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 98 | `            return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 99 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 101 | `        recordActiveGroupFrame(frame, data, false);` | 成员函数声明，具体实现在 cpp 文件中。 |
| 102 | `        applyNormalFrame(frame);` | 声明普通数据帧落库函数。 |
| 103 | `        return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 104 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 106 | `    if (DynamicProtocol::isGroupCheckSeq(frame.seq)) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 107 | `        /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 108 | `         * 组校验帧只允许校验当前正在缓存的 groupCnt。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 109 | `         * 如果 groupCnt 不一致，不能拿它去校验当前缓存，直接忽略。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 110 | `         */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 111 | `        if (!m_hasActiveGroup \|\| frame.groupCnt != m_activeGroupCnt) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 112 | `            return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 113 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 115 | `        recordActiveGroupFrame(frame, data, true);` | 成员函数声明，具体实现在 cpp 文件中。 |
| 116 | `        applyGroupCheckFrame(frame);` | 声明组校验帧处理函数。 |
| 117 | `        return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 118 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 120 | `    return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 121 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### bool DynamicProductContext::isConfigured() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 123 | `bool DynamicProductContext::isConfigured() const` | 返回当前上下文是否已经配置。 |
| 124 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 125 | `    return m_configured;` | 返回 `m_configured` 的值。 |
| 126 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### bool DynamicProductContext::isFinished() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 128 | `bool DynamicProductContext::isFinished() const` | 返回当前产品检测是否完成。 |
| 129 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 130 | `    return m_finished;` | 返回 `m_finished` 的值。 |
| 131 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### bool DynamicProductContext::isReported() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 133 | `bool DynamicProductContext::isReported() const` | 返回当前产品结果是否已上报。 |
| 134 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 135 | `    return m_reported;` | 返回 `m_reported` 的值。 |
| 136 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### void DynamicProductContext::markReported()

| 行号 | 代码 | 解释 |
|---:|---|---|
| 138 | `void DynamicProductContext::markReported()` | 把当前产品结果标记为已上报。 |
| 139 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 140 | `    m_reported = true;` | 标记该产品结果已经上报。 |
| 141 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### int DynamicProductContext::productId() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 143 | `int DynamicProductContext::productId() const` | 返回当前产品位置编号。 |
| 144 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 145 | `    return m_channelConfig.productId;` | 返回 `m_channelConfig.productId` 的值。 |
| 146 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### QString DynamicProductContext::productCode() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 148 | `QString DynamicProductContext::productCode() const` | 返回当前产品条码。 |
| 149 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 150 | `    return m_productCode;` | 返回 `m_productCode` 的值。 |
| 151 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### quint32 DynamicProductContext::txId() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 153 | `quint32 DynamicProductContext::txId() const` | 返回当前产品发送 CAN ID。 |
| 154 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 155 | `    return m_channelConfig.txId;` | 返回 `m_channelConfig.txId` 的值。 |
| 156 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### quint32 DynamicProductContext::rxId() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 158 | `quint32 DynamicProductContext::rxId() const` | 返回当前产品接收 CAN ID。 |
| 159 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 160 | `    return m_channelConfig.rxId;` | 返回 `m_channelConfig.rxId` 的值。 |
| 161 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### int DynamicProductContext::seed() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 163 | `int DynamicProductContext::seed() const` | 返回当前产品 seed。 |
| 164 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 165 | `    return m_seed;` | 返回 `m_seed` 的值。 |
| 166 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### int DynamicProductContext::thresholdFlag() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 168 | `int DynamicProductContext::thresholdFlag() const` | 返回当前产品 thresholdFlag。 |
| 169 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 170 | `    return m_channelConfig.thresholdFlag;` | 返回 `m_channelConfig.thresholdFlag` 的值。 |
| 171 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### int DynamicProductContext::groupCnt() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 173 | `int DynamicProductContext::groupCnt() const` | 返回当前已缓存数据所属 groupCnt。 |
| 174 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 175 | `    return m_groupCnt;` | 返回 `m_groupCnt` 的值。 |
| 176 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### int DynamicProductContext::detectProgress() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 178 | `int DynamicProductContext::detectProgress() const` | 返回当前检测进度。 |
| 179 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 180 | `    return m_detectProgress;` | 返回 `m_detectProgress` 的值。 |
| 181 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### int DynamicProductContext::receivedCount() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 183 | `int DynamicProductContext::receivedCount() const` | 统计当前缓存中已收到的 seq 数量。 |
| 184 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 185 | `    int count = 0;` | 定义计数变量并初始化为 0。 |
| 186 | `    for (int i = 0; i <= 60; ++i) {` | 循环语句，按条件重复执行循环体。 |
| 187 | `        if (m_received[i]) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 188 | `            ++count;` | 自增操作，将变量加 1。 |
| 189 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 190 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 191 | `    return count;` | 返回 `count` 的值。 |
| 192 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### int DynamicProductContext::checkOkCount() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 194 | `int DynamicProductContext::checkOkCount() const` | 统计当前缓存中校验通过的 seq 数量。 |
| 195 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 196 | `    int count = 0;` | 定义计数变量并初始化为 0。 |
| 197 | `    for (int i = 0; i <= 60; ++i) {` | 循环语句，按条件重复执行循环体。 |
| 198 | `        if (m_checkOk[i]) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 199 | `            ++count;` | 自增操作，将变量加 1。 |
| 200 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 201 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 202 | `    return count;` | 返回 `count` 的值。 |
| 203 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### DynamicProductResult DynamicProductContext::buildResult(const QString &message) const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 205 | `DynamicProductResult DynamicProductContext::buildResult(const QString &message) const` | 生成当前产品的最终动态检测结果。 |
| 206 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 207 | `    DynamicProductResult result;` | 创建产品结果对象，用于汇总最终结果。 |
| 209 | `    result.productId = m_channelConfig.productId;` | 把产品位置编号写入结果。 |
| 210 | `    result.productCode = m_productCode;` | 把产品条码写入结果。 |
| 211 | `    result.channelName = m_channelConfig.name;` | 把通道名称写入结果。 |
| 213 | `    result.txId = m_channelConfig.txId;` | 把发送 CAN ID 写入结果。 |
| 214 | `    result.rxId = m_channelConfig.rxId;` | 把接收 CAN ID 写入结果。 |
| 216 | `    result.seed = m_seed;` | 把 seed 写入结果。 |
| 217 | `    result.groupCnt = m_groupCnt;` | 把最终 groupCnt 写入结果。 |
| 218 | `    result.detectProgress = m_detectProgress;` | 把检测进度写入结果。 |
| 220 | `    result.receivedCount = receivedCount();` | 声明统计已接收 seq 数量的接口。 |
| 221 | `    result.checkOkCount = checkOkCount();` | 声明统计校验通过 seq 数量的接口。 |
| 223 | `    result.finished = m_finished;` | 写入产品是否完成。 |
| 225 | `    result.groupCheckReceived = m_groupCheckReceived;` | 写入是否收到组校验帧。 |
| 226 | `    result.groupCheckPassed = m_groupCheckPassed;` | 写入组校验是否通过。 |
| 228 | `    result.startTime = m_startTime;` | 写入检测开始时间。 |
| 229 | `    result.finishTime = m_finishTime.isEmpty() ? currentTimeText() : m_finishTime;` | 声明当前时间字符串生成函数。 |
| 231 | `    for (const DynamicTestItemMeta &meta : m_itemMetas) {` | 循环语句，按条件重复执行循环体。 |
| 232 | `        result.itemResults.append(buildItemResult(meta));` | 声明按配置生成单个检测项结果的函数。 |
| 233 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 235 | `    result.pass = calcProductPass(result.itemResults);` | 声明产品总判定函数。 |
| 237 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 238 | `     * 最小化调试：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 239 | `     * 产品 NG 时，输出：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 240 | `     *   1. 不合格 item 对应的 flag 原始报文；` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 241 | `     *   2. 该报文解析后的结果；` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 242 | `     *   3. 当前界面显示结果所属轮询组的所有报文。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 243 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 244 | `    if (!result.pass) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 245 | `        debugDumpProductNg(result.itemResults);` | 成员函数声明，具体实现在 cpp 文件中。 |
| 246 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 248 | `    if (!message.isEmpty()) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 249 | `        result.message = message;` | 使用外部传入的结果消息覆盖默认消息。 |
| 250 | `    } else if (!result.finished) {` | 前一个条件不成立时，继续判断新的条件。 |
| 251 | `        result.message = "检测未完成";` | 产品未完成时设置结果消息。 |
| 252 | `    } else if (result.pass) {` | 前一个条件不成立时，继续判断新的条件。 |
| 253 | `        result.message = "动态检测合格";` | 产品通过时设置结果消息。 |
| 254 | `    } else {` | 前面条件均不成立时，执行该分支。 |
| 255 | `        result.message = "动态检测不合格";` | 产品不通过时设置结果消息。 |
| 256 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 258 | `    return result;` | 返回 `result` 的值。 |
| 259 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### void DynamicProductContext::resetCache()

| 行号 | 代码 | 解释 |
|---:|---|---|
| 261 | `void DynamicProductContext::resetCache()` | 清空 seq 缓存数组。 |
| 262 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 263 | `    for (int i = 0; i <= 60; ++i) {` | 循环语句，按条件重复执行循环体。 |
| 264 | `        m_values[i] = 0;` | 将当前 seq 对应的数据值清零。 |
| 265 | `        m_received[i] = false;` | 将当前 seq 的接收标志清 false。 |
| 266 | `        m_checkOk[i] = false;` | 将当前 seq 的校验通过标志清 false。 |
| 267 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 269 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 270 | `     * 协议：合格标志位默认 1，1 代表不合格。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 271 | `     * 这里按 Excel 的 1~31 标志位初始化。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 272 | `     * 如果现场确实存在 seq=0 标志位，也不会影响接收，收到后会覆盖。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 273 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 274 | `    for (int i = 1; i <= 31; ++i) {` | 循环语句，按条件重复执行循环体。 |
| 275 | `        m_values[i] = 1;` | 将 flag 类 seq 默认置为 1，按协议含义代表默认不合格。 |
| 276 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 277 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### void DynamicProductContext::startNewGroup(int groupCnt)

| 行号 | 代码 | 解释 |
|---:|---|---|
| 279 | `void DynamicProductContext::startNewGroup(int groupCnt)` | 开始新的轮询组，清空旧组缓存并绑定新的 groupCnt。 |
| 280 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 281 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 282 | `     * 每一个 groupCnt 都是一组独立轮询数据。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 283 | `     *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 284 | `     * 关键原则：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 285 | `     *   m_values / m_received / m_checkOk 只允许保存同一组 groupCnt 的数据。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 286 | `     *   一旦 groupCnt 变化，旧组未完成数据全部清空，避免 flag 和 rec 混组。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 287 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 288 | `    resetCache();` | 声明内部缓存清空函数。 |
| 289 | `    clearActiveGroupDebugFrames();` | 成员函数声明，具体实现在 cpp 文件中。 |
| 291 | `    m_hasActiveGroup = true;` | 标记当前已经有 active group。 |
| 292 | `    m_activeGroupCnt = groupCnt;` | 记录当前 active group 的 groupCnt。 |
| 294 | `    m_groupCnt = groupCnt;` | 同步当前对外显示/结果使用的 groupCnt。 |
| 295 | `    m_detectProgress = m_finalDataMode ? DynamicProtocol::kDetectFinishedValue : 0;` | 开始新组时，如果已进入最终数据模式则保持进度 100，否则进度归 0。 |
| 297 | `    m_groupCheckReceived = false;` | 清除组校验帧接收标志。 |
| 298 | `    m_groupCheckPassed = false;` | 清除组校验通过标志。 |
| 299 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### void DynamicProductContext::lockActiveGroupAsFinished(const QString &reason)

| 行号 | 代码 | 解释 |
|---:|---|---|
| 302 | `void DynamicProductContext::lockActiveGroupAsFinished(const QString &reason)` | 把当前 active group 锁定为最终结果。 |
| 303 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 304 | `    if (m_finished) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 305 | `        return;` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 306 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 308 | `    m_finished = true;` | 标记当前产品检测已完成并锁定。 |
| 310 | `    if (m_finishTime.isEmpty()) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 311 | `        m_finishTime = currentTimeText();` | 声明当前时间字符串生成函数。 |
| 312 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 323 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### bool DynamicProductContext::ensureActiveGroup(const DynamicProtocol::ParsedFrame &frame)

| 行号 | 代码 | 解释 |
|---:|---|---|
| 325 | `bool DynamicProductContext::ensureActiveGroup(const DynamicProtocol::ParsedFrame &frame)` | 确认当前帧是否属于当前 active group，不属于时决定锁定旧组或开启新组。 |
| 326 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 327 | `    if (!m_hasActiveGroup) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 328 | `        startNewGroup(frame.groupCnt);` | 声明读取当前 groupCnt 接口。 |
| 329 | `        return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 330 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 332 | `    if (frame.groupCnt != m_activeGroupCnt) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 333 | `        /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 334 | `         * 关键修正：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 335 | `         *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 336 | `         * 上一版要求 DetectInProgressFlag==100 后，还必须等 seq=255 组校验帧，` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 337 | `         * 但现场测试表现为进度已经到 100，流程仍不停止。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 338 | `         *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 339 | `         * 说明检测板可能没有发送组校验帧，或者当前工程没有收到/识别到组校验帧。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 340 | `         *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 341 | `         * 现在改成：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 342 | `         *   如果上一组的 DetectInProgressFlag 已经到 100，` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 343 | `         *   当检测到下一组 groupCnt 开始时，说明上一组轮询已经结束，` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 344 | `         *   此时锁定上一组作为最终结果。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 345 | `         *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 346 | `         * 这样仍然保证最终显示的 item 数据来自同一个 groupCnt，` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 347 | `         * 并且不会把新组第一帧塞进上一组结果。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 348 | `         */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 349 | `        if (m_detectProgress == DynamicProtocol::kDetectFinishedValue &&` | 条件判断；只有条件成立时才执行后续代码块。 |
| 350 | `                hasRequiredFrames()) {` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 351 | `            lockActiveGroupAsFinished("DetectInProgressFlag=100 and next group started");` | 声明锁定当前 active group 为最终结果的函数。 |
| 352 | `            return false;` | 返回 false，表示条件不满足、处理失败或结果不通过。 |
| 353 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 355 | `        /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 356 | `         * 上一组没有完成，丢弃上一组未完成缓存，重新开始新组。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 357 | `         */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 358 | `        startNewGroup(frame.groupCnt);` | 声明读取当前 groupCnt 接口。 |
| 359 | `        return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 360 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 362 | `    return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 363 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### void DynamicProductContext::applyNormalFrame(const DynamicProtocol::ParsedFrame &frame)

| 行号 | 代码 | 解释 |
|---:|---|---|
| 365 | `void DynamicProductContext::applyNormalFrame(const DynamicProtocol::ParsedFrame &frame)` | 处理普通数据帧，把有效 seq 数据写入缓存。 |
| 366 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 367 | `    if (!frame.checkOk) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 368 | `        if (frame.seq >= 0 && frame.seq <= 60) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 369 | `            m_checkOk[frame.seq] = false;` | 标记当前 seq 单帧校验失败。 |
| 370 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 371 | `        return;` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 372 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 374 | `    m_groupCnt = frame.groupCnt;` | 更新当前 groupCnt 为该帧所在轮询组。 |
| 376 | `    if (DynamicProtocol::isDetectInProgressSeq(frame.seq)) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 377 | `        /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 378 | `         * 这里只记录当前轮询组的进度值。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 379 | `         * 不能在收到 DetectInProgressFlag==100 的瞬间就完成并上报，` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 380 | `         * 因为同一组后续 rec 数据可能还没接收完。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 381 | `         *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 382 | `         * 最终完成在 applyGroupCheckFrame() 中锁定：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 383 | `         *   当前组 DetectInProgressFlag == 100` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 384 | `         *   并且当前组组校验通过` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 385 | `         */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 386 | `        m_detectProgress = frame.value;` | 把 DetectInProgressFlag 的 value 保存为进度值。 |
| 387 | `        if (frame.value == DynamicProtocol::kDetectFinishedValue) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 388 | `            m_finalDataMode = true;` | 标记进入最终数据阶段，后续普通 flag/rec 才允许缓存。 |
| 389 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 391 | `        m_values[frame.seq] = frame.value;` | 按 seq 保存当前帧 value。 |
| 392 | `        m_received[frame.seq] = true;` | 标记当前 seq 已收到。 |
| 393 | `        m_checkOk[frame.seq] = true;` | 标记当前 seq 单帧校验通过。 |
| 394 | `        return;` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 395 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 397 | `    if (!m_finalDataMode) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 398 | `        /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 399 | `         * Match the legacy dynamic tester: before DetectInProgressFlag reaches` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 400 | `         * 100, normal flag/record frames are treated as intermediate data and` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 401 | `         * are not cached into the final result group.` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 402 | `         */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 403 | `        return;` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 404 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 406 | `    if (frame.seq >= 0 && frame.seq <= 60) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 407 | `        m_values[frame.seq] = frame.value;` | 按 seq 保存当前帧 value。 |
| 408 | `        m_received[frame.seq] = true;` | 标记当前 seq 已收到。 |
| 409 | `        m_checkOk[frame.seq] = true;` | 标记当前 seq 单帧校验通过。 |
| 410 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 411 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### void DynamicProductContext::applyGroupCheckFrame(const DynamicProtocol::ParsedFrame &frame)

| 行号 | 代码 | 解释 |
|---:|---|---|
| 413 | `void DynamicProductContext::applyGroupCheckFrame(const DynamicProtocol::ParsedFrame &frame)` | 处理组校验帧，计算并记录当前组校验结果。 |
| 414 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 415 | `    m_groupCheckReceived = true;` | 记录已经收到组校验帧。 |
| 416 | `    m_groupCnt = frame.groupCnt;` | 更新当前 groupCnt 为该帧所在轮询组。 |
| 418 | `    const int calc = DynamicProtocol::calcGroupCheck(m_values,` | 开始计算本地组校验值。 |
| 419 | `                                                     m_seed,` | 上一条语句的续行，继续传递参数或表达式。 |
| 420 | `                                                     frame.groupCnt);` | 普通语句，执行一次函数调用、声明或状态更新。 |
| 422 | `    m_groupCheckPassed = (calc == frame.checkValue);` | 成员函数声明，具体实现在 cpp 文件中。 |
| 424 | `    /*` | 多行注释开始，说明后续代码设计目的或业务规则。 |
| 425 | `     * 只有当前同一轮询组同时满足：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 426 | `     *   1. DetectInProgressFlag == 100` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 427 | `     *   2. 当前组组校验通过` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 428 | `     *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 429 | `     * 才允许锁定结果。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 430 | `     *` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 431 | `     * 这样可以保证：` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 432 | `     *   每个 item 的 flag 和 rec 来自同一个 groupCnt；` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 433 | `     *   不会把上一组 flag 和下一组 rec 拼到同一个 item；` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 434 | `     *   不会在进度刚到 100、但同组后续数据还没收齐时提前上报。` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 435 | `     */` | 多行注释内容，补充说明当前函数或变量的业务含义。 |
| 436 | `    if (m_detectProgress == DynamicProtocol::kDetectFinishedValue &&` | 条件判断；只有条件成立时才执行后续代码块。 |
| 437 | `            m_groupCheckPassed &&` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 438 | `            hasRequiredFrames()) {` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 439 | `        lockActiveGroupAsFinished("DetectInProgressFlag=100 and group check passed");` | 声明锁定当前 active group 为最终结果的函数。 |
| 440 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 441 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### bool DynamicProductContext::hasRequiredFrames() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 443 | `bool DynamicProductContext::hasRequiredFrames() const` | 检查配置中要求的 flagSeq/valueSeq 是否都已经收到。 |
| 444 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 445 | `    for (const DynamicTestItemMeta &meta : m_itemMetas) {` | 循环语句，按条件重复执行循环体。 |
| 446 | `        if (meta.flagSeq < 0 \|\| meta.flagSeq > 60 \|\| !m_received[meta.flagSeq]) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 447 | `            return false;` | 返回 false，表示条件不满足、处理失败或结果不通过。 |
| 448 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 450 | `        if (meta.valueSeq >= 0 &&` | 条件判断；只有条件成立时才执行后续代码块。 |
| 451 | `                (meta.valueSeq > 60 \|\| !m_received[meta.valueSeq])) {` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 452 | `            return false;` | 返回 false，表示条件不满足、处理失败或结果不通过。 |
| 453 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 454 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 456 | `    return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 457 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### DynamicItemResult DynamicProductContext::buildItemResult(const DynamicTestItemMeta &meta) const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 459 | `DynamicItemResult DynamicProductContext::buildItemResult(const DynamicTestItemMeta &meta) const` | 把一个检测项配置 meta 转换为实际检测项结果 item。 |
| 460 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 461 | `    DynamicItemResult item;` | 创建单项检测结果对象。 |
| 463 | `    item.itemNo = meta.itemNo;` | 写入检测项编号。 |
| 464 | `    item.itemName = meta.itemName;` | 写入检测项名称。 |
| 465 | `    item.standardText = meta.standardText;` | 写入检测标准文本。 |
| 466 | `    item.unit = meta.unit;` | 写入单位。 |
| 468 | `    item.flagSeq = meta.flagSeq;` | 写入该项对应的 flagSeq。 |
| 469 | `    item.valueSeq = meta.valueSeq;` | 写入该项对应的 valueSeq。 |
| 471 | `    if (meta.flagSeq >= 0 && meta.flagSeq <= 60) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 472 | `        item.flagValue = m_values[meta.flagSeq];` | 从缓存数组中读取该项 flag 值。 |
| 473 | `        item.flagReceived = m_received[meta.flagSeq];` | 读取该项 flag 是否收到。 |
| 474 | `        item.flagCheckOk = m_checkOk[meta.flagSeq];` | 读取该项 flag 单帧校验是否通过。 |
| 475 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 477 | `    if (meta.valueSeq >= 0 && meta.valueSeq <= 60) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 478 | `        item.rawValue = m_values[meta.valueSeq];` | 从缓存数组中读取该项原始检测值。 |
| 479 | `        item.valueReceived = m_received[meta.valueSeq];` | 读取该项检测值是否收到。 |
| 480 | `        item.valueCheckOk = m_checkOk[meta.valueSeq];` | 读取该项检测值单帧校验是否通过。 |
| 481 | `        item.displayValue = static_cast<double>(item.rawValue) * meta.scale;` | 成员函数声明，具体实现在 cpp 文件中。 |
| 482 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 484 | `    if (meta.isProgressItem) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 485 | `        item.pass = item.flagReceived` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 486 | `                    && item.flagCheckOk` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 487 | `                    && item.flagValue == meta.progressFinishedValue;` | 赋值语句，将右侧结果保存到左侧变量或成员中。 |
| 489 | `        if (!item.flagReceived) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 490 | `            item.message = "进度数据未收到";` | 赋值语句，将右侧结果保存到左侧变量或成员中。 |
| 491 | `        } else if (!item.flagCheckOk) {` | 前一个条件不成立时，继续判断新的条件。 |
| 492 | `            item.message = "进度数据校验失败";` | 赋值语句，将右侧结果保存到左侧变量或成员中。 |
| 493 | `        } else if (item.flagValue != meta.progressFinishedValue) {` | 前一个条件不成立时，继续判断新的条件。 |
| 494 | `            item.message = QString("检测未完成，当前进度=%1").arg(item.flagValue);` | 成员函数声明，具体实现在 cpp 文件中。 |
| 495 | `        } else {` | 前面条件均不成立时，执行该分支。 |
| 496 | `            item.message = "检测完成";` | 赋值语句，将右侧结果保存到左侧变量或成员中。 |
| 497 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 499 | `        item.rawValue = item.flagValue;` | 进度项没有 valueSeq，所以把 flagValue 也放到 rawValue 里便于显示。 |
| 500 | `        item.displayValue = item.flagValue;` | 进度项显示值直接使用进度值。 |
| 501 | `        return item;` | 返回 `item` 的值。 |
| 502 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 504 | `    const bool flagOk = item.flagReceived` | 开始计算 flag 是否合格：收到、校验通过且值为 0。 |
| 505 | `                        && item.flagCheckOk` | 代码结构行，用于组成当前函数、条件或表达式。 |
| 506 | `                        && item.flagValue == 0;` | 赋值语句，将右侧结果保存到左侧变量或成员中。 |
| 508 | `    bool valueOk = true;` | 默认认为 value 合格；如果该项没有 valueSeq，只看 flag。 |
| 510 | `    if (meta.valueSeq >= 0) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 511 | `        valueOk = item.valueReceived && item.valueCheckOk;` | 有 valueSeq 时，检测值必须收到且校验通过。 |
| 512 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 514 | `    item.pass = flagOk && valueOk;` | 普通检测项通过条件：flag 合格且 value 有效。 |
| 516 | `    if (!item.flagReceived) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 517 | `        item.message = "合格标志未收到";` | 设置失败原因：flag 未收到。 |
| 518 | `    } else if (!item.flagCheckOk) {` | 前一个条件不成立时，继续判断新的条件。 |
| 519 | `        item.message = "合格标志校验失败";` | 设置失败原因：flag 校验失败。 |
| 520 | `    } else if (item.flagValue != 0) {` | 前一个条件不成立时，继续判断新的条件。 |
| 521 | `        item.message = "检测项不合格";` | 设置失败原因：flag 值不是 0。 |
| 522 | `    } else if (meta.valueSeq >= 0 && !item.valueReceived) {` | 前一个条件不成立时，继续判断新的条件。 |
| 523 | `        item.message = "检测值未收到";` | 设置失败原因：value 未收到。 |
| 524 | `    } else if (meta.valueSeq >= 0 && !item.valueCheckOk) {` | 前一个条件不成立时，继续判断新的条件。 |
| 525 | `        item.message = "检测值校验失败";` | 设置失败原因：value 校验失败。 |
| 526 | `    } else {` | 前面条件均不成立时，执行该分支。 |
| 527 | `        item.message = "OK";` | 设置检测项消息为 OK。 |
| 528 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 530 | `    return item;` | 返回 `item` 的值。 |
| 531 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### bool DynamicProductContext::calcProductPass(const QVector<DynamicItemResult> &items) const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 533 | `bool DynamicProductContext::calcProductPass(const QVector<DynamicItemResult> &items) const` | 根据所有 item 结果计算当前产品总 PASS/NG。 |
| 534 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 535 | `    if (!m_finished) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 536 | `        return false;` | 返回 false，表示条件不满足、处理失败或结果不通过。 |
| 537 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 539 | `    for (const DynamicTestItemMeta &meta : m_itemMetas) {` | 循环语句，按条件重复执行循环体。 |
| 540 | `        if (!meta.affectsProductPass) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 541 | `            continue;` | 跳过本次循环剩余逻辑，进入下一次循环。 |
| 542 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 544 | `        for (const DynamicItemResult &item : items) {` | 循环语句，按条件重复执行循环体。 |
| 545 | `            if (item.itemNo == meta.itemNo) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 546 | `                if (!item.pass) {` | 条件判断；只有条件成立时才执行后续代码块。 |
| 547 | `                    return false;` | 返回 false，表示条件不满足、处理失败或结果不通过。 |
| 548 | `                }` | 代码块边界，用于开始或结束当前作用域。 |
| 549 | `                break;` | 跳出当前循环。 |
| 550 | `            }` | 代码块边界，用于开始或结束当前作用域。 |
| 551 | `        }` | 代码块边界，用于开始或结束当前作用域。 |
| 552 | `    }` | 代码块边界，用于开始或结束当前作用域。 |
| 554 | `    return true;` | 返回 true，表示处理成功、条件满足或结果通过。 |
| 555 | `}` | 代码块边界，用于开始或结束当前作用域。 |

### QString DynamicProductContext::currentTimeText() const

| 行号 | 代码 | 解释 |
|---:|---|---|
| 723 | `QString DynamicProductContext::currentTimeText() const` | 生成当前系统时间字符串。 |
| 724 | `{` | 代码块边界，用于开始或结束当前作用域。 |
| 725 | `    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");` | 返回 `QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")` 的值。 |
| 726 | `}` | 代码块边界，用于开始或结束当前作用域。 |

---

## 5. qDebug 调试部分说明

你要求 `qDebug` 调试内容可以跳过，因此以下函数没有展开逐行解释：

```cpp
clearActiveGroupDebugFrames()
recordActiveGroupFrame()
findLastDebugFrameBySeq()
debugFrameLine()
debugDumpNgItem()
debugDumpCurrentGroupFrames()
debugDumpProductNg()
```

它们的作用可以概括为：

```text
产品 NG 时，把当前 active groupCnt 里的原始报文、解析结果、flagSeq/valueSeq 对应关系打印出来，
用于确认是否存在报文丢失、校验失败、seed 不匹配、groupCnt 混组、valueSeq 配错等问题。
```

业务判定主流程不依赖这些 qDebug 输出，但 `recordActiveGroupFrame()` 会在处理帧时记录调试缓存。

---

## 6. 关键业务规则总结

### 6.1 只处理当前产品对应 rxId

```cpp
if (canId != m_channelConfig.rxId) {
    return false;
}
```

含义：

```text
当前 DynamicProductContext 只处理自己的接收 ID。
位置1、位置2各有一个 context，controller 会把同一帧分别交给两个 context 尝试处理。
```

---

### 6.2 seed 不匹配直接忽略

```cpp
if (!frame.seedOk) {
    return true;
}
```

含义：

```text
只接收本轮开始检测帧对应 seed 的回传数据。
旧数据、串扰数据、上一次测试残留数据不会进入缓存。
```

---

### 6.3 groupCnt 防混组

```cpp
m_values / m_received / m_checkOk 只保存同一个 activeGroupCnt 的数据。
```

含义：

```text
一个检测项的 flag 和 rec 必须来自同一轮 groupCnt。
一旦 groupCnt 变化，旧组如果没完成就清空；
旧组如果已经进度 100 且必要帧齐全，就锁定旧组作为最终结果。
```

---

### 6.4 进度未到 100 前，不缓存普通 flag/rec

```cpp
if (!m_finalDataMode) {
    return;
}
```

含义：

```text
DetectInProgressFlag 到 100 之前收到的数据都视为中间轮询数据。
只有进度到 100 后，后续 flag/rec 才作为最终结果缓存。
```

---

### 6.5 最终完成条件

当前代码有两种完成路径：

```text
路径1：
DetectInProgressFlag == 100
并且收到 seq=255 组校验帧
并且组校验通过
并且必要帧齐全

路径2：
DetectInProgressFlag == 100
并且必要帧齐全
并且检测到下一组 groupCnt 开始
```

路径2是为了解决现场“进度到 100 但可能没有收到组校验帧，流程不停止”的问题。

---

### 6.6 item.pass 判定

进度项：

```text
flagReceived == true
flagCheckOk == true
flagValue == progressFinishedValue
```

普通检测项：

```text
flagReceived == true
flagCheckOk == true
flagValue == 0
如果有 valueSeq，则 valueReceived == true 且 valueCheckOk == true
```

---

### 6.7 result.pass 判定

```text
m_finished 必须为 true；
所有 affectsProductPass=true 的 item 必须 pass=true；
否则产品总结果 NG。
```

---

## 7. 你后续调试时重点看这几个变量

```text
m_detectProgress      当前进度
m_finalDataMode       是否已经进入最终数据阶段
m_groupCnt            当前结果使用的 groupCnt
m_activeGroupCnt      当前缓存组 groupCnt
m_finished            是否已经锁定完成
m_reported            是否已上报
m_groupCheckReceived  是否收到组校验帧
m_groupCheckPassed    组校验是否通过
m_received[seq]       某个 seq 是否被缓存
m_checkOk[seq]        某个 seq 是否校验通过
m_values[seq]         某个 seq 的实际值
```

尤其是你之前关注的：

```text
valueSeq = 45
valueSeq = 49
```

要重点看：

```cpp
m_received[45]
m_checkOk[45]
m_values[45]

m_received[49]
m_checkOk[49]
m_values[49]
```

如果 CANAnalyzer 能看到，但界面显示未收到，通常说明：

```text
1. 没进入当前 rxId；
2. seedOk=false；
3. checkOk=false；
4. groupCnt 不属于最终 active group；
5. 收到时 m_finalDataMode 还没有 true；
6. 已经 m_finished，后续帧被忽略。
```
