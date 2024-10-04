#pragma once

#include "Module.h"

#include <string>

class OutputStream;

class Network : public Module {
    public:
        static Network* getInstance();
        static bool create(ConfigReader& cr);
        static void vSetupIFTask(void *pvParameters);

        bool configure(ConfigReader& cr);
        void set_abort();
        const char *get_hostname() const { return hostname.c_str(); }
        bool is_dhcp() const { return use_dhcp; }
        const char *get_ntp_server() const { return ntp_server.c_str(); }
        int get_timezone() const { return timezone; }

    private:
        static Network *instance;
        Network();
        virtual ~Network();

        void network_thread();
        bool start(void);

        bool handle_net_cmd( std::string& params, OutputStream& os );
        bool wget_cmd( std::string& params, OutputStream& os );
        bool update_cmd( std::string& params, OutputStream& os );
        bool ntp_cmd( std::string& params, OutputStream& os );

        int timezone{0};
        std::string hostname;
        std::string ntp_server;
        std::string firmware_url;

        volatile bool abort_network{false};
        bool enable_shell{false};
        bool enable_httpd{false};
        bool enable_ftpd{false};
        bool enable_ntp{true};
        bool use_dhcp;
};
