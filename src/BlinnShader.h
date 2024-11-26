#pragma once
#include "Maths.h"
#include <iostream>
#include <memory>
#include "ShaderBase.h"
#include  "Texture.h"
namespace RGS {
	struct BlinnVertex : public VertexBase {
		Vec3 ModelNormal;
		Vec2 TexCoord = { 0.0f,0.0f };
        friend std::ostream& operator<<(std::ostream& os, const BlinnVertex& bv)
        {
            os << "BlinnVertex: { " << static_cast<const VertexBase&>(bv)
                << ", ModelNormal: " << bv.ModelNormal
                << ", TexCoord: " << bv.TexCoord << " }";
            return os;
        }
	};
    struct BlinnVaryings : public VaryingsBase {
        Vec3 WorldPos;
        Vec3 WorldNormal;
        Vec2 TexCoord;
    };
    struct  BlinnUniforms : public UniformsBase
    {
        Mat4 Model;
        Mat4 ModelNormalToWorld;
        Vec3 LightPos{ 0.0f, 1.0f, 2.0f };
        Vec3 LightAmbient{ 0.2f, 0.2f, 0.2f };
        Vec3 LightDiffuse{ 0.5f, 0.5f, 0.5f };
        Vec3 LightSpecular{ 1.0f, 1.0f, 1.0f };
        Vec3 ObjectColor{ 1.0f, 1.0f, 1.0f };
        Vec3 CamerasPos;
        float Shininess = 32.0f;
        //通过程序导入的纹理数据
        Texture* Diffuse = nullptr;
        Texture* Specular = nullptr;
    };
    void BlinnVertexShader(BlinnVaryings& varyings, const BlinnVertex& vertex, const BlinnUniforms& uniforms);

    Vec4 BlinnFragmentShader(bool& discard, const BlinnVaryings& varyings, const BlinnUniforms& uniforms);
}