#ifndef FIRMWAREATTRIBUTE_H
#define FIRMWAREATTRIBUTE_H

#include <QString>
#include <QVector>

class FirmwareAttribute
{
public:
    explicit FirmwareAttribute(const QString& attribute_name);

    bool isSupported() const;
    void set(int value);
    int get() const;
    QVector<int> getSupportedValues() const;
    bool isValidValue(int value) const;
    QString getMonitoringFilePath() const;
    
    // Attribute name getter
    QString getAttributeName() const { return attribute_name_; }

private:
    QString attribute_name_;
    static const QString base_path;
};

#endif // FIRMWAREATTRIBUTE_H
