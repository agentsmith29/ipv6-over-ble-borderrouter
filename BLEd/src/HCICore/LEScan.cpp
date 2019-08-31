//
// Created by developer on 05.07.19.
//

#include "HCIInterface.h"
#include "../Logger.h"
#include "../NodeMappings.h"

#include <signal.h>
#include <sstream>



volatile int signal_received = 0;
void sigint_handler(int sig) {
    signal_received = sig;
}


int HciInterface::permanentScan(char opt){

    while(true){
        startScan(opt);
        sleep(1);
    }
    return 0;
}


int HciInterface::startScan(char opt) {


    Logger::write({"Started scanning on hci", std::to_string(hci_dev_id_)},
                  INFO_MSG, HCI_LOG_FILE);

    int err, dd;
    uint8_t own_type = LE_PUBLIC_ADDRESS;
    uint8_t scan_type = 0x01;
    uint8_t filter_type = 0;
    uint8_t filter_policy = 0x00;
    uint16_t interval = htobs(0x0010);
    uint16_t window = htobs(0x0010);
    uint8_t filter_dup = 0x01;

    switch (opt) {
        case 's':
            own_type = LE_RANDOM_ADDRESS;
            break;
        case 'p':
            own_type = LE_RANDOM_ADDRESS;
            break;
        case 'P':
            scan_type = 0x00; /* Passive */
            break;
        case 'w':
            filter_policy = 0x01; /* Whitelist */
            break;
        case 'd':
            filter_type = optarg[0];
            if (filter_type != 'g' && filter_type != 'l') {
                Logger::write("Unknown discovery procedure",
                              ERROR_MSG, BLE_LOG_FILE);
                exit(1);
            }

            interval = htobs(0x0012);
            window = htobs(0x0012);
            break;
        case 'D':
            filter_dup = 0x00;
            break;
    }


    if (hci_dev_id_ < 0)
        hci_dev_id_ = hci_get_route(NULL);


    dd = hci_open_dev(hci_dev_id_);


    if (dd < 0) {
        Logger::write({"Could not open device hci", std::to_string(hci_dev_id_), ":",
                       strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        hci_close_dev(hci_dev_id_);
        return ERR_HCITOOL_OPENING_DEVICE;
    }

    err = hci_le_set_scan_parameters(dd, scan_type, interval, window,
                                     own_type, filter_policy, 10000);

    if (err < 0) {
        Logger::write({"Set scan parameters failed: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        hci_close_dev(hci_dev_id_);
        return ERR_HCITOOL_SETTING_SCAN_PARAMS;
    }

    err = hci_le_set_scan_enable(dd, 0x01, filter_dup, 10000);

    if (err < 0) {
        Logger::write({"Enable scan failed: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        hci_close_dev(hci_dev_id_);
        return ERR_HCITOOL_ENABLE_SCAN_FAILED;
    }
    err = scanAdvertisingDevices(dd, (int) filter_type);


    if (err < 0) {
        Logger::write({"Could not receive advertising events: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        hci_close_dev(hci_dev_id_);
        return ERR_HCITOOL_NO_ADVERTISING_REPORT;
    }

    err = hci_le_set_scan_enable(dd, 0x00, filter_dup, 10000);


    if (err < 0) {
        Logger::write({"Disable scan failed: ", strerror(errno)},
                      ERROR_MSG, BLE_LOG_FILE);
        hci_close_dev(hci_dev_id_);
        return ERR_HCITOOL_DISABLE_SCAN_FAILED;
    }

    hci_close_dev(dd);
    Logger::write("HCI closed", ERROR_MSG, HCI_LOG_FILE);
    return 0;
}



//----------------------------------------------------------------------------------------------------------------------
int HciInterface::scanAdvertisingDevices(int dd, int filter_type) {

    Logger::write("Scan advertising devices", INFO_MSG, HCI_LOG_FILE);

    unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
    struct hci_filter nf, of;
    struct sigaction sa; //
    socklen_t olen;
    int len;

    data_dump tmp_data_dump; // For converting thr data dump

    olen = sizeof(of);
    if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
        Logger::write("Could not get socket options",
                      ERROR_MSG, HCI_LOG_FILE);
        return -1;
    }

    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
        Logger::write("Could not get socket options",
                      ERROR_MSG, HCI_LOG_FILE);
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_NOCLDSTOP;
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);


    while (true) {

        evt_le_meta_event *meta;
        le_advertising_info *info;
        char addr[18];


        while ((len = read(dd, buf, sizeof(buf))) < 0) {

            if (errno == EINTR && signal_received == SIGINT) {
                Logger::write({"Interrupt System call: ", strerror(errno)},
                              DEBUG_MSG, HCI_LOG_FILE);
                len = 0;
                setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

                if (len < 0)
                    return -1;

                return 0;
            }

            if (errno == EAGAIN || errno == EINTR)
                continue;

            setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));
            Logger::write({"GOTO CALLED, would continue now: "}, DEBUG_MSG, BLE_LOG_FILE);
            if (len < 0)
                return -1;

            return 0;
        }


        ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
        len -= (1 + HCI_EVENT_HDR_SIZE);

        meta = (evt_le_meta_event *) ptr;


        /* References can be found here:
         * https://www.bluetooth.org/en-us/specification/adopted-specifications - Core specification 4.1
         * [vol 2] Part E (Section 7.7.65) - Le Meta Event
         */

        if (meta->subevent != 0x02) {
            std::stringstream str;
            str << "0x" << std::uppercase << std::hex << (int) meta->subevent << " end";
            Logger::write({"meta subevent received: ", str.str()}, DEBUG_MSG, HCI_LOG_FILE);
            str.clear();

            //Set the socket
            setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

            if (len < 0)
                return -1;

            return 0;
        }


        /* Ignoring multiple reports */
        info = (le_advertising_info *) (meta->data + 1);
        if (checkReportFilter(filter_type, info)) {
            char name[30];

            memset(name, 0, sizeof(name));

            ba2str(&info->bdaddr, addr);

            parseNodeName(info->data, info->length,
                          name, sizeof(name) - 1);

            bool ipv6_func = checkIPv6Flag(info);

            // dev_list.push_back(addr);
            // Logger::writeTmpTmp(addr, DEBUG_MSG);
            HwAddress tmp_hw_add(addr, PUBLIC_ADDRESS);

            // Get the number of the hci-device to map to
            int map_to_hci_device = NodeMappings::returnNodeMappingInfo(tmp_hw_add);


            if (ipv6_func &&
                (map_to_hci_device == hci_dev_id_)) {
                Logger::write({" * FOUND: ", addr, " - IPv6"},
                              INFO_MSG, HCI_LOG_FILE);
                addIPv6NodeToBuffer(tmp_hw_add, name); // Try to add the node (with filtering)
            } else if (strcmp(name, "") && (map_to_hci_device == hci_dev_id_)) {
                tryUpdateNodesName(tmp_hw_add, name);
                Logger::write({" -> FOUND: ", addr, " Name: ", name},
                              INFO_MSG, HCI_LOG_FILE);
            } else {

                if (ipv6_func) {
                    Logger::write({" x FOUND: ", addr, " Name: ", name, "-> IPV6 -> hci",
                                   std::to_string(map_to_hci_device), "/hci", std::to_string(hci_dev_id_)},
                                  INFO_MSG, HCI_LOG_FILE);
                } else {
                    Logger::write({" x FOUND: ", addr, " Name: ", name},
                                  INFO_MSG, HCI_LOG_FILE);
                }
            }
            //usleep(250000);

        }
    }
}


//----------------------------------------------------------------------------------------------------------------------
void HciInterface::parseNodeName(uint8_t *eir, size_t eir_len, char *buf, size_t buf_len) {
    size_t offset;

    offset = 0;
    while (offset < eir_len) {
        uint8_t field_len = eir[0];
        size_t name_len;

        /* Check for the end of EIR */
        if (field_len == 0)
            break;

        if (offset + field_len > eir_len)
            snprintf(buf, buf_len, "(unknown)");

        switch (eir[1]) {
            case EIR_NAME_SHORT:
            case EIR_NAME_COMPLETE:
                name_len = field_len - 1;
                if (name_len > buf_len)
                    snprintf(buf, buf_len, "(unknown)");

                memcpy(buf, &eir[2], name_len);
                return;
        }

        offset += field_len + 1;
        eir += field_len + 1;
    }
}


//----------------------------------------------------------------------------------------------------------------------
bool HciInterface::checkIPv6Flag(le_advertising_info *info){
    int offset = 0;

    while (offset < info->length) {
        int eir_data_len = info->data[offset];
        if (extInquiryDataDump(&info->data[offset]).type == 0x03 && //COMPLETE_LIST_16BIT_UUID
        extInquiryDataDump(&info->data[offset]).data[1] == 0x18 &&
                extInquiryDataDump(&info->data[offset]).data[0] == 0x20) {

            //If we found an node, we will ad this to the list
            return true;
        }

        offset += eir_data_len + 1;
    }
    return false;
}


//----------------------------------------------------------------------------------------------------------------------
HciInterface::data_dump HciInterface::extInquiryDataDump(uint8_t *data) {


    data_dump ret_data_dump;
    ret_data_dump.type = 0x00;
    ret_data_dump.print_data = "Not init";

    char buf[128];
    uint8_t len = data[0];
    uint8_t type;
    char *str;
    int i;

    if (len == 0)
        return ret_data_dump;

    type = data[1];
    data += 2;
    len -= 1;
    std::string description;
    switch (type) {
        case 0x01:
            //p_indent(level, frm);
            description = "Flags: ";
            for (i = 0; i < len; i++) {
                ret_data_dump.data.push_back(data[i]);
                sprintf(buf, "0x%2.2x", data[i]);
            }
            ret_data_dump.type = 0x01;
            ret_data_dump.print_data = description.append(buf);
            return ret_data_dump;

        case 0x02:
        case 0x03:
            sprintf(buf, "%s service classes:",
                    type == 0x02 ? "Shortened" : "Complete");
            description = buf;

            for (i = 0; i < len; i++) {
                ret_data_dump.data.push_back(data[i]);
                sprintf(buf, "0x%2.2x%2.2x", data[i], (data - 1)[i]);
            }

            ret_data_dump.type = 0x03;
            ret_data_dump.print_data = description.append(buf);
            return ret_data_dump;

        case 0x08:
        case 0x09:
            str = (char *) malloc(len + 1);
            if (str) {
                snprintf(str, len + 1, "%s", (char *) data);
                for (i = 0; i < len; i++)
                    if (!isprint(str[i]))
                        str[i] = '.';
                sprintf(buf, "%s local name: \'%s\'",
                        type == 0x08 ? "Shortened" : "Complete", str);
                free(str);
            }
            ret_data_dump.type = 0x09;
            ret_data_dump.data_s = str;
            ret_data_dump.print_data = buf;
            return ret_data_dump;

        case 0x0a:
            // p_indent(level, frm);
            sprintf(buf, "TX power level: %d", *((uint8_t *) data));
            ret_data_dump.type = 0x0a;
            ret_data_dump.data_s = *((uint8_t *) data);
            ret_data_dump.print_data = buf;
            return ret_data_dump;

        default:
            sprintf(buf, "Unknown type 0x%02x with %d bytes data",
                    type, len);
            ret_data_dump.type = 0xff;
            ret_data_dump.data_s = type;
            ret_data_dump.print_data = buf;
            return ret_data_dump;
    }
}


//----------------------------------------------------------------------------------------------------------------------
int HciInterface::checkReportFilter(uint8_t procedure, le_advertising_info *info) {
    uint8_t flags;

    /* If no discovery procedure is set, all reports are treat as valid */
    if (procedure == 0)
        return 1;

    /* Read flags AD type value from the advertising report if it exists */
    if (read_flags(&flags, info->data, info->length))
        return 0;

    switch (procedure) {
        case 'l': /* Limited Discovery Procedure */
            if (flags & FLAGS_LIMITED_MODE_BIT)
                return 1;
            break;
        case 'g': /* General Discovery Procedure */
            if (flags & (FLAGS_LIMITED_MODE_BIT | FLAGS_GENERAL_MODE_BIT))
                return 1;
            break;
        default:
            fprintf(stderr, "Unknown discovery procedure\n");
    }

    return 0;
}


//----------------------------------------------------------------------------------------------------------------------
int HciInterface::read_flags(uint8_t *flags, const uint8_t *data, size_t size){
    size_t offset;

    if (!flags || !data)
        return -EINVAL;

    offset = 0;
    while (offset < size) {
        uint8_t len = data[offset];
        uint8_t type;

        /* Check if it is the end of the significant part */
        if (len == 0)
            break;

        if (len + offset > size)
            break;

        type = data[offset + 1];

        if (type == FLAGS_AD_TYPE) {
            *flags = data[offset + 2];
            return 0;
        }

        offset += 1 + len;
    }

    return -ENOENT;
}

