# ECPS 静态阻容与动态检测一体化上位机代码说明文档

## 1. 项目概述

本项目为 ECPS 静态阻容测试与离线动态检测合并后的 Qt 上位机工程，目标是在同一套软件中完成产品条码录入、PLC 点位控制、阻容测试、动态 CANFD 测试、手动调试、测试结果展示与 CSV 数据导出。

| 项目 | 说明 |
|---|---|
| 开发框架 | Qt 5.14.2 |
| 编译器 | MinGW 7.3 64-bit |
| 工程类型 | qmake `.pro` 工程 |
| 主要通信对象 | 三菱 PLC、万用表/电桥、ZLG USBCANFD-200U |
| 主要功能 | 静态阻容测试、双产品动态检测、手动调试、CSV 导出 |

---

## 2. 项目目录结构

```text
ECPS_StaticAndDynamic_Test/
├── ECPS_StaticAndDynamic_Test.pro
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── mainwindow.ui
├── test_runner.h
├── test_runner.cpp
├── manualstatictestwindow.h
├── manualstatictestwindow.cpp
├── manualstatictestwindow.ui
├── manualdynamictestwindow.h
├── manualdynamictestwindow.cpp
├── manualdynamictestwindow.ui
├── plctestwindow.h
├── plctestwindow.cpp
├── plctestwindow.ui
├── dynamic_test_controller.h
├── dynamic_test_controller.cpp
├── csv_exporter.h
├── csv_exporter.cpp
├── mitsubishi_plc.h
├── mitsubishi_plc.cpp
├── scpi_controller.h
├── scpi_controller.cpp
├── Device.h
├── Device.cpp
├── usbcanfd_200u.h
├── usbcanfd_200u.cpp
├── sensor.h
├── sensor.cpp
├── Singleton.h
├── DWLErrDetectCfg.h
├── UploadData.h
└── LIBS/
    └── zlgcan_x64/
```

---

## 3. 模块总览

| 模块 | 文件 | 主要职责 |
|---|---|---|
| 程序入口 | `main.cpp` | 初始化 Qt 应用、注册跨线程类型、启动主窗口 |
| 主界面 | `mainwindow.h/.cpp/.ui` | 设备连接、自动化流程、手动窗口入口、结果展示与 CSV 导出 |
| 静态测试执行器 | `test_runner.h/.cpp` | 自动阻容测试、M 点切换、电阻/电容测量、结果上报 |
| 手动阻容窗口 | `manualstatictestwindow.h/.cpp/.ui` | 单项阻容测试、测试项/方向选择、通道复位 |
| 手动动态窗口 | `manualdynamictestwindow.h/.cpp/.ui` | 手动双产品动态测试、二次确认、紧急关伺服 |
| PLC 点位窗口 | `plctestwindow.h/.cpp/.ui` | M 点、映射输出点、复位点位调试 |
| 动态测试控制器 | `dynamic_test_controller.h/.cpp` | 管理 CANFD 工作线程，封装双产品动态测试启动 |
| CSV 导出 | `csv_exporter.h/.cpp` | 阻容/动态测试结果统一导出 CSV |
| PLC 通信 | `mitsubishi_plc.h/.cpp` | 三菱 PLC 串口通信、M 点写入、脉冲复位 |
| 仪表通信 | `scpi_controller.h/.cpp` | SCPI 通信，测量电阻、电容 |
| ZLG 设备封装 | `Device.h/.cpp` | USBCANFD-200U 底层打开、初始化、收发帧 |
| 动态 CANFD 任务 | `usbcanfd_200u.h/.cpp` | CANFD 接收循环、创建 Sensor、发送复位/开始帧 |
| 动态数据解析 | `sensor.h/.cpp` | 解析动态测试 CAN 数据包，汇总 AllInfo 结果 |

---

## 4. PLC 点位与安全逻辑

### 4.1 当前最新点位约定

由于原项目中 `Y1~Y25` 实际通过 `setMRelay()` 正常控制，因此上位机侧应继续写 M 点，由 PLC 梯形图完成 M 点到实际 Y 输出的映射。

| 上位机写入点 | PLC 映射输出 | 功能 | OFF | ON |
|---|---|---|---|---|
| `M19` | 阻容测试总使能 | 阻容测试线路使能 | 关闭阻容使能 | 打开阻容使能 |
| `M20` | 测试位置选择 | 位置切换 | 位置1 | 位置2 |
| `M100` | 通道复位 | 阻容通道复位脉冲 | - | 脉冲复位 |
| `M26` | `Y26` | 测试线路切换 | 阻容测试线路 | 动态测试线路 |
| `M27` | `Y27` | 伺服电机控制 | 伺服关闭 | 伺服打开 |

如果 PLC 工程师最终确认不是 `M26/M27`，而是 `M226/M227` 或其它映射点，只需要统一修改常量，不要分散修改业务流程。

建议统一定义：

```cpp
constexpr int M_RES_CAP_ENABLE = 19;
constexpr int M_POSITION_SELECT = 20;
constexpr int M_RESET_CHANNEL = 100;
constexpr int M_DYNAMIC_MODE = 26;   // M26 -> Y26
constexpr int M_SERVO_POWER  = 27;   // M27 -> Y27
```

### 4.2 关键安全流程

进入阻容测试：

```text
M27 OFF 关闭伺服
M26 OFF 切换到阻容线路
M19 OFF 关闭阻容使能
M100 脉冲复位
开始阻容测试
```

进入动态测试：

```text
M19 OFF
M100 脉冲复位
M26 ON  切换到动态线路
M27 ON  打开伺服
等待 2s
启动双产品动态测试
```

退出动态测试：

```text
M27 OFF 关闭伺服
延时 300ms
M26 OFF 切回阻容线路
```

紧急关闭：

```text
立即 M27 OFF
```

---

## 5. 主界面模块说明

文件：

```text
mainwindow.h
mainwindow.cpp
mainwindow.ui
```

### 5.1 主界面职责

`MainWindow` 是项目主控制界面，主要负责：

1. 连接 PLC、万用表/电桥、动态 CANFD 模块；
2. 接收位置1、位置2产品编码；
3. 启动自动化测试流程；
4. 打开手动阻容测试窗口；
5. 打开手动动态测试窗口；
6. 打开 PLC 点位测试窗口；
7. 显示阻容测试结果；
8. 显示动态测试结果；
9. 保存阻容与动态 CSV；
10. 执行主界面紧急关闭伺服。

### 5.2 自动化测试流程

```text
扫码位置1产品编码
扫码位置2产品编码
点击自动化测试
    ↓
prepareStaticMode()
    ↓
启动 TestRunner 线程
    ↓
位置1阻容测试
位置2阻容测试
    ↓
保存阻容测试结果
    ↓
prepareDynamicMode()
    ↓
M26 ON
M27 ON
等待 2s
    ↓
DynamicTestController::startDynamicTest()
    ↓
双产品动态测试
    ↓
动态测试完成
    ↓
M27 OFF
延时 300ms
M26 OFF
    ↓
保存动态测试结果
```

### 5.3 独立窗口打开方式

手动窗口和 PLC 点位测试窗口必须作为独立顶层窗口打开，创建时不要传 `this` 作为父窗口。

推荐写法：

```cpp
auto *window = new PlcTestWindow(m_plc, nullptr);
window->setAttribute(Qt::WA_DeleteOnClose);
window->setWindowTitle("PLC点位测试");
window->show();
window->raise();
window->activateWindow();
```

如果创建时写成：

```cpp
new PlcTestWindow(m_plc, this);
```

该窗口会作为主界面的子控件显示在主界面内部，而不是弹出独立窗口。

---

## 6. 静态阻容测试模块说明

文件：

```text
test_runner.h
test_runner.cpp
```

### 6.1 模块职责

`TestRunner` 只负责阻容静态测试，不控制动态测试、不控制 CANFD、不保存 CSV。

主要职责包括：

1. 根据测试位置设置 `M20`；
2. 根据测试项设置对应 M 点通道；
3. 打开 `M19` 阻容测试使能；
4. 调用 SCPI 仪表测量电阻；
5. 调用 SCPI 仪表测量电容；
6. 判断电阻、电容是否在标准范围；
7. 发出测试结果信号；
8. 支持用户中断测试；
9. 测试结束执行 `M19 OFF + M100 脉冲复位`。

### 6.2 关键接口

单产品阻容测试：

```cpp
void runStaticTestsForProduct(int position,
                              const QString &productCode,
                              QList<int> selectedIndices);
```

双产品阻容测试：

```cpp
void runStaticTestsForTwoProducts(const QString &productCode1,
                                  const QString &productCode2,
                                  QList<int> selectedIndices);
```

中断测试：

```cpp
void requestInterrupt();
```

### 6.3 结果信号

兼容旧主界面 9 参数结果信号：

```cpp
void testCompleted(const QString &testName,
                   int position,
                   const QString &standardText,
                   double forwardResistanceKOhm,
                   double reverseResistanceKOhm,
                   double forwardCapacitanceNf,
                   double reverseCapacitanceNf,
                   bool pass,
                   const QString &message);
```

推荐用于 CSV 保存的新结果信号：

```cpp
void staticTestResultReady(const StaticTestResult &result);
```

`StaticTestResult` 包含产品编码、位置、测试项、方向、M 点、电阻、电容、结果、备注和时间戳。

### 6.4 阻容测试项

| 测试项 | 红黑方向 M 点 | 黑红方向 M 点 |
|---|---|---|
| `sin+ -> sin-` | `M3 + M6` | `M4 + M5` |
| `cos+ -> cos-` | `M7 + M12` | `M10 + M11` |
| `GND -> sin+` | `M1 + M4` | `M2 + M3` |
| `GND -> sin-` | `M1 + M6` | `M2 + M5` |
| `GND -> cos+` | `M1 + M10` | `M2 + M7` |
| `GND -> cos-` | `M1 + M12` | `M2 + M11` |
| `5V -> GND` | `M0 + M1` | `M0 + M2` |

`5V -> GND` 的黑红方向目前按 `M0 + M2` 编写，如现场点位不同，只需要修改 `initTestItems()`。

### 6.5 标准值说明

当前代码中的电阻、电容标准值为占位值，需要按真实工艺标准修改：

```cpp
windingStandard.resistanceMinKOhm = 10.0;
windingStandard.resistanceMaxKOhm = 20.0;
windingStandard.capacitanceMinNf = 1.0;
windingStandard.capacitanceMaxNf = 5.0;

insulationStandard.resistanceMinKOhm = 1000.0;
insulationStandard.resistanceMaxKOhm = 999999.0;
insulationStandard.capacitanceMinNf = 0.0;
insulationStandard.capacitanceMaxNf = 10.0;
```

---

## 7. 手动阻容测试窗口说明

文件：

```text
manualstatictestwindow.h
manualstatictestwindow.cpp
manualstatictestwindow.ui
```

手动阻容测试窗口用于调试单个测试项，支持选择测试位置、测试项、测试方向，并执行接通通道、测电阻、测电容、复位通道等动作。

窗口打开与关闭时建议执行：

```text
M27 OFF
M26 OFF
M19 OFF
M100 脉冲复位
```

这样可以保证伺服关闭、测试线路处于阻容模式、阻容通道处于复位状态。

---

## 8. 手动动态测试窗口说明

文件：

```text
manualdynamictestwindow.h
manualdynamictestwindow.cpp
manualdynamictestwindow.ui
```

手动动态测试窗口用于独立启动双产品动态测试，主要功能包括：

1. 显示位置1、位置2产品编码；
2. 启动动态测试前二次安全确认；
3. 控制动态线路切换；
4. 控制伺服开启；
5. 启动双产品动态测试；
6. 显示位置1、位置2动态测试结果；
7. 支持紧急关闭伺服；
8. 窗口关闭时自动关闭伺服。

启动前确认：

```text
确认产品已在台架正确安装
动态测试即将开始，电机即将启动
```

手动动态流程：

```text
M19 OFF
M100 脉冲复位
M26 ON
M27 ON
等待 2s
DynamicTestController::startDynamicTest(productCode1, productCode2)
收到 dynamicAllFinished
M27 OFF
延时 300ms
M26 OFF
```

---

## 9. PLC 点位测试窗口说明

文件：

```text
plctestwindow.h
plctestwindow.cpp
plctestwindow.ui
```

PLC 点位测试窗口用于现场调试 PLC 点位，主要支持：

1. M 点 ON；
2. M 点 OFF；
3. M 点脉冲；
4. 快捷选择 `M19`；
5. 快捷选择 `M20`；
6. 快捷选择 `M100`；
7. 快捷选择 `M26 -> Y26`；
8. 快捷选择 `M27 -> Y27`；
9. 显示点位动作日志。

当前建议将 `Y26/Y27` 理解为输出映射点测试：

```text
M26 ON/OFF 观察 Y26 是否动作
M27 ON/OFF 观察 Y27 是否动作
```

不建议直接调用 `setYRelay()`，除非 PLC 工程师确认上位机可以直接写 Y 点。

---

## 10. 动态测试控制器说明

文件：

```text
dynamic_test_controller.h
dynamic_test_controller.cpp
```

`DynamicTestController` 是上层 UI 与底层 CANFD 工作对象之间的中间层，职责包括：

1. 创建并启动 CANFD 工作线程；
2. 创建 `USBCanFd_200U` 工作对象；
3. 通过 queued signal 调用工作线程中的函数；
4. 启动双产品动态测试；
5. 转发动态测试日志；
6. 转发单产品完成信号；
7. 转发双产品完成信号；
8. 不控制 PLC；
9. 不保存 CSV。

关键接口：

```cpp
void startCanWorker();
void stopCanWorker();
void startDynamicTest(const QString &productCode1,
                      const QString &productCode2);
void requestStop();
```

默认使用新协议：

```text
位置1：00000114 + productCode1
位置2：00000116 + productCode2
```

兼容旧协议：

```cpp
m_dynamicController->setUseNewProtocol(false);
```

旧协议：

```text
位置1：00000110 + productCode1
位置2：00000112 + productCode2
```

---

## 11. CANFD 动态检测模块说明

文件：

```text
Device.h
Device.cpp
usbcanfd_200u.h
usbcanfd_200u.cpp
sensor.h
sensor.cpp
```

### 11.1 `Device`

`Device` 是 ZLG USBCANFD-200U 的底层封装，主要负责打开设备、初始化 CAN/CANFD 通道、启动通道、发送 CAN/CANFD 帧和关闭设备。

### 11.2 `USBCanFd_200U`

`USBCanFd_200U` 是动态测试工作对象，通常运行在单独线程中，主要负责：

1. 打开 CANFD 设备；
2. 启动接收循环；
3. 根据产品编码创建两个传感器对象；
4. 发送动态测试复位帧；
5. 发送动态测试开始帧；
6. 接收 CANFD 数据；
7. 将数据分发给对应 `Sensor`；
8. 收到产品完成后发出结果信号；
9. 两个产品都完成后发出总完成信号。

关键接口：

```cpp
void runn();
void requestStop();
void prepareTwoProducts(const QString &productCode1,
                        const QString &productCode2,
                        bool useNewProtocol = true);
void startCheckingTwoProducts();
```

### 11.3 `Sensor` 与 `AllInfo`

`Sensor` 负责解析动态测试报文，并把动态测试数据汇总到 `AllInfo`。

`AllInfo` 主要包含：

```text
BaseInfo
SpeedRecord
SignalRecord
TrackErrRecord
SinRecord
CosRecord
SinMaxMinRecord
CosMaxMinRecord
SoftInfo
```

动态完成后，结果通过 `std::shared_ptr<AllInfo>` 传递给主界面和 CSV 导出模块。

---

## 12. CSV 导出模块说明

文件：

```text
csv_exporter.h
csv_exporter.cpp
```

`CsvExporter` 用于统一导出 CSV 文件，支持：

1. 阻容测试结果导出；
2. 动态测试结果导出；
3. UTF-8 BOM；
4. CSV 字段转义；
5. 自动创建结果目录；
6. 默认文件名生成；
7. 数值格式化；
8. PASS/NG 统一格式。

阻容 CSV 导出接口：

```cpp
CsvExporter::exportStaticResults(staticResults, filePath, &errorMessage);
```

动态 CSV 导出接口：

```cpp
CsvExporter::exportDynamicResults(dynamicResults, filePath, &errorMessage);
```

动态结果结构：

```cpp
struct CsvDynamicResult
{
    int productId;
    QString productCode;
    bool pass;
    std::shared_ptr<AllInfo> allInfo;
    QString timestamp;
    QString message;
};
```

---

## 13. qmake 工程配置说明

`.pro` 文件需要包含以下内容：

```qmake
QT += core gui widgets serialport network

CONFIG += c++20

TEMPLATE = app
TARGET = ECPS_StaticAndDynamic_Test

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    test_runner.cpp \
    manualstatictestwindow.cpp \
    manualdynamictestwindow.cpp \
    plctestwindow.cpp \
    dynamic_test_controller.cpp \
    csv_exporter.cpp \
    mitsubishi_plc.cpp \
    scpi_controller.cpp \
    Device.cpp \
    usbcanfd_200u.cpp \
    sensor.cpp

HEADERS += \
    mainwindow.h \
    test_runner.h \
    manualstatictestwindow.h \
    manualdynamictestwindow.h \
    plctestwindow.h \
    dynamic_test_controller.h \
    csv_exporter.h \
    mitsubishi_plc.h \
    scpi_controller.h \
    Device.h \
    usbcanfd_200u.h \
    sensor.h \
    Singleton.h

FORMS += \
    mainwindow.ui \
    manualstatictestwindow.ui \
    manualdynamictestwindow.ui \
    plctestwindow.ui
```

调试阶段建议先注释掉：

```qmake
# UI_DIR =
# MOC_DIR =
# OBJECTS_DIR =
```

待工程正常编译后再整理输出目录。

---

## 14. Qt UI 文件注意事项

Qt 的 `uic` 生成规则：

```text
生成文件名：由 .ui 文件名决定
生成类名：由 .ui 文件内部 <class> 决定
```

必须保证：

| `.ui` 文件 | 内部 `<class>` | 顶层 widget name |
|---|---|---|
| `mainwindow.ui` | `MainWindow` | `MainWindow` |
| `manualstatictestwindow.ui` | `ManualStaticTestWindow` | `ManualStaticTestWindow` |
| `manualdynamictestwindow.ui` | `ManualDynamicTestWindow` | `ManualDynamicTestWindow` |
| `plctestwindow.ui` | `PlcTestWindow` | `PlcTestWindow` |

例如 `manualstatictestwindow.ui` 文件开头必须是：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>ManualStaticTestWindow</class>
 <widget class="QWidget" name="ManualStaticTestWindow">
```

如果 `manualstatictestwindow.ui` 里误写成动态窗口类名，则会生成文件 `ui_manualstatictestwindow.h`，但里面的类会是动态窗口类，导致编译错误。

---

## 15. 典型编译问题排查

### 15.1 `invalid use of incomplete type 'class Ui::ManualStaticTestWindow'`

常见原因：

1. `.cpp` 没有包含 `ui_manualstatictestwindow.h`；
2. `.ui` 内部 `<class>` 写错；
3. 生成的 `ui_manualstatictestwindow.h` 类名与 `.h/.cpp` 不一致；
4. 头文件保护宏冲突；
5. 编译的是旧路径文件。

排查命令：

```bat
dir /s /b ui_manualstatictestwindow.h
findstr /n "class Ui_" ui_manualstatictestwindow.h
findstr /n "namespace Ui" ui_manualstatictestwindow.h
```

### 15.2 `Ui::xxx has no member named yyy`

说明 `.cpp` 中使用的控件名与 `.ui` 中控件的 `objectName` 不一致。

处理方式：

1. 打开对应 `.ui`；
2. 检查控件 `objectName`；
3. 确认与 `.cpp` 中 `ui->xxx` 完全一致；
4. 执行 qmake；
5. 清理并重新构建。

### 15.3 `MitsubishiPLC has no member named setYRelay`

当前项目推荐通过 `setMRelay(26/27)` 控制 `Y26/Y27` 的 PLC 映射输出。

将：

```cpp
m_plc->setYRelay(26, true);
m_plc->setYRelay(27, false);
```

改为：

```cpp
m_plc->setMRelay(26, true);
m_plc->setMRelay(27, false);
```

### 15.4 手动窗口显示在主界面内部

原因：创建 `QWidget` 窗口时传入了 `this` 作为父控件。

错误写法：

```cpp
auto *window = new PlcTestWindow(m_plc, this);
```

正确写法：

```cpp
auto *window = new PlcTestWindow(m_plc, nullptr);
window->setAttribute(Qt::WA_DeleteOnClose);
window->show();
```

---

## 16. 线程与信号说明

### 16.1 阻容测试线程

`TestRunner` 运行在独立 `QThread` 中，避免阻塞主界面。

测试结果信号：

```cpp
staticTestResultReady(const StaticTestResult &result)
```

测试结束信号：

```cpp
allStaticTestsFinished(bool interrupted, bool hasFailed)
```

### 16.2 动态测试线程

`USBCanFd_200U` 运行在 `DynamicTestController` 创建的 CANFD 工作线程中。

动态结果信号：

```cpp
dynamicProductFinished(int productId,
                       bool pass,
                       std::shared_ptr<AllInfo> allInfo);
```

全部完成信号：

```cpp
dynamicAllFinished(bool allPass);
```

### 16.3 `main.cpp` 中的类型注册

由于多个信号跨线程传输，需要注册以下类型：

```cpp
qRegisterMetaType<StaticTestResult>("StaticTestResult");
qRegisterMetaType<std::shared_ptr<AllInfo>>("std::shared_ptr<AllInfo>");
qRegisterMetaType<ZCAN_Receive_Data>("ZCAN_Receive_Data");
qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
qRegisterMetaType<std::string>("std::string");
```

---

## 17. 当前代码需要重点确认的事项

### 17.1 PLC 映射点确认

需要 PLC 工程师确认：

```text
M26 是否映射 Y26？
M27 是否映射 Y27？
M26 OFF 是否为阻容线路？
M26 ON 是否为动态线路？
M27 OFF 是否为伺服关闭？
M27 ON 是否为伺服打开？
```

如映射不是 `M26/M27`，只修改统一常量即可。

### 17.2 阻容标准值确认

`test_runner.cpp` 和 `manualstatictestwindow.cpp` 中的标准值目前是占位范围，需替换为实际工艺要求。

### 17.3 仪表接口确认

代码中假设 `SCPIController` 提供：

```cpp
bool connectInstrument(const QString &visaAddress);
bool ping();
bool isConnected() const;
QString lastError() const;
double measureResistance(int timeoutMs);
double measureCapacitance(int timeoutMs);
```

如果现有类接口名称不同，需要在 `SCPIController` 中增加兼容函数，或修改调用处。

### 17.4 PLC 接口确认

代码中假设 `MitsubishiPLC` 提供：

```cpp
bool connectDevice(const QString &portName);
void disconnectDevice();
bool ping();
bool isConnected() const;
QString lastError() const;
bool setMRelay(int mNumber, bool on);
bool pulseMRelay(int mNumber, int pulseMs);
```

如原类函数名不同，需要增加兼容函数。

---

## 18. 推荐调试步骤

### 18.1 先调 PLC 点位

1. 打开主界面；
2. 连接 PLC；
3. 打开 PLC 点位测试窗口；
4. 测试 `M19`；
5. 测试 `M20`；
6. 测试 `M100`；
7. 测试 `M26` 是否切换 Y26；
8. 测试 `M27` 是否控制伺服 Y27。

### 18.2 再调手动阻容

1. 打开手动阻容窗口；
2. 选择位置1；
3. 选择一个测试项；
4. 接通通道；
5. 测电阻；
6. 测电容；
7. 复位通道；
8. 换位置2重复。

### 18.3 再调手动动态

1. 扫描位置1、位置2产品编码；
2. 打开手动动态窗口；
3. 确认产品安装；
4. 确认电机即将启动；
5. 观察 `M26/M27` 动作；
6. 观察 CANFD 是否接收动态结果；
7. 测试完成后确认 `M27 OFF`、`M26 OFF`。

### 18.4 最后调自动化流程

1. 连接所有设备；
2. 扫描两个产品编码；
3. 点击自动化测试；
4. 观察阻容流程；
5. 观察阻容 CSV；
6. 观察动态流程；
7. 观察动态 CSV；
8. 验证紧急关闭按钮。

---

## 19. 后续维护建议

1. 将 PLC 点位统一放入单独头文件，例如 `plc_points.h`；
2. 将阻容标准值从代码移入配置文件；
3. 将 CSV 导出完全替换到 `CsvExporter`；
4. 将自动化流程状态机化，避免流程分散在多个槽函数中；
5. 将动态协议编号 `110/112/114/116` 统一配置化；
6. 增加设备连接状态心跳；
7. 增加测试结果数据库或 MES 上传模块；
8. 增加异常断电/程序崩溃后的安全复位提示；
9. 增加操作权限，例如调试模式、生产模式；
10. 增加日志文件落盘。

---

## 20. 版本记录

| 版本 | 说明 |
|---|---|
| V1.0 | 完成静态阻容与动态检测一体化主流程代码结构 |
| V1.1 | 增加手动阻容、手动动态、PLC 点位测试窗口 |
| V1.2 | 增加双产品动态测试控制器 |
| V1.3 | 增加 CSV 导出模块 |
| V1.4 | 明确 `M26 -> Y26`、`M27 -> Y27` 的 PLC 映射控制方式 |
