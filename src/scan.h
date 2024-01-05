#ifndef SCAN_H
#define SCAN_H
// bluetooth.h is needed as hci.h does not include e.g. inttypes.h, or a definition of bdaddr_t
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

void receive_event_loop(int device_handle, void (**event_handlers)(const le_advertising_info *));
#endif // SCAN_H
