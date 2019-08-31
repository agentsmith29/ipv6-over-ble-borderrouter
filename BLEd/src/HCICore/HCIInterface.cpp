//
// Created by developer on 06.06.19.
//
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>
#include <sstream>
#include <iostream>
#include <sstream>



#include "../BLEHelper.h"
#include "HCIInterface.h"
#include "../Logger.h"
#include "../Defines.h"


//#include "proto/node_message.pb.h"

std::mutex lock_nodes_buffer_, lock_discoverd_nodes_;

/*

volatile int signal_received = 0;
void sigint_handler(int sig) {
    signal_received = sig;
}
*/

/* Constructors */
//----------------------------------------------------------------------------------------------------------------------
HciInterface::HciInterface(int hci_dev_id):
        hci_dev_id_(hci_dev_id){

    if(hci_dev_id_ < 0) {
        Logger::write({"Wrong HCI Dev No: hci", std::to_string(hci_dev_id_), ". Will set to 0"},
                      DEBUG_MSG, BLE_LOG_FILE);
        hci_dev_id_ = 0;
    }

    // The HCI-Helper has been initialized with an hci_dev_ID and
    hci_dev_hw_addr_ = HciInterface::acquireDeviceAddress(hci_dev_id_);

    if(hci_dev_hw_addr_.getAddressType() == DUMMY_ADDRESS)
        Logger::write({"HCI Device Error"},
                      ERROR_MSG, BLE_LOG_FILE);
}

//----------------------------------------------------------------------------------------------------------------------
HciInterface::HciInterface(string hci_dev_addr){
    // Get the hci device number
    hci_dev_id_ = HciInterface::acquireDeviceID(hci_dev_addr);
    if(hci_dev_id_ < 0)
        Logger::write({"HCI Device Error"},
                      ERROR_MSG, BLE_LOG_FILE);
    else
        hci_dev_hw_addr_ = HciInterface::acquireDeviceAddress(hci_dev_id_);

    if(hci_dev_hw_addr_.getAddressType() == DUMMY_ADDRESS)
        Logger::write({"HCI Device Error"},
                      ERROR_MSG, BLE_LOG_FILE);
}




/* For accessing the Node's Information by the BLE Helper */
//----------------------------------------------------------------------------------------------------------------------
Node& HciInterface::getNextDiscoverdNodes() {
    std::unique_lock<std::mutex> lock(lock_nodes_buffer_);

    //Logger::write({"Next dev ", nodes_buffer_.front().getAddress(),
    //               " with name ", nodes_buffer_.front().getName()},
    //              DEBUG_MSG, BLE_LOG_FILE);
    return nodes_buffer_.front();
}

//----------------------------------------------------------------------------------------------------------------------
int HciInterface::getDiscoverdNodesLeftInBuffer() {
    std::unique_lock<std::mutex> lock(lock_nodes_buffer_);
    return nodes_buffer_.size();
}

//----------------------------------------------------------------------------------------------------------------------
void HciInterface::popDiscoverdNodes() {
    std::unique_lock<std::mutex> lock(lock_nodes_buffer_);
    nodes_buffer_.pop();
}




/* Nodes Buffer Management */
//----------------------------------------------------------------------------------------------------------------------
int HciInterface::removeNode(HwAddress address) {
    std::lock_guard<std::mutex> disc_lock(lock_discoverd_nodes_);

    int pos = isNodeDiscovered(address);
    if(pos >= 0){ // The node has already been added
        // Remove it from the discovered_nodes_buffer_
        discovered_nodes_buffer_.erase(discovered_nodes_buffer_.begin()+pos);
        return 0;
    }
    return -1;

}


//----------------------------------------------------------------------------------------------------------------------
int HciInterface::addIPv6NodeToBuffer(HwAddress address, string name) {
    std::lock_guard<std::mutex> disc_lock(lock_discoverd_nodes_);

    // Check if a node has already been discovered
    // This function checks if a node is in the buffer and if so it will update it's name
    int node_in_buffer = isNodeDiscovered(address);
    if(node_in_buffer >= 0){ // The node has already been added
        return 0;
    }

    // If no node has been found, create a new node
    Node tmp_node(name, address, true);
    tmp_node.setConnectionState(CONNECTABLE);
    // Add this node to our discovered_nodes_buffer_ so it won't get parsed multiple times

    discovered_nodes_buffer_.push_back(tmp_node);

    // Also add the node to nodes_buffer_ so our BLEHelper can update the Node
    std::lock_guard<std::mutex> lock(lock_nodes_buffer_);
    nodes_buffer_.push(tmp_node);

    return 0;
}


//----------------------------------------------------------------------------------------------------------------------
int HciInterface::tryUpdateNodesName(HwAddress address, string name) {
    std::lock_guard<std::mutex> disc_lock(lock_discoverd_nodes_);

    // We can use the isNodeDiscovered function to search for the node
    int pos = isNodeDiscovered(address);
    if(pos < 0)
        return -1;// No update, because the node has not been found

    if(!strcmp(discovered_nodes_buffer_[pos].getName().c_str(), name.c_str())) {

        return -2; // No update, because the name is the same
    }
    else {
        discovered_nodes_buffer_[pos].setName(name); // We update the discovered node first
        // The node has already been found and updated, we need to re-add it to the nodes_buffer
        // so the BLEHelper can update the new information
        std::lock_guard<std::mutex> lock(lock_nodes_buffer_);
        nodes_buffer_.push(discovered_nodes_buffer_[pos]);

    }

    // Check if a node has already been discovered
    // This function checks if a node is in the buffer and if so it will update it's name
    return 0;

}


//----------------------------------------------------------------------------------------------------------------------
int HciInterface::isNodeDiscovered(HwAddress address){
    for (int i = 0; i < discovered_nodes_buffer_.size(); i++) {

        if(!strcmp(discovered_nodes_buffer_[i].getAddress().c_str(), address.getAddress().c_str())) {
            return i;
        }
    }
    return -1;
}


//----------------------------------------------------------------------------------------------------------------------
int HciInterface::updateStateOfNode(Node &nodeForUpdate){

    std::lock_guard<std::mutex> lock(lock_nodes_buffer_);

    updateConnectionStateList();

    // If an node has been found
    bool node_connected = false;
    int i;
    // uint16_t handle, uint8_t  type, uint8_t out, uint16_t state, uint32_t link_mode
    // Iterate though connected nodes


    for (i = 0; i < connected_nodes_.size(); ++i) {
        if(!strcmp(nodeForUpdate.getAddress().c_str(), connected_nodes_[i].node_bd_address.getAddress().c_str())
           && connected_nodes_[i].info.handle != 0) {
            if(nodeForUpdate.getConnectionState() != CONNECTED) {
                // Update connection state
                nodeForUpdate.setHCIDev(hci_dev_id_); // Set the node's HCI-Dev
                // Can also be displayed by typing 'hcitool con'
                nodeForUpdate.setConnectionInfo(
                        connected_nodes_[i].info.handle,
                        connected_nodes_[i].info.type,
                        connected_nodes_[i].info.out,
                        connected_nodes_[i].info.state,
                        connected_nodes_[i].info.link_mode);
                if(nodeForUpdate.getNodeState() == 1)
                    nodeForUpdate.setConnectionState(CONNECTED);

            } else if(nodeForUpdate.getConnectionState() != CONNECTED)
                return 0;

            node_connected = true;
            break;
        }
    }

    if (!node_connected && nodeForUpdate.getConnectionState() == CONNECTED) {
        nodeForUpdate.setConnectionState(DISCONNECTED);
        lock_nodes_buffer_.unlock();
        removeNode(HwAddress(nodeForUpdate.getAddress(), nodeForUpdate.getAddressType()));


    }

}


//----------------------------------------------------------------------------------------------------------------------
int HciInterface::updateConnectionStateList() {


    HciNodesInformation new_info;
    int flag = HCI_UP;

    struct hci_dev_list_req *dl;
    struct hci_dev_req *dr;
    //int dev_id = -1;
    int i, sk, err = 0;


    sk = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
    if (sk < 0) {
        Logger::write({"Can't open Socket: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
        return -1;
    }

    // Clear the connected_nodes_
    connected_nodes_.clear();


    dl = (hci_dev_list_req *) malloc(HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));
    if (!dl) {
        err = errno;
        close(sk);
        errno = err;
        Logger::write({"Error: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
        return -1;
    }

    memset(dl, 0, HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));

    dl->dev_num = HCI_MAX_DEV;
    dr = dl->dev_req;

    if (ioctl(sk, HCIGETDEVLIST, (void *) dl) < 0) {
        err = errno;
        Logger::write({"Error: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
        close(sk);
        errno = err;
        free(dl);
    }


    for (i = 0; i < dl->dev_num; i++, dr++) {
        if (hci_test_bit(flag, &dr->dev_opt) && dr->dev_id == hci_dev_id_) {
           // conn_list(sk, dr->dev_id, arg);
            hci_dev_id_ = dr->dev_id;

            struct hci_conn_list_req *cl;
            struct hci_conn_info *ci;


            if (!(cl = (hci_conn_list_req *) malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
                Logger::write("Can't allocate memory", FATAL_MSG, BLE_LOG_FILE);
                return -1;
            }
            cl->dev_id = hci_dev_id_;
            cl->conn_num = 10;
            ci = cl->conn_info;

            if (ioctl(sk, HCIGETCONNLIST, (void *) cl)) {
                Logger::write("Can't get connection list", FATAL_MSG, BLE_LOG_FILE);
                close(sk);
                errno = err;
                return -1;
            }

            for (i = 0; i < cl->conn_num; i++, ci++) {
                char addr[18];
                char *str;
                ba2str(&ci->bdaddr, addr);
                str = hci_lmtostr(ci->link_mode);


                // Assign the Nodes information
                new_info.node_bd_address.setAddress(addr, NONE_ADDRESS);
                new_info.info.handle = ci->handle;
                new_info.info.state = ci->state;
                new_info.info.out = ci->out;
                new_info.info.link_mode = ci->link_mode;
                new_info.info.type = ci->type;

                // Add this node to the connected_nodes buffer
                connected_nodes_.push_back(new_info);

                // Just for printing out connected nodes
                // char buf[1024];
                // sprintf(buf, "\t%s %s %s handle %d state %d lm %s", "<", "LE", addr, ci->handle, ci->state, str);
                // buf[1023] = '\0';
                // Logger::write(buf, INFO_MSG, BLE_LOG_FILE);
                bt_free(str);
            }

            free(cl);
            close(sk);
            return 0;
        }

    }

}







/* Static Functions */
// ------------------------------------------------------------------------------------------------------------------
HwAddress HciInterface::acquireDeviceAddress(int dev_id){

    // hci_for_each_dev_(HCI_UP, dev_id, "");
    int flag = HCI_UP;
    struct hci_dev_list_req *dl;
    struct hci_dev_req *dr;

    //int dev_id = -1;
    int i, sk, err = 0;
    HwAddress ret_hci_addr = HwAddress();


    sk = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
    if (sk < 0){
        Logger::write({"Could not open socket: ", strerror(errno)}, ERROR_MSG, BLE_LOG_FILE);
        return ret_hci_addr;

    }

    dl = (hci_dev_list_req*)malloc(HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));
    if (!dl) {
        err = errno;
        close(sk);
        errno = err;
        Logger::write({"Could not allocate memory: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
        return ret_hci_addr;
    }

    memset(dl, 0, HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));

    dl->dev_num = HCI_MAX_DEV;
    dr = dl->dev_req;

    if (ioctl(sk, HCIGETDEVLIST, (void *) dl) < 0) {
        err = errno;
        Logger::write({"ioctl Error: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
        close(sk);
        errno = err;
        free(dl);
        return ret_hci_addr;
    }


    for (i = 0; i < dl->dev_num; i++, dr++) {
        if (hci_test_bit(flag, &dr->dev_opt)) {

            char addr[18];
            struct hci_dev_info di = { .dev_id = (uint16_t)dev_id };

            if (ioctl(sk, HCIGETDEVINFO, (void *) &di)){
                Logger::write({"ioctl Error: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
                return ret_hci_addr;
            }

            ba2str(&di.bdaddr, addr);
            if (dr->dev_id == dev_id) {
                string address = addr;
                ret_hci_addr.setAddress(address, static_cast<int>(di.type));
                return ret_hci_addr;
            }
        }

    }


    if(ret_hci_addr.getAddressType() != DUMMY_ADDRESS)
        Logger::write({"HCI Device Address set to ", ret_hci_addr.getAddress()}, INFO_MSG, BLE_LOG_FILE);
    else
        Logger::write({"Acquiring HCI Device Address failed."}, FATAL_MSG, BLE_LOG_FILE);

    return ret_hci_addr;
}

//----------------------------------------------------------------------------------------------------------------------
int HciInterface::acquireDeviceID(string hci_hw_address){

    // hci_for_each_dev_(HCI_UP, dev_id, "");
    int flag = HCI_UP;
    struct hci_dev_list_req *dl;
    struct hci_dev_req *dr;

    //int dev_id = -1;
    int i, sk, err = 0, dev_id = -1;


    sk = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
    if (sk < 0){
        Logger::write({"Could not open socket: ", strerror(errno)}, ERROR_MSG, BLE_LOG_FILE);
        return dev_id;

    }

    dl = (hci_dev_list_req*)malloc(HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));
    if (!dl) {
        err = errno;
        close(sk);
        errno = err;
        Logger::write({"Could not allocate memory: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
        return dev_id;
    }

    memset(dl, 0, HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));

    dl->dev_num = HCI_MAX_DEV;
    dr = dl->dev_req;

    if (ioctl(sk, HCIGETDEVLIST, (void *) dl) < 0) {
        err = errno;
        Logger::write({"ioctl Error: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
        close(sk);
        errno = err;
        free(dl);
        return dev_id;
    }


    for (i = 0; i < dl->dev_num; i++, dr++) {
        if (hci_test_bit(flag, &dr->dev_opt)) {

            char addr[18];
            struct hci_dev_info di = { .dev_id = dr->dev_id };

            if (ioctl(sk, HCIGETDEVINFO, (void *) &di)){
                Logger::write({"ioctl Error: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
                return dev_id;
            }

            ba2str(&di.bdaddr, addr);
            if (!strcmp(hci_hw_address.c_str(), addr)) {
                dev_id = di.dev_id;
                Logger::write({"Returned hci", std::to_string(di.dev_id), " for address", hci_hw_address},
                              DEBUG_MSG, BLE_LOG_FILE);
                return dev_id;
            }
        }

    }


    if(dev_id < -1)
        Logger::write({"HCI Device Address set to ", std::to_string(dev_id)}, INFO_MSG, BLE_LOG_FILE);
    else
        Logger::write({"Acquiring HCI Device ID failed."}, FATAL_MSG, BLE_LOG_FILE);

    return dev_id;
}

//----------------------------------------------------------------------------------------------------------------------
HwAddress HciInterface::getHCIAddress(){
    return hci_dev_hw_addr_;
}

//----------------------------------------------------------------------------------------------------------------------
int HciInterface::getHCIDeviceID(){
    return hci_dev_id_;
}


