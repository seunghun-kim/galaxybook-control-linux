#include "FirmwareAttribute.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QStringList>
#include <QIODevice>
#include <stdexcept>

const QString FirmwareAttribute::base_path = "/sys/class/firmware-attributes/samsung-galaxybook/attributes/";

FirmwareAttribute::FirmwareAttribute(const QString& attribute_name)
    : attribute_name_(attribute_name)
{
}

bool FirmwareAttribute::isSupported() const
{
    return QDir(base_path + attribute_name_).exists();
}

void FirmwareAttribute::set(int value)
{
    if (!isSupported()) {
        throw std::runtime_error("Attribute " + attribute_name_.toStdString() + " is not supported");
    }
    
    if (!isValidValue(value)) {
        throw std::runtime_error("Invalid value " + std::to_string(value) + " for attribute " + attribute_name_.toStdString());
    }
    
    QString filePath = base_path + attribute_name_ + "/current_value";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error("Failed to open file: " + filePath.toStdString());
    }
    QTextStream out(&file);
    out << value;
    file.close();
}

int FirmwareAttribute::get() const
{
    if (!isSupported()) {
        throw std::runtime_error("Attribute " + attribute_name_.toStdString() + " is not supported");
    }
    
    QString filePath = base_path + attribute_name_ + "/current_value";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Failed to open file: " + filePath.toStdString());
    }
    QTextStream in(&file);
    int value;
    in >> value;
    file.close();
    return value;
}

QVector<int> FirmwareAttribute::getSupportedValues() const
{
    QVector<int> supported_values;
    if (!isSupported()) {
        return supported_values;
    }
    
    QFile file(base_path + attribute_name_ + "/possible_values");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = file.readAll().trimmed();
        file.close();
        
        // Split the content by semicolon and convert to integers
        QStringList values = content.split(';', Qt::SkipEmptyParts);
        for (const QString& valueStr : values) {
            bool ok;
            int value = valueStr.toInt(&ok);
            if (ok) {
                supported_values.append(value);
            }
        }
    }
    return supported_values;
}

bool FirmwareAttribute::isValidValue(int value) const
{
    return getSupportedValues().contains(value);
}

QString FirmwareAttribute::getMonitoringFilePath() const
{
    return base_path + attribute_name_ + "/current_value";
}
