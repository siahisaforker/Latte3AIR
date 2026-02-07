#pragma once
// Wii U networking stub / adapter for the Sonic 3 AIR netplay layer.
//
// The Wii U uses nsysnet (BSD sockets-like API) for networking.
// This header provides the interface for a Wii U-specific network backend.
// Currently netplay is disabled on Wii U with a clear user-facing message.

#include <cstdint>

namespace wiiu_net {

/// Initialise the Wii U networking subsystem (nsysnet).
/// Returns true if networking hardware is available.
bool initialize();

/// Shut down the networking subsystem.
void shutdown();

/// Query if netplay is supported on this platform.
/// Currently always returns false — netplay is not yet ported.
bool isNetplaySupported();

/// Get a user-facing status message explaining netplay availability.
const char* getNetplayStatusMessage();

/// TCP socket wrapper (for future use).
struct Socket {
    int fd = -1;
    bool valid() const { return fd >= 0; }
};

Socket openTCP(const char* host, uint16_t port);
void   closeTCP(Socket& sock);
int    sendTCP(Socket& sock, const void* data, int len);
int    recvTCP(Socket& sock, void* data, int len);

} // namespace wiiu_net
