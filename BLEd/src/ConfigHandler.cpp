//
// Created by developer on 11.07.19.
//
#include <mutex>

#include "Logger.h"
#include "NodeMappings.h"
#include "ConfigHandler.h"
#include "UtilityFunctions.h"
#include "sstream"

using namespace libconfig;

// Stores initial connection information for a Node
std::vector<std::pair<HwAddress, Node::ConnectionParameters>> node_conn_param;

// Stores the number of the hcis which should get initialized
static vector<int> hci_devices_no;

// Stores the default hco device
//static string default_hci_device;

// Stores the socket point
static int socket_port, default_hci_device;

// ---------------------------------------------------------------------------------------------------------------------
int ConfigHandler::loadDaemonConfigFile(string config_path) {

    Logger::write({"Loading daemon's configuration file: ", config_path}, INFO_MSG, BLE_LOG_FILE);

    /* Using libconfig for loading the file */
    Config dConf;

    try {
        dConf.readFile(config_path);
    }
    catch(const FileIOException &fioex) {
        Logger::write("I/O error while reading file.", FATAL_MSG, BLE_LOG_FILE);
        return(EXIT_FAILURE);
    }
    catch(const ParseException &pex) {
        Logger::write({"Parse error at ", pex.getFile(), ":", std::to_string(pex.getLine()),
                       " - ", pex.getError()}, FATAL_MSG, BLE_LOG_FILE);
    }

    if(parseDaemonsConfig(dConf) != 0)
        return -1;

    if(parseNodeConfig(dConf) != 0)
        return -1;

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
int ConfigHandler::parseNodeConfig(Config &dConf) {


    const Setting& root = dConf.getRoot();
    /* Try loading the configuration of the nodes */

    try  {
        const Setting &nodes = root["node_config"]["nodes"];
        int count = nodes.getLength();

        for(int i = 0; i < count; ++i) {
            const Setting &node = nodes[i];

            string bd_address, bd_address_type, con_param_setting;
            int target_hci;

            // Only output the record if all of the expected fields are present.
            if(!(node.lookupValue("bd_address", bd_address)
                 && node.lookupValue("bd_address_type", bd_address_type)
                    && node.lookupValue("target_hci", target_hci)
                 && node.lookupValue("conn_params", con_param_setting))) {
                Logger::write({"The node (" , std::to_string(i) ,") could no be read. Omitting."}, WARNING_MSG, BLE_LOG_FILE);
                continue;
            }
            Logger::write({"Add node's mapping information."}, INFO_MSG, BLE_LOG_FILE);

            NodeMappings::addNodeMappingInfo(HwAddress(bd_address, PUBLIC_ADDRESS), target_hci);

            if(strcmp(con_param_setting.c_str(), ""))
                parseNodeConnectionParameter(dConf, HwAddress(bd_address, PUBLIC_ADDRESS), con_param_setting);
        }
    }
    catch(const SettingNotFoundException &nfex) {
        Logger::write({"The node's setting couldn't be found, exiting."}, WARNING_MSG, BLE_LOG_FILE);
        return -1;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
int ConfigHandler::parseDaemonsConfig(libconfig::Config &dConf){


    const Setting& root = dConf.getRoot();



    bool autoconnect = false;

    try {
        const Setting &hci_config = root["hci_config"];


        // Read the hci's which should be initialized
        const Setting &read_hci_devs = root["hci_config"]["hci_devices"];

        if (read_hci_devs.isArray()) {

            for (int n = 0; n < read_hci_devs.getLength(); ++n) {
                try {
                    hci_devices_no.push_back(BLEd::HCIToInt(read_hci_devs[n]));
                }
                catch (string msg) {
                    Logger::write(msg, WARNING_MSG, BLE_LOG_FILE);
                }
            }
        }


        // If no value has been found, init with hci0
        if (hci_devices_no.size() == 0)
            hci_devices_no.push_back(0);


        if (!(hci_config.lookupValue("socket_port", socket_port))) {

            Logger::write({"Error Config: Socket Port not specified, using ",
                           std::to_string(DEFAULT_SOCKET_PORT)}, WARNING_MSG, BLE_LOG_FILE);
            socket_port = DEFAULT_SOCKET_PORT;
        }

        if (socket_port < 1) {
            Logger::write({"Error Config: Socket Port not valid (", std::to_string(socket_port), "), using ",
                           std::to_string(DEFAULT_SOCKET_PORT)}, WARNING_MSG, BLE_LOG_FILE);
            socket_port = DEFAULT_SOCKET_PORT;
        }

        string tmp_hci_device;
        if (!(hci_config.lookupValue("default_hci_device", tmp_hci_device))) {
            Logger::write({"Error Config: No default HCI found, using hci",
                           std::to_string(DEFAULT_HCI_NO)}, WARNING_MSG, BLE_LOG_FILE);
        }

        try {
            default_hci_device = BLEd::HCIToInt(tmp_hci_device);
        }
        catch (string msg) {
            Logger::write(msg, WARNING_MSG, BLE_LOG_FILE);
        }

        if (!(hci_config.lookupValue("autoconnect", autoconnect))) {
            Logger::write({"Error Config: No field for automatic connect found. Omitting setting."}, WARNING_MSG,
                          BLE_LOG_FILE);
        }


    }
    catch(const SettingNotFoundException &nfex) {
        Logger::write({"Error Config: Omitting config, using default values because: ",
                       nfex.what()}, WARNING_MSG, BLE_LOG_FILE);
    }

    // Just displayng a summry
    std::stringstream summary;
    summary << "Loaded settings -  HCI = [";

    for (int j = 0; j < hci_devices_no.size(); ++j) {

        if (j < hci_devices_no.size()-1)
            summary << "hci" << hci_devices_no[j] << ", ";
        else
            summary << "hci" << hci_devices_no[j];

    }
    summary << "], default hci" << default_hci_device << ", autoconnect: "
    << autoconnect << ", Socket Port: " << socket_port;


    Logger::write({summary.str()}, INFO_MSG, BLE_LOG_FILE);
    return 0;
}


// ---------------------------------------------------------------------------------------------------------------------
int ConfigHandler::parseNodeConnectionParameter(Config &dConf, HwAddress bd_address, string param_name){


    const Setting& root = dConf.getRoot();
    /* Try loading the configuration of the nodes */

    //Node::ConnectionParameters tmp_conn_param;
    std::pair<HwAddress, Node::ConnectionParameters> tmp_conn_param;
    tmp_conn_param.first.setAddress(bd_address.getAddress(), bd_address.getAddressType());
    try  {
        const Setting &nodes_con_param = root[param_name];

        // Only output the record if all of the expected fields are present.
        if(!(   nodes_con_param.lookupValue("conn_interval_min", tmp_conn_param.second.conn_interval_min)
             && nodes_con_param.lookupValue("conn_interval_max", tmp_conn_param.second.conn_interval_max)
             && nodes_con_param.lookupValue("conn_latency", tmp_conn_param.second.conn_latency)
             && nodes_con_param.lookupValue("supervision_timeout", tmp_conn_param.second.supervision_timeout)
             && nodes_con_param.lookupValue("minimum_ce_length", tmp_conn_param.second.minimum_ce_length)
             && nodes_con_param.lookupValue("maximum_ce_length", tmp_conn_param.second.maximum_ce_length))) {
            Logger::write({"The connection information could no be read."}, WARNING_MSG, BLE_LOG_FILE);
            return 0;
        }
        node_conn_param.push_back(tmp_conn_param);
        Logger::write({"Connect parameter Config pushed"}, WARNING_MSG, BLE_LOG_FILE);



    }
    catch(const SettingNotFoundException &nfex) {
        Logger::write({"The node's setting couldn't be found, exiting."}, WARNING_MSG, BLE_LOG_FILE);
        return -1;
    }


    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
int ConfigHandler::getNodeInitialConnectionParameters(HwAddress bd_addr, Node::ConnectionParameters &parameters){

    for (int i = 0; i < node_conn_param.size(); ++i) {
        Logger::write({"COMP: ", bd_addr.getAddress(), "with", node_conn_param[i].first.getAddress()}, WARNING_MSG, BLE_LOG_FILE);

        if(addrComp(bd_addr, node_conn_param[i].first)) {
           parameters.conn_interval_min = node_conn_param[i].second.conn_interval_min;
            parameters.conn_interval_max = node_conn_param[i].second.conn_interval_max;
            parameters.conn_latency = node_conn_param[i].second.conn_latency;
            parameters.supervision_timeout = node_conn_param[i].second.supervision_timeout;
            parameters.minimum_ce_length = node_conn_param[i].second.minimum_ce_length;
            parameters.maximum_ce_length = node_conn_param[i].second.maximum_ce_length;
            return 0;
        }

    }
    return -1; // No config found

}

std::vector<int> &ConfigHandler::getHCINumbersToInit(){

    return hci_devices_no;
}

int ConfigHandler::getSocketPort(){

    if (socket_port > 0)
        return socket_port;
    else{
        Logger::write({"Given Socket Port ", std::to_string(socket_port),
                       " invalid. Using Port ", std::to_string(DEFAULT_SOCKET_PORT) }, WARNING_MSG, BLE_LOG_FILE);
        return DEFAULT_SOCKET_PORT;
    }

}