//
// Created by developer on 06.06.19.
//

#include "BLEHelper.h"
#include "NodesHandler.h"
#include "Logger.h"
#include "NodeMappings.h"
#include "ConfigHandler.h"
#include "UtilityFunctions.h"
#include "Defines.h"

#include <sstream>
#include <thread>
#include <queue>

#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <csignal>

#include <limits.h>

using std::ofstream;
using std::stringstream;

#define STD_PREF "Helper(", std::to_string(hci_dev_no_), ") - "

std::mutex helper_lock_, command_lock, command_parsing_lock;

static std::vector<int> ble_helper_instances_;

static std::queue<ICommand*> commands_for_helpers_;



BLEHelper::BLEHelper(HwAddress hci_dev_address) {
    // Deep copy
    hci_dev_address_.setAddress(hci_dev_address.getAddress(), hci_dev_address.getAddressType());
    hci_dev_no_ = -1; // We get the device number later

    Logger::write({STD_PREF, "HCI Init with ", hci_dev_address_.getAddress()}, DEBUG_MSG, BLE_LOG_FILE);

    hci_scan = new HciInterface(hci_dev_address_.getAddress());

    if(hci_scan == nullptr) {
        Logger::write({STD_PREF, "Could not initialize the scanner, exit daemon now."}, FATAL_MSG, BLE_LOG_FILE);
        exit(-1);
    }

    hci_dev_no_ = hci_scan->getHCIDeviceID();

    // Increase the number in the ICommand Class.

    err_init=false;
    if(initHelper()==0)
        //ICommand::incrementHCI();
        BLEHelper::registerBLEHelperInstances(hci_dev_no_);
}


BLEHelper::BLEHelper(int hci_dev_no):
        hci_dev_no_(hci_dev_no) {

    hci_dev_address_ = HwAddress();

    hci_scan = new HciInterface(hci_dev_no_);
    Logger::write({STD_PREF, "HCICore initialized for hci", std::to_string(hci_scan->getHCIDeviceID())}, SYSTEM_MSG, BLE_LOG_FILE);
    if(hci_scan == nullptr) {
        Logger::write("Could not initialize the scanner, exit daemon now.", FATAL_MSG, BLE_LOG_FILE);
        exit(-1);
    }

    hci_dev_address_.setAddress(hci_scan->getHCIAddress().getAddress(),
                                hci_scan->getHCIAddress().getAddressType());

    Logger::write({STD_PREF, "Got address ",
                   hci_dev_address_.getAddress(), "/",
                   std::to_string(hci_dev_address_.getAddressType())},
                           SYSTEM_MSG, BLE_LOG_FILE);

    err_init=false;
    if(initHelper()==0)
        //ICommand::incrementHCI();
        BLEHelper::registerBLEHelperInstances(hci_dev_no_);
}


BLEHelper::~BLEHelper(){

    // Decrement the number in the ICommand Class.
    ICommand::decrementHCI();
}



//---------------------------------------------------------------------------------------------------------------------
int BLEHelper::initHelper() {


    nodes_handler_ = new NodesHandler(hci_dev_no_);

    if (nodes_handler_ == nullptr) {
        Logger::write({STD_PREF, "Could not initialize the node handler, exit daemon now."}, FATAL_MSG, ERROR_MSG);
        exit(-1);
    }

    if (hci_scan == nullptr) {
        Logger::write({STD_PREF, "Could not initialize the scanner, exit daemon now."}, FATAL_MSG, ERROR_MSG);
        exit(-1);
    }

    // Reset and select hci device
    if (initHCIDev(hci_dev_no_) < 0) {
       return -1;
    } else {
        Logger::write({STD_PREF, "All set up. BLE Helper successfully initialized for hci", std::to_string(hci_dev_no_)},
                      INFO_MSG, BLE_LOG_FILE);

        // selectDevice();
        Logger::write({STD_PREF, "Starting scanning for Bluetooth LE devices"}, SYSTEM_MSG, BLE_LOG_FILE);
        std::thread t1(&HciInterface::permanentScan, hci_scan, 'D');
        t1.detach();

        return 0;
    }
}


int BLEHelper::scanAndConnect() {



    if(err_init)
        return 0;

    // Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "Waiting..."}, INFO_MSG, BLE_LOG_FILE);
    // Check if an node is in the buffer and consume this node
    while (hci_scan->getDiscoverdNodesLeftInBuffer() > 0) {

        // This will add any nodes found by the HCITool. Already added nodes with new information will be updated.
        auto tmp_node_ptr = hci_scan->getNextDiscoverdNodes();


        nodes_handler_->addNode(hci_scan->getNextDiscoverdNodes());
        hci_scan->popDiscoverdNodes();
    }




    for (int i = 0; i < (*(nodes_handler_)).size(); ++i) {
        if ((*(nodes_handler_))[i].getConnectionState() == CONNECTABLE &&
                (*(nodes_handler_))[i].incrementConnectionRetry() % 100 == 0) {
            connectNode((*(nodes_handler_))[i], true);
        }


        // Update the connection state of the given node
        hci_scan->updateStateOfNode((*(nodes_handler_))[i]);
    }



    executeNextCommand();



    // Printing the connected devices
    //Logger::write(printNodes(), DEBUG_MSG, BLE_LOG_FILE);

}




//----------------------------------------------------------------------------------------------------------------------
int BLEHelper::executeNextCommand(){


    int socket_desc = -1, command_return = 0;
    // Look for incoming commands on the buffer
    for (int cmd_cnt = 0; cmd_cnt < BLEHelper::commandSize(); cmd_cnt++) {

        ICommand *tmp_command = BLEHelper::getTopCommand();

        Logger::write({"Setting command as passed for hci", std::to_string(hci_dev_no_)},
                      INFO_MSG, BLE_LOG_FILE);
        tmp_command->setCommandParsedBy(hci_dev_no_);

        if (tmp_command->targetHci() == hci_dev_no_) {

            switch (tmp_command->description()) {
                case CMD_CON_UPDATE:
                    executeConnectionUpdate(*tmp_command);
                    break;
                case CMD_GET_NODE_LIST:
                    Logger::write({STD_PREF,  " CMD_GET_NODE_LIST", },
                                  INFO_MSG, BLE_LOG_FILE);
                    tmp_command->args(0, socket_desc);

                    if(socket_desc >= 0) {
                        socket_listConnectedNodes(socket_desc, LIST_CON_NODE_SIMPLE);
                    }
                    //tmp_command->unsetInterfaceLock();
                    break;
                case PROTO_GET_NODE_LIST:
                    tmp_command->args(0, socket_desc);
                    if(socket_desc >= 0)
                        nodes_handler_->proto_getConnectedNodes(socket_desc);
                    break;

                default:
                    break;
            }
            //delete tmp_command
            popTopCommand();
            Logger::write({"HCI: ", std::to_string(hci_dev_no_),  " Deleted the Command"}, INFO_MSG, BLE_LOG_FILE);
            return 0;


        } else {
            switch (tmp_command->description()) {
                case CMD_DISCONNECT:
                    command_return = executeDisconnect(*tmp_command);
                    //popTopCommand();
                    break;
                default:
                    break;

            }

        }


       if(tmp_command->parsedByAllHelpers() && command_return == 0){
           Logger::write({"HCI: ", std::to_string(hci_dev_no_),  " ICommand discarded"}, INFO_MSG, BLE_LOG_FILE);
           popTopCommand();
           return 0;
       }
    }
    return 0;
}


int BLEHelper::executeDisconnect(ICommand &cmd) {

    HwAddress tmp_address;
    int node_position;

    // The Interface Command was an Disconnect command
    // FIX        FIX          args
    // DISCONNECT HCI-Device   <address>
    // Retrieve the address from the Command
    string address = "";
    cmd.args(0, address);

    // TODO This is always a PUBLIC ADDRESS!
    // Check if the given node is connected

    tmp_address = HwAddress(address, PUBLIC_ADDRESS);

    node_position = nodes_handler_->isNodeConnected(tmp_address);
    Logger::write({STD_PREF, " Checking if node ", tmp_address.getAddress(),
                   "/", std::to_string(tmp_address.getAddressType()), " is available: ",
                   std::to_string(node_position)},
                  INFO_MSG, BLE_LOG_FILE);
    if(node_position >= 0) {

        Logger::write({STD_PREF, " Disconnecting ", tmp_address.getAddress(), "/",
                       std::to_string(tmp_address.getAddressType())},
                      INFO_MSG, BLE_LOG_FILE);
        disconnectNode(tmp_address);
        // Remove from buffer after disconnected
        popTopCommand();
        return 1;
    }
    return 0;
}


int BLEHelper::executeConnectionUpdate(ICommand &cmd) {

    stringstream msg;
    int command_size = 8;

    struct dword{
        uint16_t higher_;
        uint16_t lower_;
    };
    dword inputs[command_size];

    // The Interface Command was an Connection Update command
    // FIX        FIX          args
    // CON-UPDATE HCI-Device   <handle> <con_min_int> <con_max_int> <con_lat_int> <sup_vis_to> <min_ce_len> <max_ce_len>
    // Retrieve the information and stores it in an array
    char buf[64];
    inputs[0].higher_ = 0x08;
    inputs[0].lower_  = 0x13;

    Node::ConnectionParameters con_params; // We want to update the nodes config as well.
    vector<int> tmp_param;

    for (int i = 0; i < command_size; ++i) {
        int values = 0;
        cmd.args(i, values);
        tmp_param.push_back(values);
        inputs[i+1].higher_ = static_cast<uint16_t>(values & 0xff);
        inputs[i+1].lower_ = static_cast<uint16_t>(values >> 8);
        sprintf(buf, "0x%2.2x 0x%2.2x", inputs[i].higher_, inputs[i].lower_);
        msg << buf << "  ";
    }

    // Send the command
    int ret = hci_scan->sendCommand(hci_dev_no_,
                          {inputs[0].higher_, inputs[0].lower_,
                           inputs[1].higher_, inputs[1].lower_,
                           inputs[2].higher_, inputs[2].lower_,
                           inputs[3].higher_, inputs[3].lower_,
                           inputs[4].higher_, inputs[4].lower_,
                           inputs[5].higher_, inputs[5].lower_,
                           inputs[6].higher_, inputs[6].lower_,
                           inputs[7].higher_, inputs[7].lower_});
    Logger::write({STD_PREF,  "Send update command: ", msg.str()}, INFO_MSG, BLE_LOG_FILE);

    con_params.conn_interval_min =  static_cast<uint16_t>(tmp_param[1]);
    con_params.conn_interval_max = static_cast<uint16_t>(tmp_param[2]);
    con_params.conn_latency = static_cast<uint16_t>(tmp_param[3]);
    con_params.supervision_timeout = static_cast<uint16_t>(tmp_param[4]);
    con_params.minimum_ce_length = static_cast<uint16_t>(tmp_param[5]);
    con_params.maximum_ce_length = static_cast<uint16_t>(tmp_param[6]);

    if (ret == 0){
        HwAddress address = nodes_handler_->getAddressFromHandle(static_cast<uint16_t>(tmp_param[0]));
        nodes_handler_->updateNodesConnectionParameter(address, con_params);
    }

    return 0;

}



//----------------------------------------------------------------------------------------------------------------------
int BLEHelper::connectNode(Node &node_for_connection, bool connect) {


    selectDevice();
    sleep(0.2);
    Logger::write({STD_PREF,  " connecting attempt for ", node_for_connection.getAddress()}, INFO_MSG, BLE_LOG_FILE);

    int addr_no = 0;
    switch (node_for_connection.getAddressType()) {
        case PUBLIC_ADDRESS:
            addr_no = 2;
            break;
        case RANDOM_STATIC_ADDRESS:
            addr_no = 1;
            break;
        default:
            addr_no = 2;
    }

    std::ofstream kernel_control;
    kernel_control.open(KERNEL_MOD_CONTROL);


    if (!kernel_control.is_open()) {
        Logger::write("Can not open 6lowpan control file",
                      FATAL_MSG, BLE_LOG_FILE);
        return -1;
    } else {
        if (connect) {
            kernel_control << "connect " << node_for_connection.getAddress() << " " << addr_no << std::endl;
            if(errno < 0) {
                Logger::write({STD_PREF, std::to_string(hci_dev_no_),  "Error connecting: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
            }
            else {
                //Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "Done connecting ", node_for_connection.getAddress()}, INFO_MSG, BLE_LOG_FILE);
            }
        } else {
            //kernel_control << "disconnect " << node_for_connection.getAddress() << " " << addr_no << std::endl;
            // if (!errno)
            //    Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "disconnect 0 error ", node_for_connection.getAddress()}, FATAL_MSG, BLE_LOG_FILE);
            stringstream cmd;
            cmd << "echo -e select " << hci_dev_address_.getAddress() << "\n"
                << "disconnect" << node_for_connection.getAddress() << " | sudo bluetoothctl";

            if(system(cmd.str().c_str()) != 0)
                Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "Disconnect Error"},
                              ERROR_MSG, BLE_LOG_FILE);

        }
    }
    kernel_control.close();
    node_for_connection.incrementConnectionRetry();


    sleep(0.5);
    return 0;
}


int BLEHelper::disconnectNode(HwAddress address) {


    hci_scan->removeNode(address);
    (*nodes_handler_).removeNode(address);

    sleep(0.2);
    Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "Disconnection event on hci with address", hci_dev_address_.getAddress()},
                  INFO_MSG, BLE_LOG_FILE);

    //Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "Connect ", node_for_connection.getAddress()}, INFO_MSG, BLE_LOG_FILE);
    int addr_no = 0;
    switch (address.getAddressType()) {
        case PUBLIC_ADDRESS:
            addr_no = 2;
            break;
        case RANDOM_STATIC_ADDRESS:
            addr_no = 1;
            break;
        default:
            addr_no = 2;
    }
        stringstream cmd;
        cmd << "echo \"select " << hci_dev_address_.getAddress() << std::endl
            << "disconnect " << address.getAddress() <<  "\" | sudo bluetoothctl > /home/pi/tmp.log";



        if (system(cmd.str().c_str()) != 0)
            Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "Disconnection Error"},
                          ERROR_MSG, BLE_LOG_FILE);

    return 0;
}


int BLEHelper::selectDevice() {

    std::ofstream kernel_control;
    kernel_control.open(KERNEL_MOD_CONTROL);


    if (!kernel_control.is_open()) {
        Logger::write("Can not open 6lowpan control file",
                      FATAL_MSG, BLE_LOG_FILE);
        return -1;
    } else {
        kernel_control << "select " << hci_dev_address_.getAddress() << std::endl;
        //Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "select " , hci_dev_address_.getAddress()}, INFO_MSG, BLE_LOG_FILE);
        // if(errno)
        //     Logger::write({"HCI: ", std::to_string(hci_dev_no_),  "Error changing device: ", strerror(errno)}, FATAL_MSG, BLE_LOG_FILE);
    }
    kernel_control.close();
    sleep(0.5);
    return errno;
}


int BLEHelper::initHCIDev(int hci_device_no) {

  if(BLEHelper::resetHCI(hci_device_no) < 0) {
      Logger::write({"Disabeling helper for hci", std::to_string(hci_dev_no_)},
                    ERROR_MSG, BLE_LOG_FILE);
      err_init = true;
      return -1;
  }

   return 0;
}


int BLEHelper::resetHCI(int hci_device_no){

    string hci_dev = "hci" + std::to_string(hci_device_no);

    stringstream cmd;

    // Read the desired hci device from the config
    Logger::write({"Reset Host Controller (", hci_dev, ")"},
                  SYSTEM_MSG, BLE_LOG_FILE);

    /* We need to concat these strings to get an valid command */
    cmd  << "sudo hciconfig " << hci_dev << " up" << std::endl;

    if(system(cmd.str().c_str()) != 0) {
        Logger::write({"HCI Controller ", hci_dev, ": Something went wrong, could not reset controller."},
                      ERROR_MSG, BLE_LOG_FILE);
    return -1;
    }
    /* We need to concat these strings to get an valid command */
    cmd  << "sudo hciconfig " << hci_dev << " reset" << std::endl;

    if(system(cmd.str().c_str()) != 0) {
        Logger::write({"HCI Controller ", hci_dev, ": Something went wrong, could not reset controller."},
                      ERROR_MSG, BLE_LOG_FILE);
        return -1;
    }

    Logger::write({"Device hci", std::to_string(hci_device_no), " initialized"},
                  INFO_MSG, BLE_LOG_FILE);
    sleep(0.5);
    return 0;
}




//----------------------------------------------------------------------------------------------------------------------
int BLEHelper::pushCommand(ICommand* command){
    std::lock_guard<std::mutex> lock(command_lock);

    commands_for_helpers_.push(command);

    if(command->targetHci() >= 0) {
        Logger::write({"ICommand pushed for hci",
                       std::to_string(command->targetHci())}, DEBUG_MSG, BLE_LOG_FILE);
    }
    else
        Logger::write({"ICommand pushed for targeting all hci devices"}, DEBUG_MSG, BLE_LOG_FILE);



    return 0;
}


ICommand* BLEHelper::getTopCommand() {

    std::unique_lock<std::mutex> lock(command_lock);

    return commands_for_helpers_.front(); //(ICommand*)(());
}


int BLEHelper::popTopCommand() {

    std::unique_lock<std::mutex> lock(command_lock);

    ICommand *off = commands_for_helpers_.front();
    commands_for_helpers_.pop();
    delete off;

    //delete  commands_for_helpers_.front();
    //commands_for_helpers_.pop();

    Logger::write({"Popped"},
                  INFO_MSG, BLE_LOG_FILE);

    return 0;
}


int BLEHelper::commandSize() {

    std::unique_lock<std::mutex> lock(command_lock);

    return commands_for_helpers_.size();
}


//---------------------------------------------------------------------------------------------------------------------
string BLEHelper::printNodes() {
    std::stringstream list_nodes;
    string conection_state = "";

    list_nodes << "Connected Nodes: \n";
    for (int i = 0; i < (*(nodes_handler_)).size(); ++i) {

        switch ((*(nodes_handler_))[i].getConnectionState()) {
            case NOT_CONNECTED:
                conection_state = "Not connected";
                break;
            case CONNECTED:
                conection_state = "Connected";
                break;
            case DISCONNECTED:
                conection_state = "Disconnected";
                break;
            case FAILED_CONNECTION:
                conection_state = "Connection failed";
                break;
            case DISABLED:
                conection_state = "Ignored";
                break;
            default:
                conection_state = "unknown connection state";
                break;
        }

        list_nodes << i << ": " << (*(nodes_handler_))[i].getName() << "/"
                   << (*(nodes_handler_))[i].getAddress() << "/"
                   << conection_state << std::endl;

    }
    return list_nodes.str();
}


int stringToDouble(string input, double &num_out){

    try{
        num_out = std::atof(input.c_str());
    }
    catch(std::invalid_argument& e){
        // if no conversion could be performed
        return -1;
    }
    catch(std::out_of_range& e){
        // if the converted value would fall out of the range of the result type
        // or if the underlying function (std::strtol or std::strtoull) sets errno
        // to ERANGE.
        return -2;
    }
    catch(...) {
        // everything else
        return -3;
    }
    return 0;

}


std::vector<int> &BLEHelper::getBLEHelperInstances(){

    return ble_helper_instances_;
}

int BLEHelper::registerBLEHelperInstances(int hci_dev){
    Logger::write({"Helper hci", std::to_string(hci_dev), " registered."}, DEBUG_MSG, BLE_LOG_FILE);
    ble_helper_instances_.push_back(hci_dev);
}


//---------------------------------------------------------------------------------------------------------------------
static constexpr int TOP = 1;
static constexpr int BOTTOM = 2;

string generateTableLine(int maxLineNumber, bool double_line);
string generateTableOutline(int maxLineNumber, int pos);
string generateTableHeadline(std::vector<std::pair<std::string, int>> column);

int BLEHelper::socket_listConnectedNodes(int socket_description, int option) {


    /* +-------------------------------------------------------------------------------+
     * | Address           | Name                                     | Handle | State |
     * +-------------------------------------------------------------------------------+
     * | XX:XX:XX:XX:XX:XX | Test IPSP Node                           | 64     | 1     |
     */
    std::stringstream table;

    switch(option) {
        case LIST_CON_NODE_SIMPLE:
            // Generate the header
            table << generateTableOutline(80, TOP);
            table << generateTableHeadline({{"Address", 19},
                                            {"Name",    42},
                                            {"Handle",  8},
                                            {"State",   7}});
            table << generateTableLine(80, false);


            for (int i = 0; i < nodes_handler_->size(); ++i) {
                table << generateTableHeadline({{(*(nodes_handler_))[i].getAddress(),                    19},
                                                {(*(nodes_handler_))[i].getName(),                       42},
                                                {std::to_string((*(nodes_handler_))[i].getNodeHandle()), 8},
                                                {std::to_string((*(nodes_handler_))[i].getNodeState()),  7}});
            }
            table << generateTableOutline(80, BOTTOM);
            break;
        case LIST_CON_NODE_WITH_CON_PARAM:
            table << generateTableOutline(80, TOP);
            table << generateTableHeadline({{"Address", 20},
                                            {"ConIntMax",    10},
                                            {"ConIntMin",    10},
                                            {"ConLat",    10},
                                            {"SupTO",    10},
                                            {"MaxCELen",    10},
                                            {"MinCELen",    10}});
            table << generateTableLine(80, false);
            for (int i = 0; i < nodes_handler_->size(); ++i) {
                table << generateTableHeadline({{(*(nodes_handler_))[i].getAddress(),                    20},
                                                {std::to_string((*(nodes_handler_))[i].getConnectionIntervalMax()), 10},
                                                {std::to_string((*(nodes_handler_))[i].getConnectionIntervalMin()), 10},
                                                {std::to_string((*(nodes_handler_))[i].getConnectionLatency()), 10},
                                                {std::to_string((*(nodes_handler_))[i].getSupervisionTimeout()), 10},
                                                {std::to_string((*(nodes_handler_))[i].getMaximumCELength()), 10},
                                                {std::to_string((*(nodes_handler_))[i].getMinimumCELength()), 10}});
            }
            table << generateTableOutline(80, BOTTOM);
            break;
    }

    send(socket_description, table.str().c_str(), strlen(table.str().c_str()), 0);
}


string generateTableOutline(int maxLineNumber, int pos){
    std::stringstream table;
    // Generate the header
    if (pos == TOP)
        table << "╔";
    else if (pos == BOTTOM)
        table << "╚";

    for (int i = 0; i < maxLineNumber; ++i) {
        table << "═";
    }
    if (pos == TOP)
        table << "╗" << std::endl;
    else if (pos == BOTTOM)
        table << "╝" << std::endl;

    return table.str();
}

inline string generateTableLine(int maxLineNumber, bool double_line){
    std::stringstream table;
    // Generate the header
    table << "╟";

    for (int i = 0; i < maxLineNumber; ++i) {
        if(double_line)
            table << "═";
        else
            table << "─";
    }
    table << "╢" << std::endl;

    return table.str();
}

inline string generateTableHeadline(std::vector<std::pair<std::string, int>> column){

    int num_of_column = column.size();
    stringstream table;

    table << "║";

    for (int i = 0; i < num_of_column; ++i) {

        table << " " << BLUE << column[i].first << RESET;
        for (int j = column[i].first.size(); j <  column[i].second; ++j) {
            table << " ";
        }
    }

    table << "║" << std::endl;
    return table.str();
}

