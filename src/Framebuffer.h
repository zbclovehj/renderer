#pragma once
#include "Maths.h"

namespace RGS {

    // Learn Framebuffer: https://learnopengl-cn.github.io/04%20Advanced%20OpenGL/05%20Framebuffers/
    class Framebuffer
    {
    public:
        Framebuffer(const int width, const int height);
        ~Framebuffer();

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

        void SetColor(const int x, const int y, const Vec3& color);
        Vec3 GetColor(const int x, const int y) const;
        void SetDepth(const int x, const int y, const float depth);
        float GetDepth(const int x, const int y) const;
        //初始化帧画面为全黑，深度离相机最远
        void Clear(const Vec3& color = { 0.0f, 0.0f, 0.0f });
        void ClearDepth(float depth = 1.0f);

    private:
        int GetPixelIndex(const int x, const int y) const { return y * m_Width + x; }

    private:
        int m_Width = 800;
        int m_Height = 600;
        int m_PixelSize;
        //每一帧的图像数据。像素RGB以及深度
        float* m_DepthBuffer;
        Vec3* m_ColorBuffer;
    };

}