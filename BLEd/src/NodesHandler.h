//
// Created by developer on 06.06.19.
//

#ifndef BLED_NODESHANDLER_H
#define BLED_NODESHANDLER_H

#include "Node.h"
#include "HwAddress.h"
#include <vector>
#include <string>
#include <iterator>

using std::vector;
using std::string;
using std::iterator;

static constexpr int LIST_CON_NODE_SIMPLE = 1;
static constexpr int LIST_CON_NODE_WITH_CON_PARAM = 2;

class NodesHandler {

private:

    int hci_dev_no;

    typedef vector<Node> NodesList;

    NodesList assigned_nodes_;



    /// Returns the position in the assigned_nodes_ buffer. If no node has been found, -1
    /// \param node_to_check
    /// \return
    int checkForNode(Node node_to_check);

public:


    NodesHandler(int hci_dev_no);


    int isNodeConnected(HwAddress hw_address);


    int isNodeAvailable(HwAddress hw_address);


    int removeNode(HwAddress hw_address);


    int addNode(Node &new_node);


    /// Add a Node from the given Information
    /// \param name
    /// \param hw_address
    /// \param ipv6_address
    /// \param connect
    /// \return
    int addNode(string name, string hw_address, string ipv6_address, bool connect);


    Node &getNode(HwAddress hw_address);


    int getHCIDevNumber();


    int updateNode(HwAddress hw_address, Node &node_for_update);


    int updateNodesConnectionState(HwAddress hw_address, int state);

    /// Updates the node Connection Parameters
    /// \param hw_address
    /// \param conn_params
    /// \return
    int updateNodesConnectionParameter(HwAddress hw_address, Node::ConnectionParameters conn_params);


    int size(){ return assigned_nodes_.size(); };


    typedef NodesList::iterator iterator;


    typedef NodesList::const_iterator const_iterator;


    iterator begin() { return assigned_nodes_.begin(); }


    iterator end() { return assigned_nodes_.end(); }


    Node &operator[](int index) {
        if (index >= size()) {
        }
        return assigned_nodes_[index];
    }

    /// Writes a protomessage to the socket with the Infromation about the nodes
    /// \param socket_description
    /// \return
    int proto_getConnectedNodes(int socket_description);



    int socket_listConnectedNodes(int socket_description, int option);


    HwAddress getAddressFromHandle(int handel);


};


#endif //BLED_NODESHANDLER_H
