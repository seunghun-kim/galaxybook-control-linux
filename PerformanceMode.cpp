#include "PerformanceMode.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QIODevice>

const QString PerformanceMode::base_path = "/sys/firmware/acpi";

bool PerformanceMode::isSupported() {
    return QFile::exists(base_path + "/platform_profile");
}

void PerformanceMode::setPerformanceMode(QString mode) {
    if (!isSupported()) {
        throw UnsupportedFeatureException("Performance mode is not supported");
    }

    QStringList supportedModes = getSupportedPerformanceModes();
    if (!supportedModes.contains(mode)) {
        throw UnsupportedFeatureException("Performance mode " + mode + " is not supported");
    }

    QFile file(base_path + "/platform_profile");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << mode;
        file.close();
    }
}

QString PerformanceMode::getPerformanceMode() {
    if (!isSupported()) {
        throw UnsupportedFeatureException("Performance mode is not supported");
    }

    QFile file(base_path + "/platform_profile");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString mode = in.readAll().trimmed();
        file.close();
        return mode;
    }
    return "";
}

QStringList PerformanceMode::getSupportedPerformanceModes() {
    if (!isSupported()) {
        throw UnsupportedFeatureException("Performance mode is not supported");
    }

    QStringList modes;
    QFile file(base_path + "/platform_profile_choices");  // file content example: low-power quiet balanced performance
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll().trimmed();
        if (!content.isEmpty()) {
            modes = content.split(' ', Qt::SkipEmptyParts);
        }
    }
    return modes;
}

QString PerformanceMode::getMonitoringFilePath() {
    return base_path + "/platform_profile";
}

