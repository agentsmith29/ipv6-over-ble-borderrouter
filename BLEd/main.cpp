#include <iostream>
#include "src/Logger.h"
#include "src/BLEHelper.h"
#include "src/InteractionInterface.h"
#include "src/BLEHelper.h"
#include "src/NodeMappings.h"
#include "src/ConfigHandler.h"
#include "src/HCICore/HCIInterface.h"
#include "src/UtilityFunctions.h"

#include <sys/time.h>
#include <csignal>
#include <vector>
//#include "src/proto/node_message.pb.h"
//Actually a nuber for git

int exitDaemon();
int load6lowpanModule();

using std::vector;

std::vector<BLEHelper*> ble_helpers;

int main() {

    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    // GOOGLE_PROTOBUF_VERIFY_VERSION;

    // Open the log Files
    Logger::initLogger();

    // Load the kernel module
    load6lowpanModule();


    NodeMappings::setAutoConnect(0);
    ConfigHandler::loadDaemonConfigFile("/home/pi/clion/ipv6-over-ble-borderrouter/BLEd/configs/bled_config.conf");


    // Init the interface
    InteractionInterface interface_1 = InteractionInterface();


    // NodeMappings::addNodeMappingInfo(HwAddress("FE:8D:E4:E3:19:69", PUBLIC_ADDRESS), 1);
    // NodeMappings::addNodeMappingInfo(HwAddress("EC:A1:6C:F6:17:E0", PUBLIC_ADDRESS), 1);
    // NodeMappings::addNodeMappingInfo(HwAddress("E2:56:67:B1:ED:BB", PUBLIC_ADDRESS), 0);
    // NodeMappings::addNodeMappingInfo(HwAddress("D8:74:EA:0B:F1:ED", PUBLIC_ADDRESS), 0);


    // We create a new BLE-Helper for our HCI-Device

    //BLEHelper ble_helper_0 = BLEHelper(0);
    //BLEHelper ble_helper_1 = BLEHelper(1);


    // Addd the helper for later parsing
    vector<int> ble_helper_nos =  ConfigHandler::getHCINumbersToInit();

    for (int j = 0; j < ble_helper_nos.size(); ++j) {
        BLEHelper* ble_helper = new BLEHelper(ble_helper_nos[j]);
        ble_helpers.push_back(ble_helper);
    }

    // Test node
    while (true) {
        for (int i = 0; i < ble_helpers.size(); ++i) {

            BLEHelper ble_helper = *(ble_helpers[i]);
            ble_helper.scanAndConnect();
        }
        usleep(100000);
    }

    // End the deamon
    exitDaemon();
    return 0;
}

int appendHelperInstance(BLEHelper* helperForAppending){
    ble_helpers.push_back(helperForAppending);
    return 0;
}




int load6lowpanModule() {
    Logger::write("Kernel module: Load bluetooth_6lowpan",
                  INFO_MSG, BLE_LOG_FILE);

    // Enable the Kernel modul
    if (system("modprobe bluetooth_6lowpan") != 0) {
        Logger::write("Kernel module: Not loaded",
                      ERROR_MSG, BLE_LOG_FILE);
        return -1;
    }

    ofstream lowpan_control_file(KERNEL_MOD_ENABLE);
    // Check if the lowpan module has been loaded
    if (!lowpan_control_file.is_open()) {
        Logger::write({"Kernel Module", KERNEL_MOD_ENABLE,
                       " File error. Kernel module has not been enabled."},
                      FATAL_MSG, BLE_LOG_FILE);
        return -1;
    } else {
        Logger::write("Kernel module: Enable 6lowpan",
                      INFO_MSG, BLE_LOG_FILE);
        lowpan_control_file << "1" << std::endl;
        //TODO: Check if enabled!
    }
    lowpan_control_file.close();
    return 0;
}



int exitDaemon() {
    Logger::closeLogger();
    return 0;
}
