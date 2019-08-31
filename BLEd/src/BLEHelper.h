//
// Created by developer on 06.06.19.
//

#ifndef BLED_BLEHELPER_H
#define BLED_BLEHELPER_H

#include "NodesHandler.h"
#include "HCICore/HCIInterface.h"
#include "ICommand.h"

#include <mutex>
#include <condition_variable>




class BLEHelper {

#define KERNEL_MOD_ENABLE "/proc/ble/6lowpan_control"
#define KERNEL_MOD_CONTROL "/proc/ble/6lowpan_control"

public:

    BLEHelper(int hci_dev_no);

    BLEHelper(HwAddress hci_dev_address);

    ~BLEHelper();

    int scanAndConnect();

    /// Check the connection of all nodes
    /// \param hw_address
    /// \return
    int checkConnection(HwAddress hw_address);


    int addNodeToQueue(HwAddress address_of_node);


    static int resetHCI(int hci_device_no);


    /// Adds a new ICommand for parsing. All available BLEHelper instances will try to parse the given ICommand
    /// If an ICommand could not be parsed, it will get discarded by the last BLEHelper instance.
    /// \param command The ICommand for processing
    /// \return
    static int pushCommand(ICommand* command);


    /// Returns initiated BLEHelpers classes as an vector of numbers. Each element represents the ID of an HCI
    /// Devices an BLEHelper got initialized with.
    /// \return An vector of numbers, representing the initiated HCI Device
    static std::vector<int> &getBLEHelperInstances();

    /// Register an new BLEHelper Instance. This will add an new entry in the ble_helper_instances_ vector.
    /// \param hci_dev
    /// \return
    static int registerBLEHelperInstances(int hci_dev);


private:

    int hci_dev_no_;

    HwAddress hci_dev_address_;


    HciInterface *hci_scan;

    NodesHandler *nodes_handler_;

    bool err_init=false;

    int initHelper();


    /// Intializes the kernel module
    /// \return
    int initKernel();

    int load6lowpanModule();
    /// Initializes the kernel module by just sending the shell-command
    /// 'sudo hciconfig hci<n> reset'
    /// \param hci_device_no The number of the HCI-Device
    /// \return Returns 0 if the HCI-Device has been reset, otherwise -1 is returned
    int initHCIDev(int hci_device_no);

    /// Returns the BD Address of the given HCI-Device.
    /// This uses hciconfig for aquiring the address
    /// \param hci_device_no
    /// \return
    HwAddress getHCIDevBDAddress(int hci_device_no);

    /// Connect the node specified by it's address and type
    /// \param addr The hardware address of the node to connect
    /// \param connect Select if you want to Connect or Disconnect the given Node
    /// \return 0 if succeeded -1 otherwise
    int connectNode(Node &node_for_connection, bool connect);


    int disconnectNode(HwAddress address);

    /// Return the nodes with hw-address and connection state
    /// \return string for printing
    string printNodes();


    int selectDevice();



    /// If an command has been pushed for the device, this will
    /// call the desired function
    /// \return
    int executeNextCommand();

    /// Executes a disconnect command for remapping to another device
    /// This won't set the node to state 'ignoring'
    /// \return
    int executeDisconnect(ICommand &cmd);

    /// Executes a connection update command
    /// \return
    int executeConnectionUpdate(ICommand &cmd);

    int executeConnect();



    static ICommand *getTopCommand();

    static int popTopCommand();

    static int commandSize();


    /// Prints the list commands
    /// \param socket_description
    /// \param option
    /// \return
    int socket_listConnectedNodes(int socket_description, int option);





};


#endif //BLED_BLEHELPER_H
