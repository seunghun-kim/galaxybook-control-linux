#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "KeyboardBacklight.h"
#include "PerformanceMode.h"
#include "BatteryChargeControl.h"
#include <QFile>
#include <QFileSystemWatcher>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , fileWatcher(new QFileSystemWatcher(this))
    , power_on_lid_open("power_on_lid_open")
    , usb_charging("usb_charging")
    , block_recording("block_recording")
{
    ui->setupUi(this);

    // Connect fileWatcher signal (only once)
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, 
            this, &MainWindow::onFileChanged);

    bool atLeastOneUiSetup = false;
    atLeastOneUiSetup |= setupUiKeyboardBacklight();
    atLeastOneUiSetup |= setupUiPerformanceMode();
    atLeastOneUiSetup |= setupUiBatteryChargeEndThreshold();
    atLeastOneUiSetup |= setupUiPowerOnLidOpen();
    atLeastOneUiSetup |= setupUiUsbCharging();
    atLeastOneUiSetup |= setupUiBlockRecording();

    if (!atLeastOneUiSetup) {
        QMessageBox::warning(this, "No Features Supported",
            "No features are supported. Please use Linux kernel 6.15 or higher.\n\n"
            "If you are using kernel version below 6.15, please install https://github.com/joshuagrisham/samsung-galaxybook-extras manually.");
    }
}

bool MainWindow::setupUiKeyboardBacklight()
{
    if (KeyboardBacklight::isSupported()) {
        ui->hsliderKeyboardBacklight->setMaximum(KeyboardBacklight::getMaxBrightness());
        ui->hsliderKeyboardBacklight->setValue(KeyboardBacklight::getBrightness());
        ui->hsliderKeyboardBacklight->setTickPosition(QSlider::TicksAbove);
        ui->hsliderKeyboardBacklight->setTickInterval(1);
        ui->hsliderKeyboardBacklight->setSingleStep(1);
        connect(ui->hsliderKeyboardBacklight, &QSlider::valueChanged, this, &MainWindow::onHsliderKeyboardBacklightValueChanged);
        // Set up monitoring for brightness_hw_changed file
        if (KeyboardBacklight::isHwChangedMonitoringSupported()) {
            QString hwChangedPath = KeyboardBacklight::getHwChangedFilePath();
            
            fileWatcher->addPath(hwChangedPath);
        }
        return true;
    } else {
        ui->hsliderKeyboardBacklight->setEnabled(false);
        return false;
    }
}

bool MainWindow::setupUiPerformanceMode()
{
    if (PerformanceMode::isSupported()) {
        ui->comboPerformanceMode->addItems(PerformanceMode::getSupportedPerformanceModes());
        ui->comboPerformanceMode->setCurrentText(PerformanceMode::getPerformanceMode());
        connect(ui->comboPerformanceMode, &QComboBox::currentTextChanged, this, &MainWindow::onComboPerformanceModeCurrentTextChanged);

        QString monitoringFilePath = PerformanceMode::getMonitoringFilePath();
        if (!monitoringFilePath.isEmpty()) {
            fileWatcher->addPath(monitoringFilePath);
        }
        return true;
    } else {
        ui->comboPerformanceMode->setEnabled(false);
        return false;
    }
}

bool MainWindow::setupUiBatteryChargeEndThreshold()
{
    // Battery Charge End Threshold setup
    if (BatteryChargeControl::isSupported()) {
        ui->hsliderBatteryChargeEndThreshold->setMinimum(1);
        ui->hsliderBatteryChargeEndThreshold->setMaximum(100);
        ui->hsliderBatteryChargeEndThreshold->setValue(BatteryChargeControl::getChargeEndThreshold());
        ui->hsliderBatteryChargeEndThreshold->setTickPosition(QSlider::TicksBelow);
        ui->hsliderBatteryChargeEndThreshold->setTickInterval(10);
        ui->hsliderBatteryChargeEndThreshold->setSingleStep(1);
        connect(ui->hsliderBatteryChargeEndThreshold, &QSlider::valueChanged, this, &MainWindow::onHsliderBatteryChargeEndThresholdValueChanged);

        QString monitoringFilePath = BatteryChargeControl::getMonitoringFilePath();
        if (!monitoringFilePath.isEmpty()) {
            fileWatcher->addPath(monitoringFilePath);
        }
        return true;
    } else {
        ui->hsliderBatteryChargeEndThreshold->setEnabled(false);
        return false;
    }
}

bool MainWindow::setupUiPowerOnLidOpen()
{
    return setupUiFirmwareAttribute(power_on_lid_open, 
                                   ui->cboxPowerOnLidOpen,
                                   &MainWindow::onCboxPowerOnLidOpenStateChanged);
}

bool MainWindow::setupUiUsbCharging()
{
    return setupUiFirmwareAttribute(usb_charging, 
                                   ui->cboxUsbCharging,
                                   &MainWindow::onCboxUsbChargingStateChanged);
}

bool MainWindow::setupUiBlockRecording()
{
    return setupUiFirmwareAttribute(block_recording, 
                                   ui->cboxBlockRecording,
                                   &MainWindow::onCboxBlockRecordingStateChanged);
}

// Generic function to setup checkbox-based firmware attributes
bool MainWindow::setupUiFirmwareAttribute(FirmwareAttribute& attribute, 
                                         QCheckBox* checkbox, 
                                         void(MainWindow::*stateChangeSlot)(int))
{
    if (attribute.isSupported()) {
        checkbox->setEnabled(true);
        checkbox->setChecked(attribute.get());
        connect(checkbox, &QCheckBox::checkStateChanged, this, stateChangeSlot);

        QString monitoringFilePath = attribute.getMonitoringFilePath();
        if (!monitoringFilePath.isEmpty()) {
            fileWatcher->addPath(monitoringFilePath);
        }
        return true;
    } else {
        checkbox->setEnabled(false);
        return false;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onHsliderKeyboardBacklightValueChanged(int value)
{
    // Only set value when not updated from hardware
    KeyboardBacklight::setBrightness(value);
    ui->statusbar->showMessage("Keyboard backlight brightness set to " + QString::number(value));
}

void MainWindow::onComboPerformanceModeCurrentTextChanged(const QString &text)
{
    PerformanceMode::setPerformanceMode(text);
    ui->statusbar->showMessage("Performance mode set to " + text);
}

void MainWindow::onHsliderBatteryChargeEndThresholdValueChanged(int value)
{
    // Only set value when not updated from hardware
    BatteryChargeControl::setChargeEndThreshold(value);
    ui->statusbar->showMessage("Battery charge end threshold set to " + QString::number(value));
}

void MainWindow::onCboxPowerOnLidOpenStateChanged(int state)
{
    // Qt checkbox states: 0=Unchecked, 1=PartiallyChecked, 2=Checked
    // Convert to boolean: 0=false, non-zero=true
    int booleanValue = (state == Qt::Checked) ? 1 : 0;
    power_on_lid_open.set(booleanValue);
    ui->statusbar->showMessage("Power on lid open set to " + QString::number(booleanValue));
}

void MainWindow::onCboxUsbChargingStateChanged(int state)
{
    // Qt checkbox states: 0=Unchecked, 1=PartiallyChecked, 2=Checked
    // Convert to boolean: 0=false, non-zero=true
    int booleanValue = (state == Qt::Checked) ? 1 : 0;
    usb_charging.set(booleanValue);
    ui->statusbar->showMessage("Usb charging set to " + QString::number(booleanValue));
}

void MainWindow::onCboxBlockRecordingStateChanged(int state)
{
    // Qt checkbox states: 0=Unchecked, 1=PartiallyChecked, 2=Checked
    // Convert to boolean: 0=false, non-zero=true
    int booleanValue = (state == Qt::Checked) ? 1 : 0;
    block_recording.set(booleanValue);
    ui->statusbar->showMessage("Block recording set to " + QString::number(booleanValue));
}

void MainWindow::onFileChanged(const QString &path)
{
    // Determine which file was changed based on file path
    if (KeyboardBacklight::isHwChangedMonitoringSupported() && 
        path == KeyboardBacklight::getHwChangedFilePath()) {
        // Handle keyboard backlight changes
        ui->hsliderKeyboardBacklight->blockSignals(true);
        try {
            int currentBrightness = KeyboardBacklight::getBrightness();
            qDebug() << "onFileChanged - Keyboard brightness: " << currentBrightness;
            ui->hsliderKeyboardBacklight->setValue(currentBrightness);
            ui->statusbar->showMessage("Keyboard backlight changed to " + QString::number(currentBrightness));
        } catch (const std::exception& e) {
            qDebug() << "Error: " << __FUNCTION__ << " brightness " << e.what();
        }
        ui->hsliderKeyboardBacklight->blockSignals(false);
    }
    else if (PerformanceMode::isSupported() && 
             path == PerformanceMode::getMonitoringFilePath()) {
        // Handle performance mode changes
        ui->comboPerformanceMode->blockSignals(true);
        try {
            QString currentPerformanceMode = PerformanceMode::getPerformanceMode();
            ui->comboPerformanceMode->setCurrentText(currentPerformanceMode);
            ui->statusbar->showMessage("Performance mode changed to " + currentPerformanceMode);
        } catch (const std::exception& e) {
            qDebug() << "Error: " << __FUNCTION__ << " performance mode " << e.what();
        }
        ui->comboPerformanceMode->blockSignals(false);
    }
    else if (BatteryChargeControl::isSupported() && 
             path == BatteryChargeControl::getMonitoringFilePath()) {
        // Handle battery charge threshold changes
        ui->hsliderBatteryChargeEndThreshold->blockSignals(true);
        try {
            int currentThreshold = BatteryChargeControl::getChargeEndThreshold();
            ui->hsliderBatteryChargeEndThreshold->setValue(currentThreshold);
            ui->statusbar->showMessage("Battery charge end threshold changed to " + QString::number(currentThreshold));
        } catch (const std::exception& e) {
            qDebug() << "Error: " << __FUNCTION__ << " battery threshold " << e.what();
        }
        ui->hsliderBatteryChargeEndThreshold->blockSignals(false);
    }
    else if (power_on_lid_open.isSupported() && 
             path == power_on_lid_open.getMonitoringFilePath()) {
        handleFirmwareAttributeFileChanged(path, power_on_lid_open, ui->cboxPowerOnLidOpen, "Power on lid open");
    }
    else if (usb_charging.isSupported() && 
             path == usb_charging.getMonitoringFilePath()) {
        handleFirmwareAttributeFileChanged(path, usb_charging, ui->cboxUsbCharging, "USB charging");
    }
    else if (block_recording.isSupported() && 
             path == block_recording.getMonitoringFilePath()) {
        handleFirmwareAttributeFileChanged(path, block_recording, ui->cboxBlockRecording, "Block recording");
    }
    
    // Re-add file monitoring (file may be deleted and recreated on some systems)
    if (!fileWatcher->files().contains(path)) {
        fileWatcher->addPath(path);
    }
}

// Generic function to handle hardware changes for firmware attributes
void MainWindow::handleFirmwareAttributeFileChanged(const QString &path, 
                                                  FirmwareAttribute& attribute,
                                                  QCheckBox* checkbox,
                                                  const QString& featureName)
{
    Q_UNUSED(path);
    
    // Block signals to prevent infinite loop
    checkbox->blockSignals(true);
    
    try {
        int currentValue = attribute.get();
        checkbox->setChecked(currentValue != 0);
        ui->statusbar->showMessage(featureName + " changed to " + QString::number(currentValue));
    } catch (const std::exception& e) {
        qDebug() << "Error: " << __FUNCTION__ << " " << e.what();
    }
    
    // Unblock signals
    checkbox->blockSignals(false);
    
    // Re-add file monitoring (file may be deleted and recreated on some systems)
    if (!fileWatcher->files().contains(path)) {
        fileWatcher->addPath(path);
    }
}
