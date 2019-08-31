//
// Created by developer on 24.06.19.
// A class for storing the mapping information
// The daemon can work with multiple HCHi Devices. If you add a node to this class, the BLE-Helper of the desired
// device will check if an node has an entry and if so, it will try to connect it.
//

#ifndef BLED_BLECONFIG_H
#define BLED_BLECONFIG_H

#include "HwAddress.h"
#include <vector>

using std::vector;

static constexpr int NODE_NO_MAPPING_INFORMATION = -1;
static constexpr int NODE_DISABLED = -2;

class NodeMappings {

public:

    /// Returns the hci device to which the node should be assigned
    /// \param address Node's address to get the hci device
    /// \return
    static int returnNodeMappingInfo(HwAddress address);

    /// Add a new mapping information entry for the node.
    /// If a node with the given address is discoverd it will be connected t the spcified hci_device
    /// \param address
    /// \param hci_dev
    /// \return
    static int addNodeMappingInfo(HwAddress address, int hci_dev);

    /// Change an node's mapping information to the given hci_device.
    /// \param address
    /// \param hci_dev
    /// \return
    static int changeNodeMappingInfo(HwAddress address, int hci_dev);

    // Delete the copy constructor
    // deleted copy constructor
    //static NodeMappings(const NodeMappings&) = delete;
    // deleted copy assignment operator
    //static NodeMappings& operator = (const NodeMappings&) = delete;

    /// Set the mapping information to an default device
    /// \param hci_default_device
    /// \return
    static int setAutoConnect(int hci_default_device);

    /// Set the mapping information to auto connect
    /// \return
    static int unsetAutoConnect();



};


#endif //BLED_BLECONFIG_H
