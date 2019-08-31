//
// Created by developer on 06.06.19.
//

#include "Node.h"
#include "Logger.h"
#include "Defines.h"
#include "ConfigHandler.h"
#include "ICommand.h"
#include "BLEHelper.h"
#include <thread>

std::mutex node_lock_;

Node::Node(string name, HwAddress hw_address, string ipv6_address, bool connect):
    name_(name), hw_address_(hw_address), ipv6_address_(ipv6_address), connect_node_(connect){

    localLinkAddress_ = "not set";

    node_connected_ = false;             // Mark this node as not connected
    connection_state_ = NOT_CONNECTED;   // Set this node to NOT_CONNECTED
    connection_retrys_ = 0;              // Set the connection retry to 0


    // Set these parameters to -1
    conn_parameters_.conn_interval_min = -1;
    conn_parameters_.conn_interval_max = -1;
    conn_parameters_.conn_latency = -1;
    conn_parameters_.supervision_timeout = -1;
    conn_parameters_.minimum_ce_length = -1;
    conn_parameters_.maximum_ce_length = -1;

    conn_to_hci_dev_ = -1;

}

Node::Node(string name, HwAddress hw_address, bool connect):
        name_(name), hw_address_(hw_address), connect_node_(connect){

    localLinkAddress_ = "not set";
    ipv6_address_ = "not set";

    node_connected_ = false;             // Mark this node as not connected
    connection_state_ = NOT_CONNECTED;   // Set this node to NOT_CONNECTED
    connection_retrys_ = 0;              // Set the connection retry to 0


    // Set these parameters to -1
    conn_parameters_.conn_interval_min = -1;
    conn_parameters_.conn_interval_max = -1;
    conn_parameters_.conn_latency = -1;
    conn_parameters_.supervision_timeout = -1;
    conn_parameters_.minimum_ce_length = -1;
    conn_parameters_.maximum_ce_length = -1;

    conn_to_hci_dev_ = -1;

}



string Node::getName(){
    return name_;
}

string Node::getAddress(){
    return hw_address_.getAddress();
}

HwAddress Node::getFullAddressData(){
    return hw_address_;
}

int Node::setName(string name){;

    name_ = name;
    return 0;
}

int Node::setHCIDev(int hci_dev){
    conn_to_hci_dev_ = hci_dev;
}

int Node::getHCIDev(){
    return conn_to_hci_dev_;
}


string Node::getIPv6_address(){
    return ipv6_address_;
}

int Node::setIPv6_address(string ipv6_address){
    ipv6_address_ = ipv6_address;
    return 0;
}

int Node::connectNode(bool connect){
    connect_node_ = connect;
    return 0;
}


string Node::getAddressType(bool as_number){
    if (as_number == false) {
        switch (hw_address_.getAddressType()){
            case PUBLIC_ADDRESS:
                return "Public Address";
            case RANDOM_STATIC_ADDRESS:
                return "Random Static Address";
            case PRIVATE_RESOLVABLE_ADDRESS:
                return "Private Resolvable Address";
            case PRIVATE_NON_RESOLVABLE_ADDRESS:
                return "Private Non-resolvable Address";
        }
    }
    else
        return std::to_string(hw_address_.getAddressType());
}

int Node::getAddressType(){
    return hw_address_.getAddressType();
}

int Node::getConnectionState(){
    return connection_state_;
}

void Node::setConnectionState(int connectionState){

    if(connection_state_ == CONNECTABLE && connectionState == CONNECTED) {
        Logger::write({"Node connected: ", BLUE, name_, RESET, "/", BLUE, hw_address_.getAddress(), RESET},
                      SYSTEM_MSG, BLE_LOG_FILE);
        ConnectionParameters tmp_con_param;
        if(ConfigHandler::getNodeInitialConnectionParameters(hw_address_, tmp_con_param) >= 0) {

            // TODO This is just a workaround!!!
            // We spawn a new thread, so the helper won't stuck
            // wait a couple of second with the Connecten Parameter Event
            // If we send it rigth after establishing a connection, this will result
            // in an error (the node will fall back to the initial config
            // After a couple of seconds, the thread will push back the command
            std::thread wait_cmd(&Node::createConnParamUpdateEvent, this, tmp_con_param);
            wait_cmd.detach();
        }
        else
            Logger::write({"No initial connection for ", hw_address_.getAddress()},
                          SYSTEM_MSG, BLE_LOG_FILE);
    }



    if(connection_state_ == NOT_CONNECTED && connectionState == CONNECTED)
        Logger::write({"Node added: ", BLUE, name_, RESET, "/", BLUE, hw_address_.getAddress(), RESET},
                SYSTEM_MSG, BLE_LOG_FILE);

    if(connection_state_ == DISCONNECTED && connectionState == CONNECTED)
        Logger::write({"Node reconnected: ", BLUE, name_, RESET, "/", BLUE, hw_address_.getAddress(), RESET},
                      SYSTEM_MSG, BLE_LOG_FILE);

    if(connection_state_ == CONNECTED && connectionState == DISCONNECTED)
        Logger::write({"Node diconnected: ", BLUE, name_, RESET, "/", BLUE, hw_address_.getAddress(), RESET},
                      SYSTEM_MSG, BLE_LOG_FILE);


    if(connectionState < 0)
        connection_state_ = 0; // TODO: error handling, this value is not valid

    connection_state_ = connectionState;
}

int Node::setConnectionInfo(uint16_t handle, uint8_t  type, uint8_t out, uint16_t state, uint32_t link_mode){

    conn_info_.handle = handle;
    conn_info_.type = type;
    conn_info_.out = out;
    conn_info_.state = state;
    conn_info_.link_mode = link_mode;
}

int Node::getNodeHandle(){

    return static_cast<int>(conn_info_.handle);
}

int Node::getNodeType(){

    // Typically LE
    return static_cast<int>(conn_info_.type);
}

int Node::getNodeState(){

    return static_cast<int>(conn_info_.state);
}

int Node::getNodeLinkMode(){

    return static_cast<int>(conn_info_.link_mode);
}

int Node::createConnParamUpdateEvent(ConnectionParameters params_for_update){
    Logger::write(std::to_string(getNodeHandle()), DEBUG_MSG, BLE_LOG_FILE);

    // The function oly takes an ICommand as an Parameter, so we create an temporary one
    // For this we need to look up the handle of the given Address
    ICommand* tmp_cmd_discon_update = new ICommand(CMD_CON_UPDATE, conn_to_hci_dev_,
                                   {getNodeHandle(),
                                    params_for_update.conn_interval_min,
                                    params_for_update.conn_interval_max,
                                    params_for_update.conn_latency,
                                    params_for_update.supervision_timeout,
                                    params_for_update.minimum_ce_length,
                                    params_for_update.minimum_ce_length});

    sleep(3);
    //Send the command for processing
    BLEHelper::pushCommand(tmp_cmd_discon_update);
}


int Node::getConnectionRetry(){

    return connection_retrys_;
}

int Node::incrementConnectionRetry(){

    return connection_retrys_++;
}


int Node::getConnectionIntervalMin(){
    return conn_parameters_.conn_interval_min;
}

int Node::getConnectionIntervalMax(){
    return conn_parameters_.conn_interval_max;
}

int Node::getConnectionLatency(){
    return conn_parameters_.conn_latency;
}

int Node::getSupervisionTimeout(){
    return conn_parameters_.supervision_timeout;
}

int Node::getMinimumCELength(){
    return conn_parameters_.minimum_ce_length;
}

int Node::getMaximumCELength(){
    return conn_parameters_.maximum_ce_length;
}

int Node::setConnectionsParameter(Node::ConnectionParameters conn_params) {

    conn_parameters_.conn_interval_min = conn_params.conn_interval_min;
    conn_parameters_.conn_interval_max = conn_params.conn_interval_max;
    conn_parameters_.conn_latency = conn_params.conn_latency;
    conn_parameters_.supervision_timeout = conn_params.supervision_timeout;
    conn_parameters_.minimum_ce_length = conn_params.minimum_ce_length;
    conn_parameters_.maximum_ce_length = conn_params.maximum_ce_length;
    return 0;
}