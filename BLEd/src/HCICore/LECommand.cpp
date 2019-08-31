//
// Created by developer on 05.07.19.
//

#include <sstream>

#include "HCIInterface.h"
#include "../Logger.h"

void hex_dump(char *pref, int width, unsigned char *buf, int len);



//----------------------------------------------------------------------------------------------------------------------
int HciInterface::sendCommand(int dev_id, initializer_list<uint16_t> args) {

    Logger::write({"Sending given command"}, INFO_MSG, BLE_LOG_FILE);
    // Convert args
    vector<char> args_;

    for (auto str : args)
        args_.push_back(str);


    unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr = buf;
    struct hci_filter flt;
    hci_event_hdr *hdr;
    int i, opt, len, dd;
    uint16_t ocf = args_[1];
    uint8_t ogf = args_[0];



    // helper_arg(2, -1, &argc, &argv, cmd_help);

    if (dev_id < 0)
        dev_id = hci_get_route(NULL);

    errno = 0;



    if (errno == ERANGE || (ogf > 0x3f) || (ocf > 0x3ff)) {
        Logger::write({"  └─> Error: ", strerror(errno)}, ERROR_MSG, BLE_LOG_FILE);
        return 0;
    }



    for (i = 2, len = 0; i < args_.size() && len < (int) sizeof(buf); i++, len++)
        *ptr++ = args_[i];

    dd = hci_open_dev(dev_id);
    if (dd < 0) {
        Logger::write("  └─> Device open failed", ERROR_MSG, BLE_LOG_FILE);
        return(EXIT_FAILURE);
    }

    /* Setup filter */
    hci_filter_clear(&flt);
    hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
    hci_filter_all_events(&flt);
    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
        Logger::write("  └─> HCI filter setup failed", ERROR_MSG, BLE_LOG_FILE);
        return(EXIT_FAILURE);
    }

    char buft1[512];
    sprintf(buft1, "  ├─> Sent HCI Command: ogf 0x%02x, ocf 0x%04x, plen %d", ogf, ocf, len);
    Logger::write({buft1}, INFO_MSG, BLE_LOG_FILE);

    hex_dump("  ", 20, buf, len); //fflush(stdout);
    //strin ss = buf;
    //Logger::write(, INFO_MSG, BLE_LOG_FILE);

    if (hci_send_cmd(dd, ogf, ocf, len, buf) < 0) {
        Logger::write("  └─> Send failed", ERROR_MSG, BLE_LOG_FILE);
        return (EXIT_FAILURE);
    }

    len = read(dd, buf, sizeof(buf));
    if (len < 0) {
        Logger::write("  └─> Read failed", ERROR_MSG, BLE_LOG_FILE);
        perror("Read failed");
        return (EXIT_FAILURE);
    }

    hdr = (hci_event_hdr *)(buf + 1);
    ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
    len -= (1 + HCI_EVENT_HDR_SIZE);

    char buft2[512];
    sprintf(buft2, "  ├─> Receive HCI Event: 0x%02x plen %d", hdr->evt, hdr->plen);
    Logger::write({buft2}, INFO_MSG, BLE_LOG_FILE);
    hex_dump("  ", 20, ptr, len); fflush(stdout);


    hci_close_dev(dd);
    return 0;
}


//----------------------------------------------------------------------------------------------------------------------
void hex_dump(char *pref, int width, unsigned char *buf, int len) {
    register int i,n;
    char buf_n[512];
    std::stringstream buf_t;


    for (i = 0, n = 1; i < len; i++, n++) {
        if (n == 1) {
            sprintf(buf_n, "%s", pref);
            buf_t << " " << buf_n;
        }
        sprintf(buf_n, "%2.2x", buf[i]);
        buf_t << " " << buf_n;
        if (n == width) {
            buf_t << "\n";
            n = 0;
        }
    }
    if (i && n!=1)
        buf_t << "\n";

    Logger::write({"  ├─> ", buf_t.str()}, INFO_MSG, BLE_LOG_FILE);
}