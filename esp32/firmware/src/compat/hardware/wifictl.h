/* Wi-Fi : version reduite de wifictl.h de la montre, reglages dans /wificfg.json sur la carte SD. */
#ifndef _WIFICTL_H
    #define _WIFICTL_H

    #define WIFICTL_CONFIG_FILE     "/wificfg.json"

    /**
     * @brief start connecting to the strongest known network of /wificfg.json (same file as the
     * watch: "networklist": [ { "ssid": "...", "psk": "..." } ]). Returns at once or after a scan
     * of a few seconds, the caller waits for WiFi.isConnected(). Call it from a worker task.
     * When connected, the clock is set by NTP (optional "tz" key: POSIX time zone, default Europe/Paris).
     */
    void wifictl_on( void );

#endif // _WIFICTL_H
