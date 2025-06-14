#ifndef PERFORMANCEMODE_H
#define PERFORMANCEMODE_H

#include "UnsupportedFeatureException.h"

#include <QString>
#include <QStringList>

class PerformanceMode
{
public:
    static bool isSupported();
    static QStringList getSupportedPerformanceModes();
    static void setPerformanceMode(QString mode);
    static QString getPerformanceMode(); 
    static QString getMonitoringFilePath();

private:
    static const QString base_path;
};

#endif // PERFORMANCEMODE_H
