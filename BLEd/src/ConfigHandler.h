//
// Created by developer on 11.07.19.
//

#ifndef BLED_CONFIGHANDLER_H
#define BLED_CONFIGHANDLER_H
#include "../libconfig/lib/libconfig.h++"
#include "Node.h"
#include <vector>

static constexpr int DEFAULT_SOCKET_PORT = 12345;
static constexpr int DEFAULT_HCI_NO = 0;
//static constexpr int DEFAULT_HCI_NO = 0;

class ConfigHandler {

public:
    // -----------------------------------------------------------------------------------------------------------------
    // This will load the desired configuration file from the given path. The file will be passed with 'libconfig'
    // If no valid config was found this function will return an error code which will result in an exit of the daemon.
    //
    // @param config_path Path to the configuration file
    //
    // @return By default, it returns the number of found node configs.
    // @return -5' if no daemon config was found.
    // @return -6 if no node config was found.
    //
    static int loadDaemonConfigFile(std::string config_path);

    /// If a connection parameter interval has been specified in an config file
    /// this mehthode will return it's value for the given Node.
    /// \param parameters
    /// \return positiv integer if foudn, -1 otherwise
    static int getNodeInitialConnectionParameters(HwAddress bd_addr, Node::ConnectionParameters &parameters);


    static std::vector<int> &getHCINumbersToInit();


    static int getSocketPort();

private:
    //std::vector<string> list
    // -----------------------------------------------------------------------------------------------------------------
    // Will parse the config for the nodes
    //
    // @param dConf The loaded configuration
    //
    // @return By default, it returns the number of found node configs.
    // @return -6 if no node config was found.
    //
    static int parseNodeConfig(libconfig::Config &dConf);


    static int parseNodeConnectionParameter(libconfig::Config &dConf, HwAddress bd_address, string param_name);

    static int parseDaemonsConfig(libconfig::Config &dConf);



 };


#endif //BLED_CONFIGHANDLER_H
