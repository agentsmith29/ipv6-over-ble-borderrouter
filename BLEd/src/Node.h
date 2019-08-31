//
// Created by developer on 06.06.19.
//

#ifndef BLED_NODE_H
#define BLED_NODE_H
#include <string>

#include "HwAddress.h"

using std::string;

static constexpr int NOT_CONNECTED = 0;
static constexpr int CONNECTABLE = 1;
static constexpr int CONNECTED = 2;
static constexpr int DISCONNECTED = 3;
static constexpr int FAILED_CONNECTION = 4;
static constexpr int DISABLED = 5;


//static  constexpr int
class Node {

public:


    Node(string name, HwAddress hw_address, string ipv6_address, bool connect);

    Node(string name, HwAddress hw_address, bool connect);


    // Getter und Setter
    string getName();
    string getIPv6_address();
    string getAddress();




    /// Returns the HW address Type and address
    HwAddress getFullAddressData();


    /// Returns a the description for the Address type
    /// \param as_number Display as a number
    /// \return A description fo the address type
    string getAddressType(bool as_number);

    /// Returns the number of the Address type
    /// \param as_number Display as a number
    /// \return A description fo the address type
    int getAddressType();

    int setName(string name);

    int setIPv6_address(string ipv6_address);

    void setConnectionState(int connectionState);

    int connectNode(bool connect);

    /// Returns the connection state for the given Node
    /// Valid states are zero (0) to four (4) with the following returns:
    /// \return 0 Node is NOT CONNECTED. No connection attempt has been made
    /// \return 1 Node is CONNECTED. A successful connection attempt has been made
    /// \return 2 Node was connected but has been DISCONNECTED. The node got disconnected
    /// \return 3 Node connection attempt FAILED. The given node could not establish a connection
    /// \return 4 Node should be IGNORED. The user added this node for ignoring
    int getConnectionState();

    int setConnectionInfo(uint16_t handle, uint8_t type, uint8_t out, uint16_t state, uint32_t link_mode);


    int setHCIDev(int hci_dev);

    int getHCIDev();


    // Node Connection Info
    struct ConnectionInfo {
        uint16_t handle;
        uint8_t  type;
        uint8_t	 out;
        uint16_t state;
        uint32_t link_mode;
    };

    int getNodeHandle();

    int getNodeType();

    int getNodeState();

    int getNodeLinkMode();

    int getConnectionRetry();
    int incrementConnectionRetry();



    // Todo change this struct, values are wrong
    struct ConnectionParameters {
        int conn_interval_min;
        int conn_interval_max;
        int conn_latency;
        int supervision_timeout;
        int minimum_ce_length;
        int maximum_ce_length;

    };

    int getConnectionIntervalMin();

    int getConnectionIntervalMax();

    int getConnectionLatency();

    int getSupervisionTimeout();

    int getMinimumCELength();

    int getMaximumCELength();

    ConnectionInfo getConnectionInfo();

    int setConnectionsParameter(ConnectionParameters conn_params);

private:

    int random_id_;

    int connection_retrys_ = 0, connection_state_ = DISCONNECTED, conn_to_hci_dev_ = -1;

    HwAddress hw_address_;

    string name_, ipv6_address_, localLinkAddress_;

    bool connect_node_, node_connected_;



    // Stores informations about handle, type, etc.
    ConnectionInfo conn_info_;

    // Stores information about the connection itself, e. g. min_event, max_event, etc
    ConnectionParameters conn_parameters_;


    /// This will create a connection Update parameter for the desired node
    /// \param params_for_update
    /// \return
    int createConnParamUpdateEvent(ConnectionParameters params_for_update);

};


#endif //BLED_NODE_H
