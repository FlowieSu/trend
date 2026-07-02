#ifndef DAILY_TEST_STATISTICS_H
#define DAILY_TEST_STATISTICS_H

#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVector>

struct ProductDailyRecord
{
    QDateTime time;
    QString productCode;

    bool finalPass = false;
    bool staticPass = false;
    bool dynamicPass = false;
};

struct DailyTestSummary
{
    int totalCount = 0;

    int passCount = 0;
    int ngCount = 0;

    int staticPassCount = 0;
    int staticNgCount = 0;

    int dynamicPassCount = 0;
    int dynamicNgCount = 0;
};

class DailyTestStatistics
{
public:
    explicit DailyTestStatistics();

    void loadToday();
    void updateProduct(const QString &productCode,
                       bool staticPass,
                       bool dynamicPass,
                       const QDateTime &time = QDateTime::currentDateTime());

    DailyTestSummary summary() const;
    QVector<ProductDailyRecord> records() const;

    bool save(QString *errorMessage = nullptr) const;

    QString todayFilePath() const;

private:
    QString makeMonthDirPath() const;
    QString escapeXml(const QString &text) const;
    QString resultText(bool pass) const;

private:
    QMap<QString, ProductDailyRecord> m_records;
};

#endif // DAILY_TEST_STATISTICS_H
