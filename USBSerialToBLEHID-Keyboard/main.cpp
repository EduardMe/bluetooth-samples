/* 
 *  Connect BLE Nano to PC via USB. After loading the hex file, follow instructins:
 *  1. Connect via (Mac) `screen /dev/tty.usbmodem1412 115200` with serial.
 *  2. Pair a device with HID_KEYBOARD.
 *  3. Use the passkey displayed in the serial terminal.
 *  4. After the connection is established, open a texteditor on the connected host and type any 
 *     character into the serial terminal. This will create keyboard input events on the host machine.
 *     The same characters should appear there.
 */
#include <string.h>
#include "mbed.h"
#include "BLE.h"
#include "DeviceInformationService.h"
#include "KeyboardService.h"

static const char     DEVICE_NAME[]        = "HID_Keyboard";
static const uint16_t uuid16_list[]        = {GattService::UUID_HUMAN_INTERFACE_DEVICE_SERVICE};

BLEDevice  ble;
DigitalOut led1(LED1);
Serial pc(USBTX, USBRX);
//Serial pc(P0_9, P0_11); // BLE Nano pins

KeyboardService* keyboard;

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    ble.startAdvertising();
}

void passkeyDisplayCallback(Gap::Handle_t handle, const SecurityManager::Passkey_t passkey)
{
    pc.printf("Input passKey: ");
    for (unsigned i = 0; i < Gap::ADDR_LEN; i++) {
        printf("%c ", passkey[i]);
    }
    printf("\r\n");
}
 
void securitySetupCompletedCallback(Gap::Handle_t handle, SecurityManager::SecurityCompletionStatus_t status)
{
    if (status == SecurityManager::SEC_STATUS_SUCCESS) {
        pc.printf("Security success\r\n", status);
    } else {
        pc.printf("Security failed\r\n", status);
    }
}

void serialReceived() 
{
    if(keyboard->isConnected())
    {
        int re = keyboard->putc(pc.getc());
    } else {
        pc.printf("Keyboard is not connected\n");    
    }
}

int main(void)
{
    led1 = 1;

    pc.baud(115200);
    pc.format(8, SerialBase::None, 1);
    
    pc.attach(&serialReceived);
    pc.printf("START\n");
    ble.init();
    keyboard = new KeyboardService(ble);
    
    ble.initializeSecurity(true, true, SecurityManager::IO_CAPS_DISPLAY_ONLY);  //IO_CAPS_DISPLAY_ONLY, IO_CAPS_NONE
    ble.onDisconnection(disconnectionCallback);
    ble.securityManager().onPasskeyDisplay(passkeyDisplayCallback);
    ble.securityManager().onSecuritySetupCompleted(securitySetupCompletedCallback);

    DeviceInformationService deviceInfo(ble, "ARM", "CYNTEC", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.accumulateAdvertisingPayload(GapAdvertisingData::KEYBOARD);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);

    ble.setAdvertisingInterval(160); 
    ble.startAdvertising();
    
    while (true) {
        ble.waitForEvent();
    }
}
