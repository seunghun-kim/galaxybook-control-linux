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
    , fileWatcher(nullptr)
    , power_on_lid_open("power_on_lid_open")
    , usb_charging("usb_charging")
    , block_recording("block_recording")
{
    ui->setupUi(this);

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
        // brightness_hw_changed 파일 모니터링 설정
        if (KeyboardBacklight::isHwChangedMonitoringSupported()) {
            fileWatcher = new QFileSystemWatcher(this);
            QString hwChangedPath = KeyboardBacklight::getHwChangedFilePath();
            
            fileWatcher->addPath(hwChangedPath);
            connect(fileWatcher, &QFileSystemWatcher::fileChanged, 
                    this, &MainWindow::onBrightnessFileChanged);
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
            fileWatcher = new QFileSystemWatcher(this);
            fileWatcher->addPath(monitoringFilePath);
            connect(fileWatcher, &QFileSystemWatcher::fileChanged, 
                    this, &MainWindow::onPerformanceModeFileChanged);
        }
        return true;
    } else {
        ui->comboPerformanceMode->setEnabled(false);
        return false;
    }
}

bool MainWindow::setupUiBatteryChargeEndThreshold()
{
    // Battery Charge End Threshold 설정
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
            fileWatcher = new QFileSystemWatcher(this);
            fileWatcher->addPath(monitoringFilePath);
            connect(fileWatcher, &QFileSystemWatcher::fileChanged, 
                    this, &MainWindow::onBatteryChargeEndThresholdFileChanged);
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
                                   &MainWindow::onCboxPowerOnLidOpenStateChanged,
                                   &MainWindow::onPowerOnLidOpenFileChanged);
}

bool MainWindow::setupUiUsbCharging()
{
    return setupUiFirmwareAttribute(usb_charging, 
                                   ui->cboxUsbCharging,
                                   &MainWindow::onCboxUsbChargingStateChanged,
                                   &MainWindow::onUsbChargingFileChanged);
}

bool MainWindow::setupUiBlockRecording()
{
    return setupUiFirmwareAttribute(block_recording, 
                                   ui->cboxBlockRecording,
                                   &MainWindow::onCboxBlockRecordingStateChanged,
                                   &MainWindow::onBlockRecordingFileChanged);
}

// Generic function to setup checkbox-based firmware attributes
bool MainWindow::setupUiFirmwareAttribute(FirmwareAttribute& attribute, 
                                         QCheckBox* checkbox, 
                                         void(MainWindow::*stateChangeSlot)(int),
                                         void(MainWindow::*fileChangeSlot)(const QString&))
{
    if (attribute.isSupported()) {
        checkbox->setEnabled(true);
        checkbox->setChecked(attribute.get());
        connect(checkbox, &QCheckBox::checkStateChanged, this, stateChangeSlot);

        QString monitoringFilePath = attribute.getMonitoringFilePath();
        if (!monitoringFilePath.isEmpty()) {
            if (!fileWatcher) {
                fileWatcher = new QFileSystemWatcher(this);
            }
            fileWatcher->addPath(monitoringFilePath);
            connect(fileWatcher, &QFileSystemWatcher::fileChanged, 
                    this, fileChangeSlot);
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
    // 하드웨어에서 업데이트된 경우가 아닐 때만 값 설정
    KeyboardBacklight::setBrightness(value);
    ui->statusbar->showMessage("Keyboard backlight brightness set to " + QString::number(value));
}

void MainWindow::onBrightnessFileChanged(const QString & path)
{
    Q_UNUSED(path);
    
    // 시그널을 차단하여 무한 루프 방지
    ui->hsliderKeyboardBacklight->blockSignals(true);
    
    try {
        // 현재 밝기 값을 읽어서 슬라이더 업데이트
        int currentBrightness = KeyboardBacklight::getBrightness();
        qDebug() << "onBrightnessFileChanged: " << currentBrightness;
        ui->hsliderKeyboardBacklight->setValue(currentBrightness);
        ui->statusbar->showMessage("Keyboard backlight changed to " + QString::number(currentBrightness));
    } catch (const std::exception& e) {
        qDebug() << "Error: " << __FUNCTION__ << " " << e.what();
    }
    
    // 시그널 차단 해제
    ui->hsliderKeyboardBacklight->blockSignals(false);
    
    // 파일 감시를 다시 추가 (일부 시스템에서 파일이 삭제되고 재생성될 수 있음)
    if (fileWatcher && !fileWatcher->files().contains(path)) {
        if (KeyboardBacklight::isHwChangedMonitoringSupported()) {
            fileWatcher->addPath(path);
        }
    }
}

void MainWindow::onComboPerformanceModeCurrentTextChanged(const QString &text)
{
    PerformanceMode::setPerformanceMode(text);
    ui->statusbar->showMessage("Performance mode set to " + text);
}

void MainWindow::onPerformanceModeFileChanged(const QString &path)
{
    Q_UNUSED(path);
    
    // 시그널을 차단하여 무한 루프 방지
    ui->comboPerformanceMode->blockSignals(true);
    
    try {
        QString currentPerformanceMode = PerformanceMode::getPerformanceMode();
        ui->comboPerformanceMode->setCurrentText(currentPerformanceMode);
        ui->statusbar->showMessage("Performance mode changed to " + currentPerformanceMode);
    } catch (const std::exception& e) {
        qDebug() << "Error: " << __FUNCTION__ << " " << e.what();
    }
    
    // 시그널 차단 해제
    ui->comboPerformanceMode->blockSignals(false);
    
    if (fileWatcher && !fileWatcher->files().contains(path)) {
        fileWatcher->addPath(path);
    }
}

void MainWindow::onBatteryChargeEndThresholdFileChanged(const QString &path)
{
    Q_UNUSED(path);
    
    // 시그널을 차단하여 무한 루프 방지
    ui->hsliderBatteryChargeEndThreshold->blockSignals(true);
    
    try {
        // 현재 밝기 값을 읽어서 슬라이더 업데이트
        int currentThreshold = BatteryChargeControl::getChargeEndThreshold();
        ui->hsliderBatteryChargeEndThreshold->setValue(currentThreshold);
        ui->statusbar->showMessage("Battery charge end threshold changed to " + QString::number(currentThreshold));
    } catch (const std::exception& e) {
        qDebug() << "Error: " << __FUNCTION__ << " " << e.what();
    }
    
    // 시그널 차단 해제
    ui->hsliderBatteryChargeEndThreshold->blockSignals(false);
    
    // 파일 감시를 다시 추가 (일부 시스템에서 파일이 삭제되고 재생성될 수 있음)
    if (fileWatcher && !fileWatcher->files().contains(path)) {
        fileWatcher->addPath(path);
    }
}
void MainWindow::onHsliderBatteryChargeEndThresholdValueChanged(int value)
{
    // 하드웨어에서 업데이트된 경우가 아닐 때만 값 설정
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

void MainWindow::onPowerOnLidOpenFileChanged(const QString &path)
{
    handleFirmwareAttributeFileChanged(path, power_on_lid_open, ui->cboxPowerOnLidOpen, "Power on lid open");
}

void MainWindow::onUsbChargingFileChanged(const QString &path)
{
    handleFirmwareAttributeFileChanged(path, usb_charging, ui->cboxUsbCharging, "USB charging");
}

void MainWindow::onBlockRecordingFileChanged(const QString &path)
{
    handleFirmwareAttributeFileChanged(path, block_recording, ui->cboxBlockRecording, "Block recording");
}

// Generic function to handle hardware changes for firmware attributes
void MainWindow::handleFirmwareAttributeFileChanged(const QString &path, 
                                                  FirmwareAttribute& attribute,
                                                  QCheckBox* checkbox,
                                                  const QString& featureName)
{
    Q_UNUSED(path);
    
    // 시그널을 차단하여 무한 루프 방지
    checkbox->blockSignals(true);
    
    try {
        int currentValue = attribute.get();
        checkbox->setChecked(currentValue != 0);
        ui->statusbar->showMessage(featureName + " changed to " + QString::number(currentValue));
    } catch (const std::exception& e) {
        qDebug() << "Error: " << __FUNCTION__ << " " << e.what();
    }
    
    // 시그널 차단 해제
    checkbox->blockSignals(false);
    
    // 파일 감시를 다시 추가 (일부 시스템에서 파일이 삭제되고 재생성될 수 있음)
    if (fileWatcher && !fileWatcher->files().contains(path)) {
        fileWatcher->addPath(path);
    }
}
