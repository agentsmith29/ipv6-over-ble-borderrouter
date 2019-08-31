//
// Created by developer on 06.06.19.
//

#ifndef BLED_HCITOOL_H
#define BLED_HCITOOL_H

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stack>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <mutex>
#include <stack>
#include <queue>

#include "../Node.h"

extern "C"{
    #include "bluetooth.h"
    #include "hci.h"
    #include "hci_lib.h"
}


static const int ERR_HCITOOL_OPENING_DEVICE =-1;
static const int ERR_HCITOOL_SETTING_SCAN_PARAMS =-2;
static const int ERR_HCITOOL_ENABLE_SCAN_FAILED =-3;
static const int ERR_HCITOOL_DISABLE_SCAN_FAILED =-4;
static const int ERR_HCITOOL_NO_ADVERTISING_REPORT =-5;

#define FLAGS_AD_TYPE               0x01
#define FLAGS_LIMITED_MODE_BIT      0x01
#define FLAGS_GENERAL_MODE_BIT      0x02
#define EIR_NAME_SHORT              0x08  /* shortened local name */
#define EIR_NAME_COMPLETE           0x09  /* complete local name */

using std::vector;
using std::string;
using std::stack;
using std::queue;

using std::initializer_list;

class HciInterface {

public:

    HciInterface(int hci_dev_id);


    HciInterface(string hci_dev_addr);


    int scan(char opt);


    /// Initializes the scan
    /// \param opt
    /// \return
    int permanentScan(char opt);




    int sendCommand(int dev_id, initializer_list<uint16_t> args);

    /// Stores the found nodes in the given stack
    /// \param nodes_for_adding
    /// \return
    //int getNewNodes(queue<Node> &nodes_for_adding);

    /// Removes the given node from the buffers, This can be called when a node
    /// got disconnected.
    /// This will remove the node from the buffer
    /// \param address The nodes address
    /// \param conn_state The connection state of the node
    /// \param conn_info The connection info (information fro hcitool con command)
    /// \return
    int removeNode(HwAddress address);


    /// Stores the found nodes in the given stack
    /// \param nodes_for_adding
    /// \return
    Node& getNextDiscoverdNodes();

    int getDiscoverdNodesLeftInBuffer() ;

    void popDiscoverdNodes();

    /// Returns the current BD Hardware address
    /// \return Returns the HCI BD Address
    HwAddress getHCIAddress();

    /// Returns the HCI Device ID
    /// \return Returns the HCI Device ID
    int getHCIDeviceID();


    /// Refreshes, when called the List of connected devices connected_nodes_ and their states.
    /// \return
    int updateConnectionStateList();


    /// Updates the connection State of the given node
    /// \param nodeForUpdate
    /// \return
    int updateStateOfNode(Node &nodeForUpdate);


    /// Sets node to state disabled so it won't be considered for adding it to nodes_buffer_ the buffer
    /// If a node is set to disabled it will be ignored. This means that the BLEHelper Class won't get an notificaton
    /// if this node has been found.
    /// \param address
    /// \return
    //int setNodetoDisabled(HwAddress address);


    /// Sets node to state 'enabled'. If a node has been disabled this command will set it back to 'enabled'
    /// for discovering.
    /// \param address
    /// \return
    //int setNodetoEnabled(HwAddress address);


    // For storing the connected Nodes Information
    struct HciNodesInformation{
        HwAddress node_bd_address;
        hci_conn_info info;
    };


    // Static Functions for getting HCI Device Information

    /// A static function which returns the device address of the given HCI Device ID
    /// \param dev_id The HCI Device ID from which the adress should be acquired
    /// \return An HWAddress datatype with the address information of the given HCI Device ID.
    static HwAddress acquireDeviceAddress(int dev_id);

    /// A static function which acquires the Device ID from a given hardware address.
    /// \param hci_hw_address The address from which the ID should be acquired
    /// \return The id of the Device, otherwise -1 if an error occurs .
    static int acquireDeviceID(string hci_hw_address);

private:




    int hci_dev_id_ = -1; // The device ID

    HwAddress hci_dev_hw_addr_ = HwAddress();

    // Stores the nodes which are connected. If a node has been connected the node will be removed
    // from the "discovered_nodes_buffer_" and added to this buffer
    vector<HciInterface::HciNodesInformation> connected_nodes_;


    // Stores the nodes which should be parsed by the BLEHelper Class
    queue<Node> nodes_buffer_;


    // This "blacklists" nodes that already has been found. If an valid node is found but not connected, they get added to the
    // "discovered_nodes_buffer_".
    // This in necessary because until this note gets connected by the BLEHelper
    // the node will pup up in the scan list until it's connection attempt was successful
    // Stores the node which has already been discovered and parsed
    vector<Node> discovered_nodes_buffer_;



    /// Returns If an node has already been found by the scan. If an Node has been found
    /// it gets added to a buffer ("blacklist the node") to avoid multiple parsing of the s
    /// ame information.
    /// \param address The address to check.
    /// \return Return -1 if no node has been found
    /// \ŗeturn The position in the buffer if the node has been found
    int isNodeDiscovered(HwAddress address);


    /// Retruns if an node has bee connected already. Returns the position of the node
    /// if it's found in the "discovered_connected_nodes_buffer_"
    /// \param address The node of the address to check
    /// \return The position of the node in the buffer if connected (positive integer)
    /// \return -1 if the given node has not been discovered
    /// \return -2 if the given node is not connected
    //int isNodeConnected(HwAddress address);


    /// Sets the given node connection status to connected as well as the connection info
    /// \param address The nodes address
    /// \param conn_state The connection state of the node
    /// \param conn_info The connection info (information fro hcitool con command)
    /// \return
    //int setNodeConnected(HwAddress address, int conn_state, hci_conn_info conn_info);



    // int removeNode(HwAddress address);
    // Buffer Management

    /// Updates the name of the given node in the blacklist buffer.
    /// If an node which has been already added to the  discovered_nodes_buffer_ has been found again but
    /// with a name information, his function checks if the name has alreday been updated, otherwise it updates the
    /// buffer with the given information.
    /// \param address Address of Node to check
    /// \param name the new name of the node
    /// \return -1 if the given Node has not been found
    /// \return 0 If the given nide has alreday the same name
    /// \return The position of the Node which has been updated.
    //int updateDiscoveredNode(HwAddress address, string name);


    /// This trys to update the given node spcified by it's address.
    /// This function only has an effect if an node has already been added
    /// to the "discovered_nodes_buffer_".
    /// \param address The address of the node to update
    /// \param name The new name of the
    /// \return 0 if updated,
    /// \return -1 if the node has not been discovered
    /// \return -2 If the node's name has already been updated
    int tryUpdateNodesName(HwAddress address, string name);



    /// Add a IPv6 nodes to the node_buffer_ attribute for later consuming. The stored nodes can be accessed with readFromBuffer()
    /// \param address  Teh node's hardware address for adding
    /// \param name Name of the the node
    /// \return 0
    int addIPv6NodeToBuffer(HwAddress address, string name);



    int read_flags(uint8_t *flags, const uint8_t *data, size_t size);


    struct data_dump {
        uint8_t type;
        string data_s;
        string print_data;
        vector <uint8_t> data;
    };


    // Scanning for new nodes
    int startScan(char opt);

    /// Scan for advertising devices. This function scans for BLE-Devices and add IPv6 enabled one to the buffer,
    /// readable with readFromBuffer for later consuming.
    /// \param dd
    /// \param filter_type
    /// \return
    int scanAdvertisingDevices(int dd, int filter_type);



    /// TODO: description
    /// \param eir
    /// \param eir_len
    /// \param buf
    /// \param buf_len
    void parseNodeName(uint8_t *eir, size_t eir_len, char *buf, size_t buf_len);


    /// TODO: description
    /// \param info
    /// \return
    bool checkIPv6Flag(le_advertising_info *info);


    /// TODO: description
    /// \param data
    /// \return
    data_dump extInquiryDataDump(uint8_t *data);


    /// TODO: description
    /// \param data
    /// \return
    int checkReportFilter(uint8_t procedure, le_advertising_info *info);



};


#endif //BLED_HCITOOL_H
