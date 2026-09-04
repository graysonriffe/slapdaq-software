
//necessary imports
#include "wifi.h"

void wifi_initialize(void) {
    ESP_LOGI(TAG, "Initializing...");

    //Initializing NVS
    esp_err_t nvs_ret = nvs_flash_init();

    if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(nvs_ret);

    //Initializing Wifi SoftAP
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    //Statically set IP, default gateway, and subnet mask
    esp_netif_t* apIF = esp_netif_create_default_wifi_ap();
    esp_netif_dhcps_stop(apIF); //DHCP automatically assigns IP, gateway, and DNS info ; we want to configure these manually so we stop DHCP server

    esp_netif_ip_info_t apIP = {};
    apIP.ip.addr = ESP_IP4TOADDR(10, 0, 0, 1);
    apIP.gw.addr = apIP.ip.addr;
    apIP.netmask.addr = ESP_IP4TOADDR(255, 255, 255, 0);
    esp_netif_set_ip_info(apIF, &apIP);

    esp_netif_dhcps_start(apIF);

    //Initalize Wifi module
    wifi_init_config_t wifiInitConfig = {};
    wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitConfig));

    //Configure the initialized wifi module
    wifi_config_t wifiConfig = {};
    strcpy((char*)wifiConfig.ap.ssid, WIFI_SSID);
    strcpy((char*)wifiConfig.ap.password, WIFI_PASSWORD);
    wifiConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifiConfig.ap.max_connection = 2;
    wifiConfig.ap.channel = 1;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    //Create the socket for TCP communication
    ESP_LOGI(TAG, "Creating socket...");

    int socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (socketFD < 0) {
        ESP_LOGE(TAG, "Could not create socket.");
        return;
    }

    //Define IPv4 information for server
    sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(15000);

    //Bind for listening
    int err = bind(socketFD, (sockaddr*)&serverAddress, sizeof(serverAddress));
    if (err != 0) {
        ESP_LOGE(TAG, "Could not bind socket!");
        close(socketFD);
        return;
    }

    err = listen(socketFD, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Could not start listening!");
        close(socketFD);
        return;
    }

    while (true) {
        ESP_LOGI(TAG, "Listening for TCP connections...");

        sockaddr_in clientAddress = {};
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(socketFD, (sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            ESP_LOGE(TAG, "Could not accept.");
            continue;
        }

        ESP_LOGI(TAG, "Got connection. Sending data...");



    }
}
