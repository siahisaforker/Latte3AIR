/*
 * PlatformNetwork.h
 * Small Wii U network adapter skeleton. Delegates to existing Sockets implementation
 * for now and provides a single place to add WUT-specific networking later.
 */
#pragma once

#include "oxygen_netcore/network/Sockets.h"

class PlatformNetwork
{
public:
    static inline bool startup()
    {
        Sockets::startupSockets();
        return true;
    }

    static inline void shutdown()
    {
        Sockets::shutdownSockets();
    }

    // Bind helpers to centralize platform quirks (Wii U may override these later)
    static inline bool bindUDPSocket(UDPSocket& socket, uint16 port, Sockets::ProtocolFamily family = Sockets::ProtocolFamily::IPv4)
    {
        return socket.bindToPort(port, family);
    }

    static inline bool bindUDPSocketAny(UDPSocket& socket, Sockets::ProtocolFamily family = Sockets::ProtocolFamily::IPv4)
    {
        return socket.bindToAnyPort(family);
    }
};
