#include "base.h"
#include "Texture.h"
#include "stb_image/stb_image.h"
#include <iostream>

namespace RGS {
	Texture::Texture(const std::string& path)
	:m_Path(path){
		Init();
	}
	Texture::~Texture()
	{
		if (m_Data)
			delete[] m_Data;
		m_Data = nullptr;
	}
	//采样距离纹理坐标最近的像素
	Vec4 Texture::Sample(Vec2 texCoords) const
	{
		//点采样，从纹理坐标中采样，纹理坐标为0-1
		float vx = Clamp(texCoords.X, 0.0f, 1.0f);
		float vy = Clamp(texCoords.Y, 0.0f, 1.0f);
		//通过纹理坐标得到屏幕空间的像素坐标去取像素值，rgb值
		int x = vx * (m_Width - 1) + 0.5f;
		int y = vy * (m_Height - 1) + 0.5f;
		//通过像素的坐标得到索引，通过索引得到像素值
		int index = y * m_Width + x;
		return m_Data[index];
	}
	//双线性插值
	Vec4 Texture::SampleBilinear(Vec2 texCoords) const {
		// 计算相邻像素坐标
		float u = texCoords.X * (m_Width - 1);
		float v = texCoords.Y * (m_Height - 1);
		//得到x,y最近的像素，向下取整，左上角像素
		int x = static_cast<int>(u);
		int y = static_cast<int>(v);
		//得到近似的比率
		float u_ratio = u - x;
		float v_ratio = v - y;
		//通过右边，下面，右下，以及本身这个像素，
		int x1 = std::min(x + 1, m_Width - 1);
		int y1 = std::min(y + 1, m_Height - 1);

		// 获取四个像素的颜色值
		Vec4 c00 = m_Data[y * m_Width + x];
		Vec4 c10 = m_Data[y * m_Width + x1];
		Vec4 c01 = m_Data[y1 * m_Width + x];
		Vec4 c11 = m_Data[y1 * m_Width + x1];

		// 进行线性插值，先进行水平方向插值 c00+(c10-c00)*u_ratio
		Vec4 interp_x1 = c00 * (1 - u_ratio) + c10 * u_ratio;
		Vec4 interp_x2 = c01 * (1 - u_ratio) + c11 * u_ratio;
		//在进行垂直方向插值
		Vec4 final_color = interp_x1 * (1 - v_ratio) + interp_x2 * v_ratio;
		// 返回最终颜色值
		return final_color;
	}
	//初始化纹理数据
	void Texture::Init()
	{
		int width, height, channels;
		//需要调整光栅化的坐标系统，y轴翻转 
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		//读取纹理图片，并获取像素的长宽个数 加载图像数据 使得在加载图像时自动将图像在垂直方向上翻转。
		data = stbi_load(m_Path.c_str(), &width, &height, &channels, 0);
		ASSERT(data);
		m_Height = height;
		m_Width = width;
		m_Channels = channels;
		int size = height * width;//像素个数
		m_Data = new Vec4[size];//存储像素数据的结构体
		if (channels == 4) {
			//根据颜色通道来存储像素,将rgb值转化为0-1 将像素值从0-255范围转换为0-1的浮点数范围在进行图形处理和渲染时提供了更高的精
			for (int i = 0; i < size; ++i) {
				m_Data[i].X = UChar2Float(data[i * 4]);
				m_Data[i].Y = UChar2Float(data[i * 4 + 1]);
				m_Data[i].Z = UChar2Float(data[i * 4 + 2]);
				m_Data[i].W = UChar2Float(data[i * 4 + 3]);
			}
		} else if (channels == 3) {
			//根据颜色通道来存储像素
			for (int i = 0; i < size; ++i) {
				m_Data[i].X = UChar2Float(data[i * 3]);
				m_Data[i].Y = UChar2Float(data[i * 3 + 1]);
				m_Data[i].Z = UChar2Float(data[i * 3 + 2]);
				m_Data[i].W = 0.0f;
			}
		} else if (channels == 2) {
			//根据颜色通道来存储像素
			for (int i = 0; i < size; ++i) {
				m_Data[i].X = UChar2Float(data[i * 2]);
				m_Data[i].Y = UChar2Float(data[i * 2 + 1]);
				m_Data[i].Z = 0.0f;
				m_Data[i].W = 0.0f;
			}
		} else 	if (channels == 1) {
				//根据颜色通道来存储像素
				for (int i = 0; i < size; ++i) {
					m_Data[i].X = UChar2Float(data[i]);
					m_Data[i].Y = 0.0f;
					m_Data[i].Z = 0.0f;
					m_Data[i].W = 0.0f;
				}
			}
	}
}