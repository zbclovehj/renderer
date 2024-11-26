#include "Renderer.h"
#include <iostream>
#include <algorithm>  // 包含min和max函数的头文件
namespace RGS {
	//一个顶点的 X、Y、Z 坐标必须在 [-W, W] 的范围内，以保证它在视锥体内
	bool Renderer::IsVertexVisible(const Vec4& clipPos) {
		return fabs(clipPos.X) <= clipPos.W && fabs(clipPos.Y) <= clipPos.W && fabs(clipPos.Z) <= clipPos.W;
	}
	//根据传入的平面类型，函数使用 switch 语句来选择相应的条件判断。每个条件都是基于裁剪空间坐标与 W 坐标的关系来判断顶点是否在指定平面的内侧。
    //w为视锥体的界限
	bool Renderer::IsInsidePlane(const Vec4& clipPos, const Plane plane) {
		switch (plane)//判断裁剪顶点是否在对应的平面内
		{
		case Plane::POSITIVE_W:
			return clipPos.W >= 0.0f;
        case Plane::POSITIVE_X:
            return clipPos.X <= +clipPos.W;
        case Plane::NEGATIVE_X:
            return clipPos.X >= -clipPos.W;
        case Plane::POSITIVE_Y:
            return clipPos.Y <= +clipPos.W;
        case Plane::NEGATIVE_Y:
            return clipPos.Y >= -clipPos.W;
        case Plane::POSITIVE_Z:
            return clipPos.Z <= +clipPos.W;
        case Plane::NEGATIVE_Z:
            return clipPos.Z >= -clipPos.W;
        default:
            ASSERT(false);
            return false;
		}
	}
    //判断像素是否在三角形内部,通过重心坐标来判断
    bool Renderer::IsInsideTriangle(float(&weights)[3]) {
        return weights[0] >= -EPSILON && weights[1] >= -EPSILON && weights[2] >= -EPSILON;
    }
    //在右手坐标系中，如果顶点按逆时针顺序排列（从观察者视角看去是逆时针），则它们形成的三角形面向观察者，有向面积应该是正的。
    //相反，如果面积是负的或零，表示三角形是背面朝向的或者在视线平面上。 判断三角形的顶点顺逆时针
    bool Renderer::IsBackFacing(const Vec4& a, const Vec4& b, const Vec4& c) {
        // 逆时针为正面（可见）
        float signedArea = a.X * b.Y - a.Y * b.X +
            b.X * c.Y - b.Y * c.X +
            c.X * a.Y - c.Y * a.X;
        return signedArea <= 0;
    }
    bool Renderer::PassDepthTest(const float writeDepth, const float fDepth, const DepthFuncType depthFunc) {
        switch (depthFunc) {
        //如果深度缓冲区中的值 (fDepth) 减去当前片段的深度值 (writeDepth) 大于一个很小的正数 (EPSILON)，则返回 true。这表示当前片段比缓冲区中已有的片段更靠近观察者。
        case DepthFuncType::LESS:
            return fDepth - writeDepth > EPSILON;
        //如果深度缓冲区中的值减去当前片段的深度值大于或等于 EPSILON，则返回 true。这允许当前片段与缓冲区中的片段在几乎相同的深度时也能通过测试。
        case DepthFuncType::LEQUAL:
            return fDepth - writeDepth >= -EPSILON;
        case DepthFuncType::ALWAYS:
            return true;
        default:
            return false;
        }
    }
    //P(t)=P1​+t(P2​−P1​)通过这个公式计算对于 某平面（即正负 x y z  裁剪平面），我们需要找到 t 的值,计算线段与平面的交点，就可以直接令某个坐标轴等于这个平面在这个轴的值
    //(P1​.W+t(P2​.W−P1​.W))=(P1​.X+t(P2​.X−P1​.X))  t=(P1​.W−P1​.X)/((P2​.W−P2​.X)-(P1​.W−P1​.X))​
    //找到两点之间与平面相交的参数值t
    float Renderer::GetIntersectRatio(const Vec4& prev, const Vec4& curr, const Plane plane) {
        switch (plane) {
        case Plane::POSITIVE_W:
            return (prev.W - 0.0f) / (prev.W - curr.W);
        case Plane::POSITIVE_X:
            return (prev.W - prev.X) / ((prev.W - prev.X) - (curr.W - curr.X));
        case Plane::NEGATIVE_X:
            return (prev.W + prev.X) / ((prev.W + prev.X) - (curr.W + curr.X));
        case Plane::POSITIVE_Y:
            return (prev.W - prev.Y) / ((prev.W - prev.Y) - (curr.W - curr.Y));
        case Plane::NEGATIVE_Y:
            return (prev.W + prev.Y) / ((prev.W + prev.Y) - (curr.W + curr.Y));
        case Plane::POSITIVE_Z:
            return (prev.W - prev.Z) / ((prev.W - prev.Z) - (curr.W - curr.Z));
        case Plane::NEGATIVE_Z:
            return (prev.W + prev.Z) / ((prev.W + prev.Z) - (curr.W + curr.Z));
        default:
            ASSERT(false);
            return 0.0f;
        }
    }
    //计算包围盒，在二维屏幕像素空间种计算三角形包围盒
    Renderer::BoundingBox Renderer::GetBoundingBox(const Vec4(&fragCoords)[3], const int width, const int height) {
        auto xList = { fragCoords[0].X,fragCoords[1].X,fragCoords[2].X };
        auto yList = { fragCoords[0].Y,fragCoords[1].Y,fragCoords[2].Y };
        float minX = std::min<float>(xList);
        float maxX = std::max<float>(xList);
        float minY = std::min<float>(yList);
        float maxY = std::max<float>(yList);
        minX = Clamp(minX, 0.0f, (float)(width - 1));
        maxX = Clamp(maxX, 0.0f, (float)(width - 1));
        minY = Clamp(minY, 0.0f, (float)(height - 1));
        maxY = Clamp(maxY, 0.0f, (float)(height - 1));
        //在渲染管线中，通过计算三角形的边界框可以有效地减少需要处理的像素数量，特别是在光栅化阶段，只有位于边界框内的像素才需要进一步处理。
        BoundingBox bBox;
        bBox.MinX = std::floor(minX);
        bBox.MinY = std::floor(minY);
        bBox.MaxX = std::ceil(maxX);
        bBox.MaxY = std::ceil(maxY);
        return bBox;
    }
    //计算重心坐标，判断像素是否在三角形内部
    void Renderer::CalculateWeights(float(&screenWeights)[3],
        float(&weights)[3],
        const Vec4(&fragCoords)[3],
        const Vec2& screenPoint) {
        Vec2 ab = fragCoords[1] - fragCoords[0];
        Vec2 ac = fragCoords[2] - fragCoords[0];
        Vec2 ap = screenPoint - fragCoords[0];
        //通过面积的比值来得到重心坐标,通过叉乘获得面积
        float factor = 1.0f / (ab.X * ac.Y - ab.Y * ac.X);
        float s = (ac.Y * ap.X - ac.X * ap.Y) * factor;
        float t = (ab.X * ap.Y - ab.Y * ap.X) * factor;
        screenWeights[0] = 1 - s - t;
        screenWeights[1] = s;
        screenWeights[2] = t;
        //透视矫正
        //在透视投影中，由于远处的物体在屏幕上映射得更小，如果直接使用线性插值（即不考虑深度信息的普通插值方法），
        //则会导致纹理拉伸或压缩的视觉失真。透视校正的目的是调整这种插值方式，使得纹理和其他属性能够按照正确的透视比例显示。
        float w0 = fragCoords[0].W * screenWeights[0];
        float w1 = fragCoords[1].W * screenWeights[1];
        float w2 = fragCoords[2].W * screenWeights[2];
        float normalizer = 1.0f / (w0 + w1 + w2);
        weights[0] = w0 * normalizer;
        weights[1] = w1 * normalizer;
        weights[2] = w2 * normalizer;
    }
}