#include "wiiu_network.h"

#if defined(PLATFORM_WIIU)

#if __has_include(<nsysnet/socket.h>)
#include <nsysnet/socket.h>
#include <nsysnet/nssl.h>
#define WIIU_HAS_NSYSNET 1
#else
#define WIIU_HAS_NSYSNET 0
#endif

#include <cstring>
#include <cerrno>

namespace {
    bool g_netInitialized = false;
}

namespace wiiu_net {

bool initialize()
{
    if (g_netInitialized) return true;
#if WIIU_HAS_NSYSNET
    // socket_lib_init() initialises the nsysnet BSD socket layer
    int ret = socket_lib_init();
    if (ret < 0) return false;
    g_netInitialized = true;
    return true;
#else
    return false;
#endif
}

void shutdown()
{
    if (!g_netInitialized) return;
#if WIIU_HAS_NSYSNET
    socket_lib_finish();
#endif
    g_netInitialized = false;
}

bool isNetplaySupported()
{
    // TODO: Enable when full netplay port is complete
    return false;
}

const char* getNetplayStatusMessage()
{
    return "Netplay is not yet available on Wii U. Stay tuned for a future update!";
}

Socket openTCP(const char* host, uint16_t port)
{
    Socket s;
#if WIIU_HAS_NSYSNET
    if (!g_netInitialized) return s;

    s.fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s.fd < 0) return s;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // Simple numeric IP parse (no DNS on Wii U without extra work)
    unsigned int a, b, c, d;
    if (sscanf(host, "%u.%u.%u.%u", &a, &b, &c, &d) == 4)
    {
        addr.sin_addr.s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
    }
    else
    {
        close(s.fd);
        s.fd = -1;
        return s;
    }

    if (connect(s.fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        close(s.fd);
        s.fd = -1;
    }
#endif
    return s;
}

void closeTCP(Socket& sock)
{
#if WIIU_HAS_NSYSNET
    if (sock.valid())
    {
        close(sock.fd);
        sock.fd = -1;
    }
#endif
}

int sendTCP(Socket& sock, const void* data, int len)
{
#if WIIU_HAS_NSYSNET
    if (!sock.valid()) return -1;
    return static_cast<int>(send(sock.fd, data, len, 0));
#else
    (void)sock; (void)data; (void)len;
    return -1;
#endif
}

int recvTCP(Socket& sock, void* data, int len)
{
#if WIIU_HAS_NSYSNET
    if (!sock.valid()) return -1;
    return static_cast<int>(recv(sock.fd, data, len, 0));
#else
    (void)sock; (void)data; (void)len;
    return -1;
#endif
}

} // namespace wiiu_net

#else

// Non-Wii U stubs
namespace wiiu_net {
bool initialize() { return false; }
void shutdown() {}
bool isNetplaySupported() { return false; }
const char* getNetplayStatusMessage() { return "Not on Wii U"; }
Socket openTCP(const char*, uint16_t) { return {}; }
void closeTCP(Socket&) {}
int sendTCP(Socket&, const void*, int) { return -1; }
int recvTCP(Socket&, void*, int) { return -1; }
} // namespace wiiu_net

#endif
