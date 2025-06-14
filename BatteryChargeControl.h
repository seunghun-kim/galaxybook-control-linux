#ifndef BATTERYCHARGECONTROL_H
#define BATTERYCHARGECONTROL_H

#include <QString>
#include <QList>
#include "UnsupportedFeatureException.h"

class BatteryChargeControl
{
public:
    static bool isSupported();
    static void setChargeEndThreshold(int threshold);
    static int getChargeEndThreshold();
    static QList<int> getRecommendedThresholds();
    static QString getMonitoringFilePath();

private:
    static const QString base_path;
    static const QList<int> recommended_thresholds;
    
    BatteryChargeControl() = delete;
};

#endif // BATTERYCHARGECONTROL_H 