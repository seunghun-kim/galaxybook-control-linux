#include "BatteryChargeControl.h"
#include <QFile>
#include <QTextStream>
#include <QIODevice>

const QString BatteryChargeControl::base_path = "/sys/class/power_supply/BAT1";
const QList<int> BatteryChargeControl::recommended_thresholds = {50, 60, 70, 80, 90, 100};

bool BatteryChargeControl::isSupported()
{
    return QFile::exists(base_path + "/charge_control_end_threshold");
}

void BatteryChargeControl::setChargeEndThreshold(int threshold)
{
    if (!isSupported())
    {
        throw UnsupportedFeatureException("Battery charge control is not supported on this device");
    }
    
    // Validate value range (1-100)
    if (threshold < 1 || threshold > 100)
    {
        throw UnsupportedFeatureException("Charge end threshold must be between 1 and 100");
    }
    
    QFile file(base_path + "/charge_control_end_threshold");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << threshold;
        file.close();
    }
}

int BatteryChargeControl::getChargeEndThreshold()
{
    if (!isSupported())
    {
        throw UnsupportedFeatureException("Battery charge control is not supported on this device");
    }
    
    QFile file(base_path + "/charge_control_end_threshold");
    int threshold = 100; // Default value
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in >> threshold;
        file.close();
    }
    return threshold;
}

QList<int> BatteryChargeControl::getRecommendedThresholds()
{
    return recommended_thresholds;
}

QString BatteryChargeControl::getMonitoringFilePath()
{
    return base_path + "/charge_control_end_threshold";
}