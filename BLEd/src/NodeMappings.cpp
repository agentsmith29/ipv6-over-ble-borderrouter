//
// Created by developer on 24.06.19.
//

#include "NodeMappings.h"
#include "ICommand.h"
#include "BLEHelper.h"
#include "Logger.h"


#include <mutex>



std::mutex lock_node_mapping_;

int auto_connect_to_hci = NODE_NO_MAPPING_INFORMATION;

typedef struct {
    HwAddress address;
    int hci_device;
} MappingInformation;


static vector<MappingInformation> mapping_informations_;

int NodeMappings::returnNodeMappingInfo(HwAddress address) {

    std::lock_guard<std::mutex> lock(lock_node_mapping_);

    for (int i = 0; i < mapping_informations_.size(); ++i) {
        if(addrComp(mapping_informations_[i].address, address))
            return mapping_informations_[i].hci_device;
    }
    if(auto_connect_to_hci >= 0)
        return auto_connect_to_hci;
    else
        return NODE_NO_MAPPING_INFORMATION; // No mapping Information, default is HCI0
}


int NodeMappings::addNodeMappingInfo(HwAddress address, int hci_dev) {

    std::lock_guard<std::mutex> lock(lock_node_mapping_);

    Logger::write({"Mapping device: ", address.getAddress(), " to hci" ,
                   std::to_string(hci_dev)}, INFO_MSG, BLE_LOG_FILE);

    MappingInformation tmp_info;
    tmp_info.address.setAddress(address.getAddress(), address.getAddressType());
    tmp_info.hci_device = hci_dev;

    mapping_informations_.push_back(tmp_info);

    return 0;
}

int NodeMappings::changeNodeMappingInfo(HwAddress address, int hci_dev) {
    std::lock_guard<std::mutex> lock(lock_node_mapping_);



    for (int i = 0; i < mapping_informations_.size(); ++i) {
        if(addrComp(mapping_informations_[i].address, address)){

            // Remap the device
            mapping_informations_[i].hci_device = hci_dev;

            Logger::write({"Remapping device: ", mapping_informations_[i].address.getAddress(), " to hci" ,
                           std::to_string(mapping_informations_[i].hci_device)}, DEBUG_MSG, BLE_LOG_FILE);
            return i;
        }

    }
    Logger::write({"New mapping information for node ", address.getAddress(), " -> hci", std::to_string(hci_dev)},
            INFO_MSG, BLE_LOG_FILE);

    MappingInformation tmp_info;
    tmp_info.address.setAddress(address.getAddress(), address.getAddressType());
    tmp_info.hci_device = hci_dev;

    mapping_informations_.push_back(tmp_info);

    return 0; // No mapping Information, default is HCI0

}


int NodeMappings::setAutoConnect(int hci_default_device){
    if(hci_default_device >= 0)
        auto_connect_to_hci = hci_default_device;
}


int NodeMappings::unsetAutoConnect(){
        auto_connect_to_hci = NODE_NO_MAPPING_INFORMATION;
}