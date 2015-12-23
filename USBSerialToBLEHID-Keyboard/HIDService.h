#ifndef __BLE_HID_SERVICE_H__
#define __BLE_HID_SERVICE_H__

#include "BLE.h"
/**
* @class Human Interface Device Service
* @brief BLE Human Interface Device Service. This service displays the Glucose measurement value represented as a 16bit Float format.<br>
* @Author: Marco.Hsu  
* @Email: marco.missyou@gmail.com  
*/

extern const uint8_t KeyboardReportMap[76];
        
class HIDService {
public:
    HIDService(BLEDevice &_ble, const uint8_t* key = &KeyboardReportMap[0]):
        ble(_ble),
        protocol_modeValue(1),  // Report Protocol Mode(1), Boot Protocol Mode(0)
        KeyboardMap(key),
        Protocol_Mode(GattCharacteristic::UUID_PROTOCOL_MODE_CHAR, &protocol_modeValue, 1, 1, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE|GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        ReportMap(GattCharacteristic::UUID_REPORT_MAP_CHAR, KeyboardMap.getPointer(), 76, sizeof(KeyboardMap), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        Report(GattCharacteristic::UUID_REPORT_CHAR, reportValue.getPointer(), 8, 8, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY|GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        HID_Information(GattCharacteristic::UUID_HID_INFORMATION_CHAR, hidInformation.getPointer(), 4, 4, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        HID_Control_Point(GattCharacteristic::UUID_HID_CONTROL_POINT_CHAR, &hidcontrolPointer, 1, 1, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE)
        {
            static bool serviceAdded = false; /* We should only ever need to add the heart rate service once. */
            if (serviceAdded) {
            return;
            }
            //Report.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_OPEN_LINK);
            GattCharacteristic *charTable[] = {&Protocol_Mode, &ReportMap, &Report, &HID_Information, &HID_Control_Point};
            GattService         HIDGattService(GattService::UUID_HUMAN_INTERFACE_DEVICE_SERVICE, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
            ble.addService(HIDGattService);
            serviceAdded = true;
            ble.onDataWritten(this, &HIDService::onDataWritten);
        }
public:
    void updateReport(uint8_t modifydata, uint8_t data) {
        reportValue.updateReportValue(modifydata, data);
        ble.updateCharacteristicValue(Report.getValueAttribute().getHandle(), reportValue.getPointer(), 8);
    }
    
    virtual void onDataWritten(const GattWriteCallbackParams *params) {
        if (params->handle == HID_Control_Point.getValueAttribute().getHandle()) {
            uint16_t bytesRead = params->len;
            if (bytesRead == 1) {
                memcpy(&hidcontrolPointer, params->data, bytesRead);
            }
        }
        if (params->handle == Report.getValueAttribute().getHandle()) {
            uint16_t bytesRead = params->len;
            if (bytesRead <= 4) {
                memcpy(&reportValue, params->data, bytesRead);
            }
        }
    }

private:
    struct ReportMapStructure{
            uint8_t KeyboardMap[76];
            ReportMapStructure(const uint8_t* data): KeyboardMap() {
            memcpy(&KeyboardMap[0], data, 76);
            }
            uint8_t     *getPointer(void) {
            return      KeyboardMap;
            }
    };

private:
   struct ReportStructure {
            // Initial setting report value
            ReportStructure(): reportValue() {
                uint8_t data= 0x00;
                updateReportValue(data, data);
            }
            
            void updateReportValue(uint8_t modifyKey, uint8_t data){
                memset(&reportValue[0], 0 ,8);
                memcpy(&reportValue[0], &modifyKey, 1);
                memcpy(&reportValue[2], &data, 1);
            }
        
            uint8_t     *getPointer(void) {
            return      reportValue;
            }

            uint8_t reportValue[8];
        };
        
private:
    struct HIDInforStructure{
            uint16_t    bcdHID;
            uint8_t     bCountryCode;
            uint8_t     Flags;
            
            HIDInforStructure():bcdHID(0),bCountryCode(0),Flags(0){
                    memcpy(&hidInformation[0], &bcdHID, 2);
                    memcpy(&hidInformation[2], &bCountryCode, 1);
                    memcpy(&hidInformation[3], &Flags, 1);
                }
            uint8_t     *getPointer(void) {
            return      hidInformation;
            }
            
            uint8_t hidInformation[4];
        };
        
private:
    BLEDevice           &ble;
    uint8_t             protocol_modeValue;
    ReportStructure     reportValue;
    uint8_t             hidcontrolPointer;
    ReportMapStructure  KeyboardMap;
    HIDInforStructure   hidInformation;
    GattCharacteristic      Protocol_Mode;
    GattCharacteristic      ReportMap;
    GattCharacteristic      Report;
//    ReadOnlyGattCharacteristic         Boot_Keyboard_Input_Report;
//    ReadWriteGattCharacteristic        Boot_Keyboard_Output_Report;
//    ReadOnlyGattCharacteristic         Boot_Mouse_Input_Report;
    GattCharacteristic      HID_Information;
    GattCharacteristic      HID_Control_Point;
};
#endif /* #ifndef __BLE_GLUCOSE_SERVICE_H__*/