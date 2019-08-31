//
// Created by developer on 22.06.19.
//

#ifndef BLED_ICOMMAND_H
#define BLED_ICOMMAND_H

#include <vector>

#include "HwAddress.h"
#include <mutex>
#include <condition_variable>


using std::vector;
using std::initializer_list;



static constexpr int CMD_CONNECT = 1;
static constexpr int CMD_DISCONNECT = 2;
static constexpr int CMD_CON_UPDATE = 3;
static constexpr int CMD_GET_NODE_LIST = 4;

static constexpr int PROTO_GET_NODE_LIST = 5;




static constexpr int TARGET_ALL_HCI = -1;


class ICommand{

public:

    //ICommand<T>(int command_desc, int target_hci_dev, std::vector<T> args);

    ICommand(int command_desc, int target_hci_dev, initializer_list<int> args);

    ICommand(int command_desc, int target_hci_dev, initializer_list<string> args);

    ICommand(int command_desc, int target_hci_dev, vector<int> &args);

    ~ICommand();

    int description();

    int targetHci();

    int args(int pos, string& args);

    int args(int pos, int& args);


    /// Decrements the BLEHelper count which is initialized by the constructor with the value
    /// stored in ble_helper_counter_.
    /// This keeps track which of the BLEHelperd already tried to process a ICommand.
    /// If an BLE Helper was not able to process the given ICommand than counter should be decremented
    /// If no BLEHelper can process the command it should get discarded.
    /// \return
    int decrementBLEHelperCounter();

    int setInterfaceLock(std::mutex *locked) ;

    int unsetInterfaceLock();

    static int incrementHCI();

    static int decrementHCI();

    bool parsedByAllHelpers();

    int setCommandParsedBy(int hci);

private:
    int command_desc_;
    int target_hci_dev_;
    std::vector<string> args_string_;
    std::vector<int> args_int_;
    std::mutex *lock_variable_;
    bool command_locked = false;

    int assignBLEHelperInstances();

    /// Stores the instances of the BLE-Helpers and if the command got already parsed
    /// by the Helper.
    /// eg.
    /// 0, false -> BLEHelper fpr hci0 hasn't parsed the command
    /// 1, true -> BLEHelper for hci1 has already parsed the command
    /// If all entries are set to true, the command will get discarted
    std::vector<std::pair<int, bool>> parsed_by_ble_helper_instances_;



};




#endif //BLED_ICOMMAND_H
