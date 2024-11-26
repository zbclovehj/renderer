#include "BlinnShader.h"
#include "Maths.h"
#include <cmath>
namespace RGS {
	//将三维顶点转化为着色器中需要的变量，顶点着色器
	void BlinnVertexShader(BlinnVaryings& varyings, const BlinnVertex& vertex, const BlinnUniforms& uniforms) {
		//将模型顶点转化为在裁剪空间的坐标
		varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
		//获取纹理坐标
		varyings.TexCoord = vertex.TexCoord;
		//得到在世界坐标系下的坐标
		varyings.WorldPos = uniforms.Model * vertex.ModelPos;
		//得到在世界坐标系下的法线
		varyings.WorldNormal = uniforms.ModelNormalToWorld * Vec4{ vertex.ModelNormal,0.0f };

	}
	//片段着色器，考虑光照得到像素颜色 
	Vec4 BlinnFragmentShader(bool& discard, const BlinnVaryings& varyings, const BlinnUniforms& uniforms)
	{
		//包含了所有的统一变量（uniform），如相机位置、光源位置、光照参数和纹理采样器。
		discard = false;
		const Vec3& cameraPos = uniforms.CamerasPos;
		const Vec3& lightPos = uniforms.LightPos;
		const Vec3& worldPos = varyings.WorldPos;
		Vec3 worldNormal = Normalize(varyings.WorldNormal);
		//计算法线向量、视线方向向量、光线方向向量和半向量
		Vec3 viewDir = Normalize(cameraPos - worldPos);
		Vec3 lightDir = Normalize(lightPos - worldPos);
		Vec3 halfDir = Normalize(lightDir + viewDir);
		Vec3 ambient = uniforms.LightAmbient;
		Vec3 specularStrength{ 1.0f, 1.0f, 1.0f };
		Vec3 diffColor{ 1.0f,1.0f,1.0f };
		//纹理采样：如果提供了漫反射和镜面反射纹理，使用纹理坐标从纹理中采样颜色值。这些采样值用于调整环境光和镜面光的强度。
		//通过这个顶点绑定的纹理坐标去采样像素rgb颜色值,而像素对应的纹理坐标，在三角形内部是没有绑定的，这时候就要通过重心坐标插值得到纹理坐标
		//三角形内部的像素对应的纹理坐标通常是通过三角形三个顶点的重心坐标（也称为重心插值或重心坐标插值）来计算得到的
		if (uniforms.Diffuse && uniforms.Specular) {
			const Vec2& texCoord = varyings.TexCoord;
			diffColor = uniforms.Diffuse->Sample(texCoord);
			//Iambient​ = Lambient​×Mambient​
			ambient = ambient * diffColor;
			specularStrength = uniforms.Specular->Sample(texCoord);
		}
		//Idiffuse​ = Ldiffuse​×Mdiffuse​×max(n⋅l, 0)
		Vec3 diffuse = std::max(0.0f, Dot(worldNormal, lightDir)) * uniforms.LightDiffuse * diffColor;
		////Ispecular​ = Lspecular​×Mspecular​×(max(h⋅n, 0)^shininess)
		Vec3 specular = (float)pow(std::max(0.0f, Dot(halfDir, worldNormal)), uniforms.Shininess) * uniforms.LightSpecular * specularStrength;
		//计算结果
		Vec3 result = ambient + diffuse + specular;
		return { result, 1.0f };
	}
}