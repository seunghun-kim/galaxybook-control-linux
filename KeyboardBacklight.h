#ifndef KEYBOARDBACKLIGHT_H
#define KEYBOARDBACKLIGHT_H

#include <QString>
#include "UnsupportedFeatureException.h"

class KeyboardBacklight
{
public:
    static bool isSupported();
    static void setBrightness(int brightness_level);
    static int getBrightness();
    static int getMaxBrightness();
    
    // brightness_hw_changed 파일 관련 메서드들
    static bool isHwChangedMonitoringSupported();
    static QString getHwChangedFilePath();
    static QString getBrightnessFilePath();

private:
    static const QString base_path;
    
    KeyboardBacklight() = delete;
};

#endif // KEYBOARDBACKLIGHT_H
