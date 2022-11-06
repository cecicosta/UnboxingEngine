#pragma once

#include <cstdint>

class Texture
{
    public:
    int *imageData;
    std::uint32_t bpp;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t texID;
    int type;
};
