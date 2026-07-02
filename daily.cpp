#include "daily_test_statistics.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

DailyTestStatistics::DailyTestStatistics()
{
}

void DailyTestStatistics::loadToday()
{
    /*
     * 如果你需要程序重启后继续读取当天历史记录，
     * 建议后面再单独做一个 CSV/JSON 数据源读取。
     *
     * 当前版本先以内存 m_records 为准，每次自动测试完成后重写当天统计文件。
     */
    m_records.clear();
}

void DailyTestStatistics::updateProduct(const QString &productCode,
                                        bool staticPass,
                                        bool dynamicPass,
                                        const QDateTime &time)
{
    const QString code = productCode.trimmed();

    if (code.isEmpty()) {
        return;
    }

    ProductDailyRecord record;
    record.time = time;
    record.productCode = code;
    record.staticPass = staticPass;
    record.dynamicPass = dynamicPass;
    record.finalPass = staticPass && dynamicPass;

    /*
     * 核心逻辑：
     * 同一条码重复测试，直接覆盖旧记录。
     * 所有计数都由 m_records 重新统计，不做简单累加。
     */
    m_records[code] = record;
}

DailyTestSummary DailyTestStatistics::summary() const
{
    DailyTestSummary s;

    for (auto it = m_records.constBegin(); it != m_records.constEnd(); ++it) {
        const ProductDailyRecord &r = it.value();

        s.totalCount++;

        if (r.finalPass) {
            s.passCount++;
        } else {
            s.ngCount++;
        }

        if (r.staticPass) {
            s.staticPassCount++;
        } else {
            s.staticNgCount++;
        }

        if (r.dynamicPass) {
            s.dynamicPassCount++;
        } else {
            s.dynamicNgCount++;
        }
    }

    return s;
}

QVector<ProductDailyRecord> DailyTestStatistics::records() const
{
    QVector<ProductDailyRecord> list;

    for (auto it = m_records.constBegin(); it != m_records.constEnd(); ++it) {
        list.append(it.value());
    }

    std::sort(list.begin(), list.end(), [](const ProductDailyRecord &a,
                                           const ProductDailyRecord &b) {
        return a.time < b.time;
    });

    return list;
}

QString DailyTestStatistics::makeMonthDirPath() const
{
    const QDate today = QDate::currentDate();

    return QString("D:/电涡流测试结果/%1年%2月")
            .arg(today.year())
            .arg(today.month(), 2, 10, QChar('0'));
}

QString DailyTestStatistics::todayFilePath() const
{
    const QDate today = QDate::currentDate();

    return QString("%1/%2测试结果.xls")
            .arg(makeMonthDirPath())
            .arg(today.toString("yyyyMMdd"));
}

QString DailyTestStatistics::escapeXml(const QString &text) const
{
    QString value = text;
    value.replace("&", "&amp;");
    value.replace("<", "&lt;");
    value.replace(">", "&gt;");
    value.replace("\"", "&quot;");
    value.replace("'", "&apos;");
    return value;
}

QString DailyTestStatistics::resultText(bool pass) const
{
    return pass ? "PASS" : "NG";
}

bool DailyTestStatistics::save(QString *errorMessage) const
{
    const QString dirPath = makeMonthDirPath();

    QDir dir;
    if (!dir.mkpath(dirPath)) {
        if (errorMessage) {
            *errorMessage = QString("创建统计目录失败：%1").arg(dirPath);
        }
        return false;
    }

    QFile file(todayFilePath());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("打开统计文件失败：%1").arg(file.errorString());
        }
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    const DailyTestSummary s = summary();
    const QVector<ProductDailyRecord> list = records();

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<?mso-application progid=\"Excel.Sheet\"?>\n";
    out << "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\n";
    out << " xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n";
    out << " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"\n";
    out << " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\">\n";

    /*
     * sheet1：当日测试情况
     */
    out << "<Worksheet ss:Name=\"当日测试情况\">\n";
    out << "<Table>\n";

    out << "<Row>"
        << "<Cell><Data ss:Type=\"String\">日期</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">总测试数</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">PASS数量</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">NG数量</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">阻容PASS数量</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">阻容NG数量</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">动态PASS数量</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">动态NG数量</Data></Cell>"
        << "</Row>\n";

    out << "<Row>"
        << "<Cell><Data ss:Type=\"String\">" << QDate::currentDate().toString("yyyy-MM-dd") << "</Data></Cell>"
        << "<Cell><Data ss:Type=\"Number\">" << s.totalCount << "</Data></Cell>"
        << "<Cell><Data ss:Type=\"Number\">" << s.passCount << "</Data></Cell>"
        << "<Cell><Data ss:Type=\"Number\">" << s.ngCount << "</Data></Cell>"
        << "<Cell><Data ss:Type=\"Number\">" << s.staticPassCount << "</Data></Cell>"
        << "<Cell><Data ss:Type=\"Number\">" << s.staticNgCount << "</Data></Cell>"
        << "<Cell><Data ss:Type=\"Number\">" << s.dynamicPassCount << "</Data></Cell>"
        << "<Cell><Data ss:Type=\"Number\">" << s.dynamicNgCount << "</Data></Cell>"
        << "</Row>\n";

    out << "</Table>\n";
    out << "</Worksheet>\n";

    /*
     * sheet2：当日测试产品信息
     */
    out << "<Worksheet ss:Name=\"当日测试产品信息\">\n";
    out << "<Table>\n";

    out << "<Row>"
        << "<Cell><Data ss:Type=\"String\">时间</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">产品编码</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">测试结果</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">阻容测试结果</Data></Cell>"
        << "<Cell><Data ss:Type=\"String\">动态测试结果</Data></Cell>"
        << "</Row>\n";

    for (const ProductDailyRecord &r : list) {
        out << "<Row>"
            << "<Cell><Data ss:Type=\"String\">" << r.time.toString("yyyy-MM-dd HH:mm:ss") << "</Data></Cell>"
            << "<Cell><Data ss:Type=\"String\">" << escapeXml(r.productCode) << "</Data></Cell>"
            << "<Cell><Data ss:Type=\"String\">" << resultText(r.finalPass) << "</Data></Cell>"
            << "<Cell><Data ss:Type=\"String\">" << resultText(r.staticPass) << "</Data></Cell>"
            << "<Cell><Data ss:Type=\"String\">" << resultText(r.dynamicPass) << "</Data></Cell>"
            << "</Row>\n";
    }

    out << "</Table>\n";
    out << "</Worksheet>\n";

    out << "</Workbook>\n";

    file.close();
    return true;
}
