//
// Created by developer on 06.06.19.
//

#ifndef BLED_INTERACTIONINTERFACE_H
#define BLED_INTERACTIONINTERFACE_H

#include <string>
#include <vector>
#include <arpa/inet.h>

using std::string;

static constexpr int DISCONNECTED_FROM_SOCKET = -255;

class InteractionInterface {


public:
    InteractionInterface();




private:

    const int max_socket_requests_ = 5;
    int current_socket_requests_ = 0;
    string fifo_file_name;


    // Socket Address
    sockaddr_in address_;
    // For configuration we use sockets. The socket accepts multiple connections
    // Socket handling


    /// Initializes a master socket. Binds the socket to localhost with the given port
    /// \param Port the bird to bind the socket
    /// \return Return 0 if success
    int initInterfaceSocket(int Port);


    /// Handles given incoming request and distributes responses to the subsockets
    /// \param master_socket_handle The handle to the master socket
    /// \return Returns 0 if success
    int handleSocketRequest(int socket, fd_set readfds);





    //------------------------------------------------------------------------------------------------------..----------
    /// Parses the given Command
    /// \param str
    /// \param socket_description
    /// \return
    int parseCommand(const string& str, int socket_description);


    void tokenize(std::string const &str, const char* delim,std::vector<std::string> &out);


    /// Provides an Interface for remap a device
    /// \param socket_desc
    /// \return
    int cmdRemap(int socket_desc, std::vector<string> tokens);


    /// TODO Description
    /// \param socket_desc
    /// \param tokens
    /// \return
    int cmdConUpdate(int socket_desc, std::vector<string> tokens);


    /// TODO Description
    /// \param socket_desc
    /// \param tokens
    /// \return
    int cmdDisconnect(int socket_desc, std::vector<string> tokens);


    /// TODO Description
    /// \param socket_desc
    /// \param tokens
    /// \return
    int cmdConnect(int socket_desc, std::vector<string> tokens);



    /// TODO Description
    /// \param socket_desc
    /// \param tokens
    /// \return
    int cmdHelp(int socket_desc);



    /// TODO Descripton
    /// \param socket_desc
    /// \param tokens
    /// \return
    int cmdList(int socket_desc, std::vector<string> tokens);


    /// TODO Description
    /// \param socket_desc
    /// \return
    int protoGetNodes(int socket_desc, std::vector<string> tokens);

    /// For sending socket message without the need of using pointers
    /// or temporary string objects
    /// \param message
    /// \param socket_desc
    /// \return
    int sendSocket(string message, int socket_desc);


    /// For receiving socket messages
    /// \param socket_desc
    /// \return
    int receiveSocket(string& message, int socket_desc);



    /// Convert the a string in the format hcix where x stands for the hci number
    /// to an number.
    /// \param input
    /// \param hci_dev_out
    /// \return
    int argToHci(string input, int &hci_dev_out);


    int authUser();

    //------------------------------------------------------------------------------------------------------------------
    /// Initializes the Pipe for configuration
    /// \param hci_dev_no
    /// \return
    int initPipe(int hci_dev_no);



};


#endif //BLED_INTERACTIONINTERFACE_H
