/* 
 *  Connect BLE Nano to PC via USB. After loading the hex file, follow instructins:
 *  1. Connect via (Mac) `screen /dev/tty.usbmodem1412 115200` with serial.
 *  2. Pair a device with HID_MOUSE.
 *  3. Use the passkey displayed in the serial terminal.
 *  4. After the connection is established, type two numbers, each with a space separated. Finish with enter to send the coordinates.
 *     Example: 100 <space> 100 <space> <enter>
 *     This will move the cursor by x=100, y=100 relatively.
 */

#include <string.h>
#include "mbed.h"
#include "BLE.h"
#include "DeviceInformationService.h"
#include "MouseService.h"

static const char     DEVICE_NAME[]        = "HID_Mouse";
static const uint16_t uuid16_list[]        = {GattService::UUID_HUMAN_INTERFACE_DEVICE_SERVICE};

#define TXRX_BUF_LEN                     200

BLEDevice  ble;
DigitalOut led1(LED1);
Serial pc(USBTX, USBRX);
//Serial pc(P0_9, P0_11); // BLE Nano pins

static char rx_buf[TXRX_BUF_LEN];
static int rx_len=0;

int x = -1;
int y = -1;

MouseService* mouse;

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
    while(pc.readable())
    {       
        char c = pc.getc();
        if(mouse->isConnected())
        {
            rx_buf[rx_len++] = c;
            pc.putc(c);
            
            if(c == 13 && x != -1 && y != -1)
            {
                pc.printf("Sending %d/%d\n", x, y);
                
                mouse->sendMouse(x, y, MOUSE_BUTTON_LEFT, BUTTON_DOWN);
                mouse->sendMouse(x, y, MOUSE_BUTTON_LEFT, BUTTON_UP);
                x = -1;
                y = -1;
                
                memset(rx_buf, 0, TXRX_BUF_LEN);
                rx_len = 0;
            } else if(c == 32)
            {
                if(x == -1)
                {
                    pc.printf("set x\n");
                    x = atoi(rx_buf);
                } else 
                {
                    pc.printf("set y\n");
                    y = atoi(rx_buf);
                }
                rx_len = 0;
                memset(rx_buf, 0, TXRX_BUF_LEN);
            }

        } else {
            pc.printf("mouse is not connected or too many characters typed\n");    
        }
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
    mouse = new MouseService(ble);
    
    ble.initializeSecurity(true, true, SecurityManager::IO_CAPS_DISPLAY_ONLY);  //IO_CAPS_DISPLAY_ONLY, IO_CAPS_NONE
    ble.onDisconnection(disconnectionCallback);
    ble.securityManager().onPasskeyDisplay(passkeyDisplayCallback);
    ble.securityManager().onSecuritySetupCompleted(securitySetupCompletedCallback);

    DeviceInformationService deviceInfo(ble, "ARM", "CYNTEC", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.accumulateAdvertisingPayload(GapAdvertisingData::MOUSE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);

    ble.setAdvertisingInterval(160); 
    ble.startAdvertising();
    
    while (true) {
        ble.waitForEvent();
    }
}
