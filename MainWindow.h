#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QCheckBox>
#include <memory>
#include <functional>
#include "FirmwareAttribute.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // User interface slots
    void onHsliderKeyboardBacklightValueChanged(int value);
    void onComboPerformanceModeCurrentTextChanged(const QString &text);
    void onHsliderBatteryChargeEndThresholdValueChanged(int value);
    void onCboxPowerOnLidOpenStateChanged(int state);
    void onCboxUsbChargingStateChanged(int state);
    void onCboxBlockRecordingStateChanged(int state);

    // File change monitoring slots
    void onFileChanged(const QString &path);

private:
    Ui::MainWindow *ui;
    std::unique_ptr<QFileSystemWatcher> fileWatcher;

    FirmwareAttribute power_on_lid_open;
    FirmwareAttribute usb_charging;
    FirmwareAttribute block_recording;

    bool setupUiKeyboardBacklight();
    bool setupUiPerformanceMode();
    bool setupUiBatteryChargeEndThreshold();
    bool setupUiPowerOnLidOpen();
    bool setupUiUsbCharging();
    bool setupUiBlockRecording();

    // Generic function to setup checkbox-based firmware attributes
    bool setupUiFirmwareAttribute(FirmwareAttribute& attribute, 
                                 QCheckBox& checkbox, 
                                 std::function<void(int)> stateChangeSlot);
    
    // Generic function to handle file changes for firmware attributes
    void handleFirmwareAttributeFileChanged(const QString &path, 
                                          FirmwareAttribute& attribute,
                                          QCheckBox& checkbox,
                                          const QString& featureName);
};
#endif // MAINWINDOW_H
