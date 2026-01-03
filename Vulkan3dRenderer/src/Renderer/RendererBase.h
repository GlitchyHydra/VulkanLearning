//
// Created by Kvan Valerii on 24.12.2025.
//

#ifndef VULKAN3DRENDERER_RENDERERBASE_H
#define VULKAN3DRENDERER_RENDERERBASE_H


class RendererBase {
    explicit RendererBase(const Device& device);
    virtual ~RendererBase();
    virtual void fillCommandBuffer();
};


#endif //VULKAN3DRENDERER_RENDERERBASE_H