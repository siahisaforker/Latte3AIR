#pragma once
#include <cstdint>

class Geometry {
public:
    enum class Type {
        SPRITE,
        TILE,
        OTHER
    };

    Geometry(Type t) : mType(t) {}
    virtual ~Geometry() {}

    Type getType() const { return mType; }

private:
    Type mType;
};
