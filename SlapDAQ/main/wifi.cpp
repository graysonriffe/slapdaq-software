
//necessary imports
#include "wifi.h"

static const char* WIFI_TAG = "wifi";

static int s_listenFD = -1;
static int s_clientFD = -1;

// variables to hold the bytes of partially received host packet
static uint8_t s_rxBuf[16];
static size_t s_rxLen = 0;

// close the current client (if any)
static void drop_client(void)
{
    if (s_clientFD >= 0)
    {
        close(s_clientFD);
        s_clientFD = -1;
    }
    s_rxLen = 0;
}

static void configure_client_socket(int fd)
{
    // send small packets instead of one large packet
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    // check to make sure TCP send doesn't delay measurement loop
    timeval snd_timeout = {1, 0};
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &snd_timeout, sizeof(snd_timeout));
}

bool wifi_initialize(void) {
    ESP_LOGI(WIFI_TAG, "Initializing...");

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
    ESP_LOGI(WIFI_TAG, "Creating socket...");

    int socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (socketFD < 0) {
        ESP_LOGE(WIFI_TAG, "Could not create socket, errno=%d", errno);
        return false;
    }

    //Define IPv4 information for server
    sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(WIFI_TCP_PORT);

    // quick rebinding after a reboot
    int opt = 1;
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


    //Bind for listening
    int err = bind(socketFD, (sockaddr*)&serverAddress, sizeof(serverAddress));
    if (err != 0) {
        ESP_LOGE(WIFI_TAG, "Could not bind socket! errno=%d", errno);
        close(socketFD);
        return false;
    }

    err = listen(socketFD, 1);
    if (err != 0) {
        ESP_LOGE(WIFI_TAG, "Could not start listening! errno=%d", errno);
        close(socketFD);
        return false;
    }

    s_listenFD = socketFD;
    ESP_LOGI(WIFI_TAG, "Listening on port 15000");
    return true;
}

bool wifi_client_connected(int timeout_ms)
{
    if (s_listenFD < 0) return false;
    if (s_clientFD >= 0) timeout_ms = 0;

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(s_listenFD, &readSet);
    timeval tv = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

    int sel = select(s_listenFD + 1, &readSet, nullptr, nullptr, &tv);

    if (sel < 0)
    {
        ESP_LOGE(WIFI_TAG, "select failed, errno=%d", errno);
        return s_clientFD >= 0;
    }

    if (sel > 0)
    {
        sockaddr_in clientAddress = {};
        socklen_t len = sizeof(clientAddress);
        int fd = accept(s_listenFD, (sockaddr*)&clientAddress, &len);
        
        if (fd >= 0)
        {
            if (s_clientFD >= 0)
            {
                ESP_LOGW(WIFI_TAG, "New connection replaces the previous client");
            }
            drop_client();
            configure_client_socket(fd);
            s_clientFD = fd;
            ESP_LOGI(WIFI_TAG, "Got connection from %s. Sending data", inet_ntoa(clientAddress.sin_addr));
        }
        else
        {
            ESP_LOGE(WIFI_TAG, "accept failed, errno=%d", errno);
        }
    }
    return s_clientFD >= 0;
}

bool wifi_send(const char *data, size_t len)
{
    if(s_clientFD < 0) return false;

    size_t sent = 0;
    while (sent < len)
    {
        ssize_t n = send(s_clientFD, data + sent, len - sent, 0);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            ESP_LOGW(WIFI_TAG, "Send failed, closing client");
            drop_client();
            return false;
        }
        sent += (size_t) n;
    }
    return true;
}

int wifi_receive(uint8_t *buf, size_t len)
{
    if (s_clientFD < 0) return 0;

    ssize_t n = recv(s_clientFD, buf, len, MSG_DONTWAIT);

    if (n > 0) return (int) n;

    if (n == 0)
    {
        ESP_LOGW(WIFI_TAG, "Client disconnected, closing socket");
        drop_client();
        return -1;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;

    ESP_LOGW(WIFI_TAG, "Recieve failed (errno=%d, closing client", errno);
    drop_client();
    return -1;
}

bool wifi_receive_packet(uint8_t *pkt, size_t len)
{
    if (pkt == nullptr || len == 0 || len > sizeof(s_rxBuf)) return false;

    while (s_rxLen < len)
    {
        int n = wifi_receive(s_rxBuf + s_rxLen, len - s_rxLen);
        if (n <= 0) return false;
        s_rxLen += (size_t) n;
    }

    memcpy(pkt, s_rxBuf, len);
    s_rxLen = 0;
    return true;
}