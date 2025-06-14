#include "KeyboardBacklight.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QIODevice>

const QString KeyboardBacklight::base_path = "/sys/class/leds/samsung-galaxybook::kbd_backlight";

bool KeyboardBacklight::isSupported()
{
    return QFile::exists(base_path);
}

void KeyboardBacklight::setBrightness(int brightness_level)
{
    if (!isSupported())
    {
        throw UnsupportedFeatureException("Keyboard backlight is not supported on this device");
    }
    QFile file(base_path + "/brightness");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << brightness_level;
        file.close();
    }
}

int KeyboardBacklight::getBrightness()
{
    if (!isSupported())
    {
        throw UnsupportedFeatureException("Keyboard backlight is not supported on this device");
    }
    QFile file(base_path + "/brightness");
    int brightness_level = 0;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in >> brightness_level;
        file.close();
    }
    return brightness_level;
}

int KeyboardBacklight::getMaxBrightness()
{
    if (!isSupported())
    {
        throw UnsupportedFeatureException("Keyboard backlight is not supported on this device");
    }
    QFile file(base_path + "/max_brightness");
    int max_brightness = 0;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in >> max_brightness;
        file.close();
    }
    return max_brightness;
}

bool KeyboardBacklight::isHwChangedMonitoringSupported()
{
    if (!isSupported()) {
        return false;
    }
    return QFile::exists(base_path + "/brightness_hw_changed");
}

QString KeyboardBacklight::getHwChangedFilePath()
{
    return base_path + "/brightness_hw_changed";
}

QString KeyboardBacklight::getBrightnessFilePath()
{
    return base_path + "/brightness";
}