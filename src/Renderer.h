#pragma once
#include "Framebuffer.h"
#include "ShaderBase.h"
#include "base.h"
#include "Maths.h"
#include <type_traits>
#include <cmath>

namespace RGS {
	template<typename vertex_t>
	struct Triangle {
		static_assert(std::is_base_of_v<RGS::VertexBase, vertex_t>, "vertex_t 必须继承VertexBase");
		vertex_t Vertex[3];//三个顶点，构成三角形
		//对于结构体Triangle来说 符号[]传入参数i,返回的是vertex_t类型的第i个（三角形顶点）
		//这两个是操作符重载函数，允许使用下标操作符 [] 来访问 Triangle 的顶点。
		//第一个版本返回一个可修改的引用，用于非常量 Triangle 对象；
		vertex_t& operator[](size_t i) { return Vertex[i]; }
		//第二个版本返回一个常量引用，用于常量 Triangle 对象。
		const vertex_t& operator[](size_t i) const { return Vertex[i]; }
		Triangle() = default;
		//友元函数，允许 Triangle 对象使用 << 操作符进行输出。这个函数输出 Triangle 的详细信息，
		//包括每个顶点的数据。循环遍历三个顶点，并使用 << 操作符将每个顶点的信息输出到 std::ostream 对象 os 中。
		friend std::ostream& operator<<(std::ostream & os, const Triangle<vertex_t>&triangle)
		{
			os << "Triangle: {\n";
			for (int i = 0; i < 3; ++i) {
				os << "  Vertex[" << i << "]: " << triangle.Vertex[i] << "\n";
			}
			os << "}";
			return os;
		}
	};
	//深度测试的类型
	enum class DepthFuncType {
		LESS,
		LEQUAL,
		ALWAYS,
	};
	template<typename vertex_t,typename uniforms_t,typename varyings_t>
	struct Program {
		bool EnableDepthTest = true;
		bool EnableWriteDepth = true;
		bool EnableDoubleSided = false;
		bool EnableBlend = false;
		DepthFuncType DepthFunc = DepthFuncType::LESS;
		//定义一个名为 vertex_shader_t 的类型别名，它是一个指向函数的指针。这个函数接受三个参数，并返回 void
		using vertex_shader_t = void(*)(varyings_t&, const vertex_t&, const uniforms_t&);
		vertex_shader_t VertexShader;
		using fragment_shader_t = Vec4(*)(bool& discard,const varyings_t&, const uniforms_t&);
		fragment_shader_t FragmentShader;
		//传入两个着色器程序函数 顶点着色器，片段着色器
		Program(const vertex_shader_t vertexShader,const fragment_shader_t fragmentShader):VertexShader(vertexShader),FragmentShader(fragmentShader){}

	};
	class Renderer {
		/*
static：这个关键字表示 RGS_MAX_VARYINGS 是一个静态成员变量。静态成员变量属于类本身，而不是类的任何特定对象。这意味着所有实例共享同一个 RGS_MAX_VARYINGS 变量。
constexpr：这个关键字用于定义常量表达式，即在编译时就能确定其值的表达式。这里，它确保 RGS_MAX_VARYINGS 的值在编译时是已知的，这对于数组大小或模板参数等场景非常有用。
		*/

	private:
		static constexpr int RGS_MAX_VARYINGS = 9;
	private:
		//描述对象在三维空间中相对于各个轴的正负方向的平面。
		enum class Plane
		{
			POSITIVE_W,
			POSITIVE_X,
			NEGATIVE_X,
			POSITIVE_Y,
			NEGATIVE_Y,
			POSITIVE_Z,
			NEGATIVE_Z,
		};
		struct BoundingBox
		{
			int MinX, MaxX, MinY, MaxY;
		};
		static bool IsVertexVisible(const Vec4& clipPos);
		static bool IsInsidePlane(const Vec4& clipPos, const Plane plane);
		static bool IsInsideTriangle(float(&weights)[3]);
		static bool IsBackFacing(const Vec4& a, const Vec4& b, const Vec4& c);
		static bool PassDepthTest(const float writeDepth, const float fDepth, const DepthFuncType depthFunc);
		static float GetIntersectRatio(const Vec4& prev, const Vec4& curr, const Plane plane);
		//片段着色器的包围盒
		static BoundingBox GetBoundingBox(const Vec4(&fragCoords)[3], const int width, const int height);
		static void CalculateWeights(float(&screenWeights)[3], float(&weights)[3], const Vec4(&fragCoords)[3], const Vec2& screenPoint);
		//对变量进行线性插值 计算裁剪的顶点坐标
		template<typename varyings_t>
		static void LerpVaryings(varyings_t& out, const varyings_t& start, const varyings_t& end, const float ratio) {

			constexpr uint32_t floatNum = sizeof(varyings_t) / sizeof(float);

			float* startFloat = (float*)&start;
			float* endFloat = (float*)&end;
			float* outFloat = (float*)&out;
			for (int i = 0; i < (int)floatNum; ++i) {
				//线性插值
				outFloat[i] = Lerp(startFloat[i], endFloat[i], ratio);
			}
		}
		//根据权重插值，并计算裁剪空间，归一化设置以及屏幕像素空间的 插值坐标 这个是计算三角形内部的坐标
		template<typename varyings_t>
		static void LerpVaryings(
			varyings_t& out,
			const varyings_t(&varyings)[3],
			const float(&weights)[3],
			const int width,
			const int height) {

			/*
			插值计算：
out.ClipPos：计算插值后的裁剪空间位置（Clip Position）。这是通过将每个顶点的 ClipPos 与相应的权重相乘并求和来实现的。
归一化设备坐标（NDC）计算：
out.NdcPos：计算归一化设备坐标（Normalized Device Coordinates）。这是通过将插值后的裁剪空间坐标除以其齐次坐标 W 来实现的，用于将坐标转换到标准化的设备坐标空间。
out.NdcPos.W：设置 NdcPos 的 W 分量。由于在NDC空间中，W 应该是1，这里通过取 ClipPos.W 的倒数来进行设置。
			*/
			out.ClipPos = varyings[0].ClipPos * weights[0] +
				varyings[1].ClipPos * weights[1] +
				varyings[2].ClipPos * weights[2];

			out.NdcPos = out.ClipPos / out.ClipPos.W;
			out.NdcPos.W = 1.0f / out.ClipPos.W;
			//将NDC坐标转换为屏幕空间坐标。X 和 Y 坐标通过映射到 [0, 1] 范围并乘以宽度和高度来计算。Z 坐标也映射到 [0, 1] 范围。
			//将[0, 1]范围的X坐标映射到[0, width]范围，即屏幕的宽度，从而得到实际的屏幕像素坐标。
			//将[0, 1]范围的Y坐标映射到[0, height]范围，即屏幕的高度，从而得到实际的屏幕像素坐标。
			out.FragPos.X = ((out.NdcPos.X + 1.0f) * 0.5 * width);
			out.FragPos.Y = ((out.NdcPos.Y + 1.0f) * 0.5 * height);
			out.FragPos.Z = (out.NdcPos.Z + 1.0f) * 0.5f;
			out.FragPos.W = out.NdcPos.W;
			//跳过这些顶点的偏移
			constexpr uint32_t floatOffset = sizeof(Vec4) * 3 / (sizeof(float));
			//计算这个类型剩余属性，如果有则进行插值  纹理坐标，三角形内部坐标对应的纹理坐标
			constexpr uint32_t floatNum = sizeof(varyings_t) / sizeof(float);
			float* v0 = (float*)&varyings[0];
			float* v1 = (float*)&varyings[1];
			float* v2 = (float*)&varyings[2];
			float* outFloat = (float*)&out;
			//这个循环对所有其他浮点属性进行插值，使用与 ClipPos 相同的权重方法。
			for (int i = floatOffset; i < (int)floatNum; i++)
			{
				outFloat[i] = v0[i] * weights[0] + v1[i] * weights[1] + v2[i] * weights[2];
			}
		}
		//用于裁剪不在平面视野内的顶点,得到裁剪后的顶点数据，由X, Y, Z的正负组成的六个平面
		template<typename varyings_t>
		static int ClipAgainstPlane(varyings_t(&outVaryings)[RGS_MAX_VARYINGS],
			const varyings_t(&inVaryings)[RGS_MAX_VARYINGS],
			const Plane plane,
			const int inVertexNum)
		{
			ASSERT(inVertexNum >= 3);
			//输出的顶点，裁剪后顶点个数
			int outVertexNum = 0;
			for (int i = 0; i < inVertexNum; i++)
			{
				int prevIndex = (inVertexNum - 1 + i) % inVertexNum;
				int curIndex = i;
				const varyings_t& preVaryings = inVaryings[prevIndex];
				const varyings_t& curVaryings = inVaryings[curIndex];
				const bool prevInside = IsInsidePlane(preVaryings.ClipPos, plane);
				const bool curInside = IsInsidePlane(curVaryings.ClipPos, plane);
				//当前顶点与前一个顶点判断不一致的情况下，肯定有一个顶点是在平面外部，所以要通过插值的方式得到两个顶点之间的插值点，作为在平面外部的裁剪顶点
				//如果当前顶点和前一个顶点在平面的不同侧，计算它们的交点，并使用线性插值来生成新的顶点数据。
				if (curInside != prevInside) {
					float ratio = GetIntersectRatio(preVaryings.ClipPos, curVaryings.ClipPos, plane);
					LerpVaryings(outVaryings[outVertexNum], preVaryings, curVaryings, ratio);
					outVertexNum++;
				}
				//并且判断当前顶点是否在平面内，如果在则直接加入
				if (curInside) {
					outVaryings[outVertexNum++] = inVaryings[curIndex];
				}
			}
			ASSERT(outVertexNum <= RGS_MAX_VARYINGS);
			return outVertexNum;
		}
		//判断三角形面片的顶点在裁剪空间之内,并得到裁剪之后的顶点
		template<typename varyings_t>
		static int Clip(varyings_t(&varyings)[RGS_MAX_VARYINGS]) {
			//检查三个顶点是否在视锥体内。IsVertexVisible 基于顶点的裁剪空间位置（ClipPos）来判断顶点是否可见。
			bool v0_Visible = IsVertexVisible(varyings[0].ClipPos);
			bool v1_Visible = IsVertexVisible(varyings[1].ClipPos);
			bool v2_Visible = IsVertexVisible(varyings[2].ClipPos);
			//堵在视锥体之内则直接返回
			if (v0_Visible && v1_Visible && v2_Visible) {
				return 3;
			}
			int vertexNum = 3;
			//针对六个裁剪平面（ X, Y, Z的正负方向）重复执行，确保顶点数据完全在视锥体内。类似于天空盒在一个空间内，由X, Y, Z的正负组成
			varyings_t varyings_[RGS_MAX_VARYINGS];
			//Plane::POSITIVE_W 是一个关键的裁剪平面，用于确保所有顶点都处于一个可以安全进行透视投影的位置。
			vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::POSITIVE_W, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_X, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_X, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_Y, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_Y, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_Z, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_Z, vertexNum);
			if (vertexNum == 0) return 0;
			//使用 memcpy 将临时数组 varyings_ 中的数据复制回原数组 varyings，更新顶点数据。
			//如果当前顶点是至少有一个顶点在裁剪空间之内的，则要通过ClipAgainstPlane计算出所有的裁剪顶点，并将其输出，计算裁剪之后的顶点个数
			memcpy(varyings, varyings_, sizeof(varyings_));
			return vertexNum;
		}
		//负责将顶点坐标从裁剪空间转换到归一化设备坐标（NDC）
		template<typename varyings_t>
		static void CaculateNdcPos(varyings_t(&varyings)[RGS_MAX_VARYINGS], const int vertexNum) {
			for (int i = 0; i < vertexNum; i++)
			{
				float w = varyings[i].ClipPos.W;
				varyings[i].NdcPos = varyings[i].ClipPos / w;
				varyings[i].NdcPos.W = 1.0f / w;
			}
		}
		//从NDC转换到片段坐标（屏幕空间坐标）z坐标判断为深度
		template<typename varyings_t>
		static void CaculateFragPos(varyings_t(&varyings)[RGS_MAX_VARYINGS], const int vertexNum, const int width, const int height) {
			for (int i = 0; i < vertexNum; i++)
			{
				float x = ((varyings[i].NdcPos.X + 1.0f) * 0.5f * width);
				float y = ((varyings[i].NdcPos.Y + 1.0f) * 0.5f * height);
				float z = (varyings[i].NdcPos.Z + 1.0f) * 0.5f;
				float w = varyings[i].NdcPos.W;

				varyings[i].FragPos.X = x;
				varyings[i].FragPos.Y = y;
				varyings[i].FragPos.Z = z;
				varyings[i].FragPos.W = w;
			}
		}
		//得到每个位置的像素值和深度值,渲染像素
		template<typename vertex_t, typename uniforms_t, typename varyings_t>
		static void ProcessPixel(Framebuffer& framebuffer,
			const int x,
			const int y,
			const Program<vertex_t, uniforms_t, varyings_t>& program,
			const varyings_t& varyings,
			const uniforms_t& uniforms)
		{
			/* Pixel Shading */
			bool discard = false;
			Vec4 color{ 0.0f, 0.0f, 0.0f, 0.0f };
			//将输入的像素坐标，进入片段着色器得到颜色值，通过纹理坐标得到对应的像素值
			color = program.FragmentShader(discard, varyings, uniforms);
			if (discard)
			{
				return;
			}
			//将颜色的每个分量限制在0到1之间，确保不会出现颜色值溢出。
			color.X = Clamp(color.X, 0.0f, 1.0f);
			color.Y = Clamp(color.Y, 0.0f, 1.0f);
			color.Z = Clamp(color.Z, 0.0f, 1.0f);
			color.W = Clamp(color.W, 0.0f, 1.0f);

			/* Blend */
			//根据源颜色的透明度与目标颜色进行线性插值（混合）。这通常用于实现透明效果。如果未启用混合，则直接将计算的颜色值写入帧缓冲区。
			if (program.EnableBlend)
			{
				Vec3 dstColor = framebuffer.GetColor(x, y);
				Vec3 srcColor = color;
				float alpha = color.W;
				color = { Lerp(dstColor, srcColor, alpha), 1.0f };
				framebuffer.SetColor(x, y, color);
			}
			else
			{
				framebuffer.SetColor(x, y, color);
			}
			//将当前片段的深度值写入深度缓冲区。这用于后续的深度测试，以确定像素的可见性。到这一步相当于这个像素是可见的了，要更新深度值
			if (program.EnableWriteDepth)
			{
				float depth = varyings.FragPos.Z;
				framebuffer.SetDepth(x, y, depth);
			}
		}
		template<typename vertex_t, typename uniforms_t, typename varyings_t>
		static void RasterizeTriangle(Framebuffer& framebuff,
			const Program<vertex_t, uniforms_t, varyings_t>& program,
			const varyings_t(&varyings)[3],
			const uniforms_t& uniforms) {
			//背面剔除：如果不渲染双面，检查三角形是否为背面。这通常通过计算三角形的法向量与视线方向的点积来判断。如果是背面，则不进行进一步处理。
			// 三角形的有向面积来实现，判断顺逆时针
			//通过归一化设备坐标来计算
			if (!program.EnableDoubleSided)
			{
				bool isBackFacing = false;
				isBackFacing = IsBackFacing(varyings[0].NdcPos, varyings[1].NdcPos, varyings[2].NdcPos);
				if (isBackFacing)
				{
					return;
				}
			}
			int width = framebuff.GetWidth();
			int height = framebuff.GetHeight();
			//得到当前三角形的屏幕空间坐标，三个像素坐标
			Vec4 fragCoords[3];//屏幕空间坐标
			fragCoords[0] = varyings[0].FragPos;
			fragCoords[1] = varyings[1].FragPos;
			fragCoords[2] = varyings[2].FragPos;
			//计算包围三角形的最小边界框，以便只处理可能覆盖三角形的像素。在三角形之外的像素不渲染
			BoundingBox bBox = GetBoundingBox(fragCoords, width, height);
			//遍历包围盒内的每个像素，我们初始只给顶点绑定了纹理坐标，而三角面片内部的像素是没有纹理坐标的，只能通过计算重心坐标得到纹理坐标，再去得到纹素
			for (int y = bBox.MinY; y <= bBox.MaxY; ++y) {
				for (int x = bBox.MinX; x <= bBox.MaxX; ++x) {
					float screenWeights[3];
					float weights[3];
					//每个像素的大小为1x1的方格，所以每个像素的中心点是当前的坐标在x,y轴上 +0.5
					Vec2 screenPoint((float)x + 0.5f, (float)y + 0.5);
					//通过变量包围盒的像素坐标和三角形三个顶点的像素坐标，去计算得到重心坐标
					CalculateWeights(screenWeights, weights, fragCoords, screenPoint);
					//在三角形之外的像素不渲染，权重值如果有负数，则在三角形外面
					if (!IsInsideTriangle(weights)) {
						continue;
					}
					varyings_t pixVaryings;
					//得到通过重心坐标插值后的 得到在三角形内部中的像素坐标，包括了深度值，纹理坐标
					LerpVaryings(pixVaryings, varyings, weights, width, height);

					//得到像素坐标之后，进行深度测试，判断当前像素是否可见
			//如果通过深度测试，当前像素的颜色和深度值将被用来更新帧缓冲区(像素)和深度缓冲区（深度）中相应位置的值。如果通过同一位置的深度测试则覆盖像素，
			// 如果不通过测试，当前片段将被丢弃，不会对帧缓冲区和深度缓冲区产生影响。
			//由这种方法来过滤掉看不见的像素
					if (program.EnableDepthTest) {
						float depth = pixVaryings.FragPos.Z;//得到当前像素的深度
						float fDepth = framebuff.GetDepth(x, y);//获得缓冲帧在这个位置能看到的像素对应的深度，用于判断当前像素是否可见
						DepthFuncType depthFunc = program.DepthFunc;
						//判断是否通过深度测试，当前的像素是否有被覆盖，如果没有通过则不渲染，比较depth fDepth离视点更近
						if (!PassDepthTest(depth, fDepth, depthFunc)) {
							continue;
						}
					}
					//通过了深度测试则更新当前帧在x,y坐标 像素值以及深度值
					//三角形内部的像素对应的纹理坐标通常是通过三角形三个顶点的重心坐标（也称为重心插值或重心坐标插值）来计算得到的
					//通过纹理坐标得到像素
					ProcessPixel(framebuff, x, y, program, pixVaryings, uniforms);
				}
			}

		}
public:
		template<typename vertex_t, typename uniforms_t, typename varyings_t>
		static void Draw(Framebuffer& framebuffer, const Program<vertex_t, uniforms_t, varyings_t>& program, const Triangle<vertex_t>& triangle, const uniforms_t& uniforms)
		{
			static_assert(std::is_base_of_v<VertexBase, vertex_t>, "vertex_t 必须继承自 RGS::VertexBase");
			static_assert(std::is_base_of_v<VaryingsBase, varyings_t>, "varyings_t 必须继承自 RGS::VaryingsBase");
			varyings_t varyings[RGS_MAX_VARYINGS];
			//通过顶点着色器，进行坐标转换，得到clip坐标，纹理坐标等，投影到屏幕空间
			for (int i = 0; i < 3; i++)
			{
				program.VertexShader(varyings[i], triangle[i], uniforms);
			}
			//对坐标进行裁剪，得到裁剪空间中的可见顶点数量，和裁剪之后的顶点坐标
			int vertexNum = Clip(varyings);
			//计算归一化设备坐标
			CaculateNdcPos(varyings, vertexNum);
			int fWidth = framebuffer.GetWidth();
			int fHeight = framebuffer.GetHeight();
			//计算屏幕空间坐标，像素坐标，每个顶点在屏幕上对应的像素位置
			CaculateFragPos(varyings, vertexNum, (float)fWidth, (float)fHeight);
			//得到裁剪之后的三角形对应的顶点属性（各种坐标系下的坐标），可能有两个三角形,裁剪之后的顶点个数可能是四个
			//依次渲染裁剪空间到屏幕空间上的三角形，进行光栅化，将三角形的顶点转化为屏幕空间中的像素
			//渲染屏幕空间的三角形
			for (int i = 0; i < vertexNum - 2; ++i) {
				varyings_t triVaryings[3];//三角形的三个顶点
				triVaryings[0] = varyings[0];
				triVaryings[1] = varyings[i + 1];
				triVaryings[2] = varyings[i + 2];
				RasterizeTriangle(framebuffer, program, triVaryings, uniforms);
			}
		}

	};
}