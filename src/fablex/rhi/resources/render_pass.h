#pragma once

#include "internal.h"

namespace fe::rhi
{

class TextureView;
class SwapChain;
class Semaphore;

struct ClearValues
{
    std::array<float, 4> color = { 0.0f, 0.0f, 0.0f, 1.0f };

    struct
    {
        float depth = 1.0f;
        uint32 stencil = 0;
    } depthStencil;
};

struct RenderTarget
{
    TextureView* target;
    LoadOp loadOp;
    StoreOp storeOp;
    ClearValues clearValue;
};

struct MultiviewInfo
{
    bool isEnabled = false; // false is default
    uint32_t viewCount = 0;
};

struct RenderingBeginInfo
{
    enum Type
    {
        OFFSCREEN_PASS,
        SWAP_CHAIN_PASS,
    };

    RenderingBeginInfo(Type inType) : type(inType) { }

    Type type;
    MultiviewInfo multiviewInfo;
    RenderingBeginInfoFlags flags;

    struct OffscreenPass
    {
        std::vector<RenderTarget> renderTargets;
        RenderingBeginInfoFlags flags;
        MultiviewInfo multiviewInfo;		// Not necessary

        ~OffscreenPass() = default;
    };

    struct SwapChainPass
    {
        SwapChain* swapChain = nullptr;
        ClearValues clearValues;

        ~SwapChainPass() = default;
    };

    OffscreenPass offscreenPass;
    SwapChainPass swapChainPass;
};

}