//
// Created by developer on 06.06.19.
//

#include "NodesHandler.h"
#include "Logger.h"
#include "Defines.h"
#include "NodeMappings.h"
#include "proto/node_message.pb.h"
#include <sstream>


NodesHandler::NodesHandler(int hci_dev_no){

}

int NodesHandler::removeNode(HwAddress hw_address) {

    int node_position = isNodeAvailable(hw_address);


    if (node_position >= 0) {

        Logger::write({"Removed node: ", assigned_nodes_[node_position].getAddress(), "/",
                       assigned_nodes_[node_position].getAddressType(true)},
                      SYSTEM_MSG, BLE_LOG_FILE);
        assigned_nodes_.erase(assigned_nodes_.begin() + node_position);
        assigned_nodes_.shrink_to_fit();

        return 0;
    }


}

int NodesHandler::addNode(Node &new_node){

    // Add a new node
    // If a node has already been added, we update the information
    if(updateNode(new_node.getFullAddressData(), new_node) >= 0)
        return 0; // A node has been updated

    assigned_nodes_.push_back(new_node);
    Logger::write({"Added new node: ", new_node.getAddress(), "/", new_node.getAddressType(true)},
                  SYSTEM_MSG, BLE_LOG_FILE);

    return 0;
}

int NodesHandler::addNode(string name, string hw_address, string ipv6_address, bool connect){

    // Add a new node
    // Construct a temporary node
    HwAddress tmp_address(hw_address, PUBLIC_ADDRESS);
    Node node_to_add(name, tmp_address, ipv6_address, connect);

    if(isNodeConnected(tmp_address) == true) {
        return 0;
    }
    assigned_nodes_.push_back(node_to_add);
    Logger::write({"Added new node: ", node_to_add.getAddress(), "/", node_to_add.getAddressType(true)},
                  SYSTEM_MSG, BLE_LOG_FILE);

    return 0;
}



int NodesHandler::getHCIDevNumber(){
    return 0;
}

int NodesHandler::updateNode(HwAddress hw_address, Node& node_for_update){

    // Returns the position of the node in the buffer
    int i, node_pos = checkForNode(node_for_update);

    if(node_pos < 0)
        return -1; // No update


    for (i = 0; i < assigned_nodes_.size(); i++) {
        if(!strcmp(assigned_nodes_[i].getAddress().c_str(), hw_address.getAddress().c_str())) {

            assigned_nodes_.erase(assigned_nodes_.begin()+i);
            assigned_nodes_.push_back(node_for_update);
            // TODO: Add all parameters for updating the node
            // Here we add the parameters which should get updated
            // if(strcmp(node_for_update.getName().c_str(), ""))
            //    assigned_nodes_[i].setName(node_for_update.getName());

            //if(node_for_update.getConnectionState() == CONNECTABLE)
            //    assigned_nodes_[i].setConnectionState(node_for_update.getConnectionState());
            break;
        }
    }



    Logger::write({"Updated node ", assigned_nodes_[assigned_nodes_.size()-1].getAddress(), ": ",
                   "Name: ", BLUE, assigned_nodes_[assigned_nodes_.size()-1].getName(), RESET},
                   DEBUG_MSG, BLE_LOG_FILE);
    return i; // The node has been updated
}


int NodesHandler::isNodeConnected(HwAddress hw_address) {

    for (int i = 0; i < assigned_nodes_.size(); ++i) {
        Logger::write({"    |-->Address ", assigned_nodes_[i].getAddress(),
                       "/", std::to_string(assigned_nodes_[i].getAddressType()),
                       " at pos:", std::to_string(i), " to compare with :",
                       hw_address.getAddress(),
                       "/", std::to_string(hw_address.getAddressType())},
                      DEBUG_MSG, BLE_LOG_FILE);

        if (addrComp(hw_address, HwAddress(assigned_nodes_[i].getAddress(), assigned_nodes_[i].getAddressType()))) {


            if (assigned_nodes_[i].getConnectionState() == CONNECTED){
                Logger::write({"Address ", assigned_nodes_[i].getAddress()," at pos:", std::to_string(i)},
                              DEBUG_MSG, BLE_LOG_FILE);
            }
                return i;
        }
    }

    return -1;
}


int NodesHandler::isNodeAvailable(HwAddress hw_address) {

    for (int i = 0; i < assigned_nodes_.size(); ++i) {
        if (addrComp(hw_address, HwAddress(assigned_nodes_[i].getAddress(), assigned_nodes_[i].getAddressType()))) {
            return i;
        }
    }

    return -1;
}


int NodesHandler::checkForNode(Node node_to_check){
    for (int i = 0; i <  assigned_nodes_.size(); i++) {
        if(!strcmp(assigned_nodes_[i].getAddress().c_str(), node_to_check.getAddress().c_str()) &&
                assigned_nodes_[i].getAddressType() == node_to_check.getAddressType()){
            return i;
        }
    }
    return -1;
}


int NodesHandler::updateNodesConnectionState(HwAddress hw_address, int state){

    // Get the that should be updated
    // Returns the position of the node in the buffer
    int node_pos = isNodeAvailable(hw_address);

    if(node_pos < 0)
        return -1; // Node not found

    //if(state == DISABLED)
    //    NodeMappings

    assigned_nodes_[node_pos].setConnectionState(state);
    return 0;
}


int NodesHandler::updateNodesConnectionParameter(HwAddress hw_address, Node::ConnectionParameters conn_params){
    int node_pos = isNodeAvailable(hw_address);
    if(node_pos >= 0)
        assigned_nodes_[node_pos].setConnectionsParameter(conn_params);
    return 0;
}

using namespace std;
int NodesHandler::proto_getConnectedNodes(int socket_description){


    BLEd_message::NodesList newNodeList;

    for (int i = 0; i < assigned_nodes_.size(); ++i) {

        // Create a node
        BLEd_message::Node *newNode = newNodeList.add_conn_nodes();

        BLEd_message::NodeConnectionInfo *newNodeConnInfo;

        newNodeConnInfo->set_handle(assigned_nodes_[i].getNodeHandle());
        newNodeConnInfo->set_type(assigned_nodes_[i].getNodeType());
        newNodeConnInfo->set_state(assigned_nodes_[i].getNodeState());
        newNodeConnInfo->set_link_mode(assigned_nodes_[i].getNodeLinkMode());

        newNode->set_allocated_conn_info(newNodeConnInfo);


        newNode->set_hw_address(assigned_nodes_[i].getAddress());
        newNode->set_hw_address_type(assigned_nodes_[i].getAddressType());
        newNode->set_connection_state(assigned_nodes_[i].getConnectionState());
        newNode->set_name(assigned_nodes_[i].getName());
        // more to set..



    }
    string output;


    newNodeList.SerializeToString(&output); //SerializeToOstream(&output);

    send(socket_description, output.c_str(), strlen(output.c_str()), 0);
    send(socket_description,"Done", strlen("Done"), 0);
    return 0;
}









string generateTableContent(std::vector<std::pair<std::string, int>> column){

    int num_of_column = column.size();
    stringstream table;

    table << "|";

    for (int i = 0; i < num_of_column; ++i) {

        table << " " << column[i].first;
        for (int j = column[i].first.size(); j <  column[i].second; ++j) {
            table << " ";
        }
    }

    table << "|" << std::endl;
    return table.str();
}

HwAddress NodesHandler::getAddressFromHandle(int handel){
    for (int i = 0; i < assigned_nodes_.size(); ++i) {
        if (assigned_nodes_[i].getNodeHandle() == handel)
            return assigned_nodes_[i].getFullAddressData();
    }
    HwAddress dummy = HwAddress();
    return dummy;

}