---
# Fill in the fields below to create a basic custom agent for your repository.
# The Copilot CLI can be used for local testing: https://gh.io/customagents/cli
# To make this agent available, merge this file into the default repository branch.
# For format details, see: https://gh.io/customagents/config

name:
description:
---

# My Agent

Platform-Specific Rule: Wii U Endianness

The agent must ensure the codebase handles the mixed-endian architecture of the Wii U correctly.

Hardware characteristics:

Wii U CPU (Espresso / PowerPC): big-endian

Wii U GPU (Latte): little-endian

Because of this difference, all data transferred between CPU memory and GPU resources must be explicitly endian-safe.

Endian Safety Requirements

The agent should ensure the project defines and uses explicit endian conversion macros instead of relying on implicit platform behavior.

Required macros:

#define READ_LE16(x)
#define READ_LE32(x)
#define READ_LE64(x)

#define READ_BE16(x)
#define READ_BE32(x)
#define READ_BE64(x)

#define WRITE_LE16(dst, val)
#define WRITE_LE32(dst, val)
#define WRITE_LE64(dst, val)

#define WRITE_BE16(dst, val)
#define WRITE_BE32(dst, val)
#define WRITE_BE64(dst, val)

The agent may implement these using compiler intrinsics or byte-swap operations.

Example implementation:

static inline uint32_t bswap32(uint32_t v) {
    return ((v >> 24) & 0x000000FF) |
           ((v >> 8)  & 0x0000FF00) |
           ((v << 8)  & 0x00FF0000) |
           ((v << 24) & 0xFF000000);
}
GPU Data Transfers

When data structures are uploaded to the GPU:

Structures must be converted to little-endian layout

CPU-side code must perform byte swapping when necessary

GPU-facing buffers must not assume host endianness

The agent should verify:

vertex buffers

uniform buffers

shader constant buffers

texture headers

command buffers

Struct Layout Checks

The agent should flag:

direct struct writes to GPU buffers

memcpy of CPU structures into GPU memory

unsafe pointer casting

Instead, the agent should recommend explicit packing functions that perform endian conversion.

Shader Interface Validation

The agent should ensure CPU-side structures match shader expectations.

Checks include:

field size

alignment

endian layout

padding consistency

If mismatches are detected, the agent should propose safe packing code.

Goal

Ensure that the rendering pipeline functions correctly across Wii U’s mixed-endian architecture by enforcing explicit byte-order handling and preventing silent data corruption.
