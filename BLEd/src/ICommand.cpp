//
// Created by developer on 22.06.19.
//

#include "ICommand.h"
#include "Logger.h"
#include "BLEHelper.h"


// If an ICommand target all interface we need to track wich halper already tried
// processing the command
// Every new helper will increment this variable, if the constructor of an BLEHelper
// gets call it will be decremented.
static int no_of_ble_helpers_ = 0;
int ble_helper_counter_ = 0;

int ICommand::assignBLEHelperInstances(){
    std::vector<int> ble_inst= BLEHelper::getBLEHelperInstances();
    Logger::write({"New ICommand created, initialized for ",
                   std::to_string(ble_inst.size()), " helper instances"}, INFO_MSG, BLE_LOG_FILE);

    for (int i = 0; i < ble_inst.size(); ++i) {
        std::pair<int, bool> tmp_p;
        tmp_p.first = ble_inst[i];
        tmp_p.second = false;
        parsed_by_ble_helper_instances_.push_back(tmp_p);

    }
}
//---------------------------------------------------------------------------------------------------------------------
ICommand::ICommand(int command_desc, int target_hci_dev, initializer_list<int> args):
        command_desc_(command_desc), target_hci_dev_(target_hci_dev)  {

    for (auto args_to_push : args)
        args_int_.push_back(args_to_push);

    ble_helper_counter_ = no_of_ble_helpers_;
    assignBLEHelperInstances();
}

//---------------------------------------------------------------------------------------------------------------------
ICommand::ICommand(int command_desc, int target_hci_dev, initializer_list<string> args):
        command_desc_(command_desc), target_hci_dev_(target_hci_dev)  {

    for (auto args_to_push : args)
        args_string_.push_back(args_to_push);

    ble_helper_counter_ = no_of_ble_helpers_;
    lock_variable_ = new std::mutex;
    assignBLEHelperInstances();
}


//---------------------------------------------------------------------------------------------------------------------
ICommand::ICommand(int command_desc, int target_hci_dev, vector<int> &args):
        command_desc_(command_desc), target_hci_dev_(target_hci_dev)  {

    for (int i = 0; i < args.size(); ++i) {
        args_int_.push_back(args[i]);
    }

    ble_helper_counter_ = no_of_ble_helpers_;
    lock_variable_ = new std::mutex;
    assignBLEHelperInstances();
}





//---------------------------------------------------------------------------------------------------------------------
int ICommand::args(int pos, string& args){
    if(args_string_.size() == 0)
        return -1;

    args = args_string_[pos];
    return 0;
}


//---------------------------------------------------------------------------------------------------------------------
int ICommand::args(int pos, int& args){

    if(args_int_.size() == 0)
        return -1;
    char buf[512];
    sprintf(buf, "%x - %d", args_int_[pos], args_int_[pos]);
    args = args_int_[pos];
    return 0;
}


//---------------------------------------------------------------------------------------------------------------------
int ICommand::description(){
    return command_desc_;
}


//---------------------------------------------------------------------------------------------------------------------
int ICommand::targetHci(){
    return  target_hci_dev_;
}


//---------------------------------------------------------------------------------------------------------------------
int ICommand::setCommandParsedBy(int hci){
    for (int i = 0; i < parsed_by_ble_helper_instances_.size(); ++i) {
        if(parsed_by_ble_helper_instances_[i].first == hci) {
            parsed_by_ble_helper_instances_[i].second = true;
            Logger::write({"Command parsed by hci",
                           std::to_string(parsed_by_ble_helper_instances_[i].first),
                           ". Total of ",
                           std::to_string(parsed_by_ble_helper_instances_.size()), " devices"},
                                   INFO_MSG, BLE_LOG_FILE);
            return 0;
        }
    }
return 0;
}

bool ICommand::parsedByAllHelpers(){
    for (int i = 0; i < parsed_by_ble_helper_instances_.size(); ++i) {

        if(parsed_by_ble_helper_instances_[i].second) {
            Logger::write({"ICommand for hci",
                           std::to_string(parsed_by_ble_helper_instances_[i].first), " parsed: YES"},
                          INFO_MSG, BLE_LOG_FILE);
        }
        else if (!parsed_by_ble_helper_instances_[i].second) {
            Logger::write({"ICommand for hci",
                           std::to_string(parsed_by_ble_helper_instances_[i].first), " parsed: NO"},
                          INFO_MSG, BLE_LOG_FILE);
            return false;
        }
    }
    return true;
}

int ICommand::decrementBLEHelperCounter(){
    // If this is zero, all helper classes tried to parse the command
    Logger::write({"ICommand left: ",
                   std::to_string(ble_helper_counter_)}, INFO_MSG, BLE_LOG_FILE);
    return  --ble_helper_counter_;
}


//---------------------------------------------------------------------------------------------------------------------
int ICommand::incrementHCI(){
    return ++no_of_ble_helpers_;
};


//---------------------------------------------------------------------------------------------------------------------
int ICommand::decrementHCI() {

    if(no_of_ble_helpers_ > 0)
        return --no_of_ble_helpers_;
    else
        return -1;
};


int ICommand::setInterfaceLock(std::mutex *locked) {

    Logger::write({"ICommand requested InteractionInterface lock. ",
                   "Wait for precessing the command by one of the Helpers"}, INFO_MSG, BLE_LOG_FILE);
    command_locked = true;
    lock_variable_ = locked;

    if(lock_variable_ != nullptr)
        lock_variable_->lock();
    return 0;
}

int ICommand::unsetInterfaceLock() {

    if(lock_variable_ != nullptr)
        lock_variable_->unlock();

    Logger::write({"ICommand releasing InteractionInterface lock."}, INFO_MSG, BLE_LOG_FILE);
    return 0;
}

//int ICommand::unseInterfaceLock() {
//       lock_variable_->unlock();
//};

ICommand::~ICommand() {
    Logger::write({" -> Delete Command"}, INFO_MSG, BLE_LOG_FILE);
    if(command_locked)
        unsetInterfaceLock();
}