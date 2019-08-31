//
// Created by developer on 06.06.19.
//

#include "InteractionInterface.h"
#include "Logger.h"
#include "NodeMappings.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <algorithm>
#include <sstream>
#include <thread>
#include <sstream>

#include "ICommand.h"
#include "Defines.h"
#include "BLEHelper.h"
#include "UtilityFunctions.h"
#include "ConfigHandler.h"


using std::string;
using std::stringstream;

std::mutex socket_lock;

//int openInterfaceSocket(int Port);

InteractionInterface::InteractionInterface() {

    Logger::write({"Opening communication socket."}, SYSTEM_MSG, BLE_LOG_FILE);
    //initInterfaceSocket(11111);
    std::thread comm_socket(&InteractionInterface::initInterfaceSocket, this, ConfigHandler::getSocketPort());
    comm_socket.detach();
}



//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::initInterfaceSocket(int Port) {


    int opt = true;
    int master_socket, addrlen;
    int activity, new_socket, max_sd;

    vector<std::thread> client_socket;

    // set of socket descriptors
    fd_set readfds;


    // create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        Logger::write({"Interface - Opening socket failed: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        return -1;
    }

    // set master socket to allow multiple connections ,
    // this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        Logger::write({"Interface -  Setting options failed: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        return -1;
    }

    // type of socket created
    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(Port);

    // bind the socket to localhost port 11111
    if (bind(master_socket, (struct sockaddr *) &address_, sizeof(address_)) < 0) {
        Logger::write({"Interface -  Binding failed: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        return -1;
    }

    Logger::write({"Interfacing - Listed on port: ", std::to_string(Port)},
                  SYSTEM_MSG, BLE_LOG_FILE);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0) {
        Logger::write({"Interfacing -  Listening failed: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        return -1;
    }

    Logger::write({"Interfacing socket -  Ready for connections on port ", std::to_string(Port)},
                  SYSTEM_MSG, BLE_LOG_FILE);


    addrlen = sizeof(address_);


    while (true) {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        // wait for an activity on one of the sockets , timeout is NULL ,
        // so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            Logger::write({"Interfacing socket -  Select error: ", strerror(errno)},
                          ERROR_MSG, BLE_LOG_FILE);
            return -1;
        }


        // If something happened on the master socket ,
        // then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *) &address_, (socklen_t *) &addrlen)) < 0) {
                Logger::write({"Interfacing socket -  Accept error: ", strerror(errno)},
                              ERROR_MSG, BLE_LOG_FILE);
                return -1;
            }


            if (current_socket_requests_ < max_socket_requests_) {

                std::thread t1 = std::thread(&InteractionInterface::handleSocketRequest, this,
                                             new_socket, readfds);
                t1.detach();
                current_socket_requests_++;
            } else {
                Logger::write("Maximum connection requests exceeded", ERROR_MSG, BLE_LOG_FILE);
            }
        }
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::handleSocketRequest(int socket, fd_set readfds) {

    // This just creates a master socket for interfacing the daemon
    // The welcome message
    stringstream welcome_message;
    welcome_message << "BLEd Daemon v "
                    << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_REVISION << VERSION_BUILD
                    << "(" << STATE << ")" << std::endl;


    // send new connection greeting message
    if (send(socket, welcome_message.str().c_str(), strlen(welcome_message.str().c_str()), 0)
        != strlen(welcome_message.str().c_str())) {
        Logger::write({"Interfacing socket -  Sending: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        return -1;
    }


    string prefix = "BLEd $ ";


    int sd;
    char buffer[1025];  //data buffer of 1K
    string tmp_buf;


    // initialise all client_socket[] to 0 so not checked
    //for (int i = 0; i < max_clients; i++) {
    //    client_socket[i] = 0;
    //}

    sockaddr_in address = address_;

    sd = socket;
    if (sd > 0)
        FD_SET(sd, &readfds);



    while (true) {
        send(sd, prefix.c_str(), strlen(prefix.c_str()), 0);
        // Delete the content of the buffer
        tmp_buf = "";
        for (int j = 0; j < sizeof(buffer); ++j) {
            buffer[j] = 0;
        }

        if(receiveSocket(tmp_buf, sd) == DISCONNECTED_FROM_SOCKET) {
            Logger::write({"Interfacing socket -  Disconnected: ", inet_ntoa(address_.sin_addr),
                           std::to_string(address_.sin_port)},
                          INFO_MSG, BLE_LOG_FILE);
            current_socket_requests_--;
            return DISCONNECTED_FROM_SOCKET;
        }
        else {
            Logger::write({"Parsing ", inet_ntoa(address_.sin_addr),
                           std::to_string(address_.sin_port)},
                          INFO_MSG, BLE_LOG_FILE);
            if (parseCommand(tmp_buf, sd) == DISCONNECTED_FROM_SOCKET) {
                Logger::write({"Interfacing socket -  Disconnected: ", inet_ntoa(address_.sin_addr),
                               std::to_string(address_.sin_port)},
                              INFO_MSG, BLE_LOG_FILE);
                current_socket_requests_--;
                return DISCONNECTED_FROM_SOCKET;
            }
        }


    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::receiveSocket(string& message, int sd) {

    std::lock_guard<std::mutex> unique_lock(socket_lock);
    int valread, addrlen;
    char buffer[1025];  //data buffer of 1K


    for (int j = 0; j < sizeof(buffer); ++j) {
        buffer[j] = 0;
    }



    if ((valread = read(sd, buffer, 1024)) == 0) {
        //Somebody disconnected , get his details and print
        getpeername(sd, (struct sockaddr *) &address_, \
                    (socklen_t * ) & addrlen);


        // Close the socket and mark as 0 in list for reuse
        close(sd);
        return DISCONNECTED_FROM_SOCKET;
        //client_socket[i] = 0;
    } else {



        string tmp_buf = "";
        tmp_buf = buffer;
        // set the string terminating NULL byte on the end
        // of the data read
        // assign it to a string an trim the buffer
        buffer[valread] = '\0';

        tmp_buf.erase(std::remove(tmp_buf.begin(), tmp_buf.end(), '\n'), tmp_buf.end()); // Trim the string
        tmp_buf.append("\0");
        Logger::write({"Recieved from user: ", tmp_buf},   INFO_MSG, BLE_LOG_FILE);
        message = tmp_buf;
        return 0;
    }

    return 0;

}

//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::sendSocket(string message, int socket_desc) {
    std::unique_lock <std::mutex> lock(socket_lock);
    send(socket_desc, message.c_str(), strlen(message.c_str()), 0);
    return 0;

}




// For handeling the commands
void InteractionInterface::tokenize(std::string const &str, const char *delim,  std::vector<std::string> &out) {
    char *token = strtok(const_cast<char *>(str.c_str()), delim);
    while (token != nullptr) {
        out.push_back(std::string(token));
        token = strtok(nullptr, delim);
    }
}

//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::parseCommand(const string &str, int socket_desc) {

    if(!strcmp(str.c_str(), ""))
        return 0;

    // Tokenize the string
    std::vector<std::string> tokens;
    const char *delim = " ";
    tokenize(str, delim, tokens);

    string command = "";
    if(tokens.size() > 0) {
        command = tokens[0];
        Logger::write({"SET COMMAND <", command, ">"}, SYSTEM_MSG, BLE_LOG_FILE);
        //string hci_dev_input = tokens[1];
    }
    Logger::write({"User: Received <", command, ">"}, SYSTEM_MSG, BLE_LOG_FILE);


    if (!strcmp(command.c_str(), "connect")) {
        return cmdConnect(socket_desc, tokens);
    }
    else if (!strcmp(command.c_str(), "remap")) {
        return cmdRemap(socket_desc, tokens);
    }
    else if (!strcmp(command.c_str(), "con-update")) {
        return cmdConUpdate(socket_desc, tokens);
    }
    else if (!strcmp(command.c_str(), "disconnect")) {
        return cmdDisconnect(socket_desc, tokens);
    }
    else if (!strcmp(command.c_str(), "help")) {
        return cmdHelp(socket_desc);
    }
    else if (!strcmp(command.c_str(), "list")){
        return cmdList(socket_desc, tokens);
    }
    else if (command[0] == 0x03 && command[1] == 0x30){
        sendSocket("Got command\n", socket_desc);
        return protoGetNodes(socket_desc, tokens);
    }
    else{
       sendSocket("Unknown command. List commands with \"help\".\n", socket_desc);
       return cmdHelp(socket_desc);
    }

    return 0;
    //add_args.clear();
}


//----------------------------------------------------------------------------------------------------------------------
// The commands
int InteractionInterface::cmdRemap(int socket_desc, vector<string> tokens) {

    /* Command Remap
     * remap <BD-Address> <to-hci>
     */

    int hci_dev_from_int = 0, hci_dev_to_int  = 0;
    string address = "";

    if(tokens.size() != 3 ) {
        sendSocket("Arguments did't match, expecting 2.\n", socket_desc);
        sendSocket("usage: remap <bd-address> <to-hci>\n", socket_desc);
        return -1;
    }

    if(!strcmp(tokens[1].c_str(), "")) {
        sendSocket("BLE-Node's BD-Address cant be empty. Use the list-command to display available nodes.\n", socket_desc);
        sendSocket("usage: remap <bd-address> <to-hci>\n", socket_desc);

        return -1;
    }
    if(tokens.size() == 3 && tokens[1].size() == 17) {
        address = tokens[1].c_str();
    }else
        return -1;

    try {
        hci_dev_to_int = BLEd::HCIToInt(tokens[2]);
    }catch (string msg){
        sendSocket(msg, socket_desc);
        sendSocket("usage: remap <bd-address> <to-hci>\n", socket_desc);
        return -1;
    }

    if(!strcmp(address.c_str(), "")){
        sendSocket("usage: remap <bd-address> <to-hci>\n", socket_desc);
        return -1;
    }



    NodeMappings::changeNodeMappingInfo(HwAddress(address, PUBLIC_ADDRESS), hci_dev_to_int);

    // Create an disconnect Command from the previous hci device
    // Command is <disconnect> <hci_dev> <bd_addr>
    ICommand* disconn_cmd = new ICommand(CMD_DISCONNECT, TARGET_ALL_HCI,
                                    {address});
    //Send the command for processing
    BLEHelper::pushCommand(disconn_cmd);

    return 0;

}

//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::cmdConUpdate(int socket_desc, std::vector<string> tokens) {

    /* Command Remap
     * remap <hci>  for  or
     * remap <hci>  or
     */

    /*if(!strcmp(tokens[0].c_str(), "")) {
        sendSocket("usage: con-update <hci-device> for using the assistant\n"
                   "       con-update <hci-device> <handle> \\"
                   "                  <con_int_min> <con_int_max> \\"
                   "                  <con_lat> <sup_timeout> \\"
                   "                  <min_ce_len> <max_ce_len\n", socket_desc);
        return 0;
    }
    */

    //TODO Convert the hi device in a better way: a valid inout should be hci0, hci1, ...
    int hci_dev = 0;
    if(BLEd::stringToInt(tokens[1], hci_dev) < 0) {
        sendSocket("usage: con-update <hci-device> for using the assistant\n"
                   "       con-update <hci-device> <handle> \\ \n"
                   "                  <con_int_min> <con_int_max> \\ \n"
                   "                  <con_lat> <sup_timeout> \\ \n"
                   "                  <min_ce_len> <max_ce_len\n", socket_desc);
        return 0;
    }


    string input ;//tokens[3].c_str();
    float return_val = 0;
    int val_for_pushing = 0;
    vector<string> messages_to_send = {"Handle: ",
                                 "Connection Interval Min (ms): ",
                                 "Connection Interval Max (ms): ",
                                 "Conn Latency (hex): ",
                                 "Supervision Timeout (ms): ",
                                 "Minimum CE Length (hex): ",
                                 "Maximum CE Length (hex): "};


    std::vector<int> inputs; // Stores the user's input
    stringstream debugMsg;
    for(int i = 0; i < messages_to_send.size(); i++) {


        stringstream msg;
        msg << GREEN << messages_to_send[i] << RESET;
        sendSocket(msg.str(), socket_desc);
        if (receiveSocket(input, socket_desc) == DISCONNECTED_FROM_SOCKET)
            return DISCONNECTED_FROM_SOCKET;
        if (!strcmp(input.c_str(), ""))
            return 0;

// If its the Connection Interval, convert the number to a Hex value
        if(i == 0) {

            if (BLEd::stringToFloat(input, return_val) < 0) {
                sendSocket("Given input was not an number, retry!\n", socket_desc);
                i--;
                continue;
            } else {
               // debugMsg << "IN: " << return_val;
                val_for_pushing = static_cast<int>(return_val);
            }
        }
        else if(i == 1 || i == 2) {

            if (BLEd::stringToFloat(input, return_val) < 0) {
                sendSocket("Given input was not an number, retry!\n", socket_desc);
                i--;
                continue;
            } else {
                //debugMsg << "IN: " << return_val;

                return_val /= 1.25;
                //debugMsg << " CONV: " << return_val;

                val_for_pushing = static_cast<int>(return_val);
            }
        }
        else if(i == 3 || i == 5 || i == 6){

            if (BLEd::hexToInt(input, val_for_pushing) < 0) {
                sendSocket("Given input was not an hex value, retry!\n", socket_desc);
                i--;
                continue;
            } else {
                //debugMsg << "IN: " << val_for_pushing;
            }
        }
        else if(i == 4) {

            if (BLEd::stringToFloat(input, return_val) < 0) {
                sendSocket("Given input was not an number, retry!\n", socket_desc);
                i--;
                continue;
            } else {
               // debugMsg << "IN: " << return_val;

                return_val /= 10;
               // debugMsg << " CONV: " << return_val;

                val_for_pushing = static_cast<int>(return_val);
            }
        }


        //debugMsg << " PUSH(dec): " << val_for_pushing;
       // debugMsg << " PUSH(hex): 0x" << std::hex << val_for_pushing;
        uint16_t higher_ = static_cast<uint16_t>(val_for_pushing & 0xff);
        uint16_t lower_ = static_cast<uint16_t>(val_for_pushing >> 8);
        //debugMsg << " PUSH(split): 0x" << higher_ << " 0x" << lower_ << std::endl;
        debugMsg << " 0x" << higher_ << " 0x" << lower_;

        inputs.push_back(val_for_pushing);
    }
    debugMsg << std::endl;
    //sendSocket(debugMsg.str(), socket_desc);
    // Create an disconnect Command from the previous hci device
    // Command is <disconnect> <hci_dev> <bd_addr>
    ICommand* conn_update_cmd = new
            ICommand(CMD_CON_UPDATE, hci_dev,
                             inputs);
    //Send the command for processing
    BLEHelper::pushCommand(conn_update_cmd);
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::cmdDisconnect(int socket_desc, std::vector<string> tokens){

    /* Command Remap
     * disconnect <bd-address>
     */

    string address = tokens[1].c_str();

    NodeMappings::changeNodeMappingInfo(HwAddress(address, PUBLIC_ADDRESS), NODE_DISABLED);


    // Create an disconnect Command from the previous hci device
    // Command is <disconnect> <hci_dev> <bd_addr>
    // We want to target all devices, so we set the hci to 'TARGET_ALL_HCI'
    ICommand* disconn_cmd = new ICommand(CMD_DISCONNECT, TARGET_ALL_HCI,
                                                    {address});
    //Send the command for processing
    BLEHelper::pushCommand(disconn_cmd);

}

//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::cmdConnect(int socket_desc, std::vector<string> tokens){

    int target_hci_dev = -1;

    if(tokens.size() <= 1
            || !strcmp(tokens[1].c_str(), "")
            || BLEd::stringToInt(tokens[2], target_hci_dev) < 0) {
        sendSocket("Connects the given node to an HCI device.\n"
                   "usage: connect <BD-address> will connect to the previous hci device\n"
                   "       connect <BD-address> <hci-device> will connect to the specifiedd hci device\n", socket_desc);
        return -1;
    }




    string address = tokens[1].c_str();

    if(target_hci_dev >= 0) {

        NodeMappings::changeNodeMappingInfo(HwAddress(address, PUBLIC_ADDRESS), target_hci_dev);
    }
   return 0;


}

//----------------------------------------------------------------------------------------------------------------------
int InteractionInterface::cmdHelp(int socket_desc){
    stringstream help_message;
    help_message << std::endl;

    help_message  << BLUE << DESCRIPTION << " v " << VERSION_MAJOR << "."
                  << VERSION_MINOR << "." << VERSION_REVISION << VERSION_BUILD
                  << "(" << STATE << ")" << RESET << std::endl;

    help_message << std::endl << AUTHOR << std::endl;

    help_message << "Usage"  << std::endl;
    help_message << BLUE << "connect    " << RESET << "<BD-address> <hci-device>" << std::endl;
    help_message << BLUE << "disconnect " << RESET << "<bd-address>"              << std::endl;
    help_message << BLUE << "remap      " << RESET << "<BD-Address> <to-hci>"     << std::endl;
    help_message << BLUE << "con-update " << RESET << "<hci-device>"              << std::endl;
    sendSocket(help_message.str(), socket_desc);
}


int InteractionInterface::protoGetNodes(int socket_desc, std::vector<string> tokens){

    /* proto-get_list <hci-dev>
     * */
    int target_hci_dev = -1;
    BLEd::stringToInt(tokens[1], target_hci_dev);

    if(target_hci_dev < 0)
        return -1;

    // Create an disconnect Command from the previous hci device
    // Command is <disconnect> <hci_dev> <bd_addr>
    // We want to target all devices, so we set the hci to 'TARGET_ALL_HCI'
    ICommand* disconn_cmd = new ICommand(PROTO_GET_NODE_LIST, target_hci_dev,
                                    {socket_desc});
    //Send the command for processing
    BLEHelper::pushCommand(disconn_cmd);


}


int InteractionInterface::cmdList(int socket_desc, std::vector<string> tokens){
    std::mutex locked;
    /* proto-get_list <hci-dev>
     * */
    int target_hci_dev = -1;
    try {
        target_hci_dev = BLEd::HCIToInt(tokens[1]);
    } catch (string msg) {
        sendSocket(msg, socket_desc);
        return -1;
    }
    Logger::write({"User requesting list for hci", std::to_string(target_hci_dev)}, INFO_MSG, BLE_LOG_FILE);

    if(target_hci_dev < 0)
        return -1;

    // Create an list Command
    ICommand *list_cmd = new ICommand(CMD_GET_NODE_LIST, target_hci_dev,
                                    {socket_desc});


    list_cmd->setInterfaceLock(&locked);
    //Send the command for processing
    BLEHelper::pushCommand(list_cmd);

    // Wait until the BLEHelper unlocks the variable
    locked.lock();

    locked.unlock();
    return 0;
}








//---------------------------------------------------------------------------------------------------------------------
// For using the pipe
int InteractionInterface::initPipe(int hci_dev_no){

    string fifo_path= "home/developer/workspace";
    fifo_file_name = fifo_path + "bleControl_hci" + std::to_string(hci_dev_no);

    if (mkfifo (fifo_file_name.c_str(), 0666) < 0) {
        /* FIFO bereits vorhanden - kein fataler Fehler */
        if(errno == EEXIST)
            Logger::write("Versuche, vorh. FIFO zu verwenden", INFO_MSG, hci_dev_no);
        else {
            Logger::write("mkfifio()", ERROR_MSG, hci_dev_no);
            exit (EXIT_FAILURE);
        }
    }

    /* Empfänger liest nur aus dem FIFO */
    int fd = open(fifo_file_name.c_str(), O_RDONLY);
    Logger::write("FIFO opend", INFO_MSG, hci_dev_no);
    if (fd == -1) {
        Logger::write("open()", ERROR_MSG, hci_dev_no);
        exit (EXIT_FAILURE);
    }
    return 0;

}






















