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
	//�������������������������
	Vec4 Texture::Sample(Vec2 texCoords) const
	{
		//������������������в�������������Ϊ0-1
		float vx = Clamp(texCoords.X, 0.0f, 1.0f);
		float vy = Clamp(texCoords.Y, 0.0f, 1.0f);
		//ͨ����������õ���Ļ�ռ����������ȥȡ����ֵ��rgbֵ
		int x = vx * (m_Width - 1) + 0.5f;
		int y = vy * (m_Height - 1) + 0.5f;
		//ͨ�����ص�����õ�������ͨ�������õ�����ֵ
		int index = y * m_Width + x;
		return m_Data[index];
	}
	//˫���Բ�ֵ
	Vec4 Texture::SampleBilinear(Vec2 texCoords) const {
		// ����������������
		float u = texCoords.X * (m_Width - 1);
		float v = texCoords.Y * (m_Height - 1);
		//�õ�x,y��������أ�����ȡ�������Ͻ�����
		int x = static_cast<int>(u);
		int y = static_cast<int>(v);
		//�õ����Ƶı���
		float u_ratio = u - x;
		float v_ratio = v - y;
		//ͨ���ұߣ����棬���£��Լ�����������أ�
		int x1 = std::min(x + 1, m_Width - 1);
		int y1 = std::min(y + 1, m_Height - 1);

		// ��ȡ�ĸ����ص���ɫֵ
		Vec4 c00 = m_Data[y * m_Width + x];
		Vec4 c10 = m_Data[y * m_Width + x1];
		Vec4 c01 = m_Data[y1 * m_Width + x];
		Vec4 c11 = m_Data[y1 * m_Width + x1];

		// �������Բ�ֵ���Ƚ���ˮƽ�����ֵ c00+(c10-c00)*u_ratio
		Vec4 interp_x1 = c00 * (1 - u_ratio) + c10 * u_ratio;
		Vec4 interp_x2 = c01 * (1 - u_ratio) + c11 * u_ratio;
		//�ڽ��д�ֱ�����ֵ
		Vec4 final_color = interp_x1 * (1 - v_ratio) + interp_x2 * v_ratio;
		// ����������ɫֵ
		return final_color;
	}
	//��ʼ����������
	void Texture::Init()
	{
		int width, height, channels;
		//��Ҫ������դ��������ϵͳ��y�ᷭת 
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		//��ȡ����ͼƬ������ȡ���صĳ������ ����ͼ������ ʹ���ڼ���ͼ��ʱ�Զ���ͼ���ڴ�ֱ�����Ϸ�ת��
		data = stbi_load(m_Path.c_str(), &width, &height, &channels, 0);
		ASSERT(data);
		m_Height = height;
		m_Width = width;
		m_Channels = channels;
		int size = height * width;//���ظ���
		m_Data = new Vec4[size];//�洢�������ݵĽṹ��
		if (channels == 4) {
			//������ɫͨ�����洢����,��rgbֵת��Ϊ0-1 ������ֵ��0-255��Χת��Ϊ0-1�ĸ�������Χ�ڽ���ͼ�δ������Ⱦʱ�ṩ�˸��ߵľ�
			for (int i = 0; i < size; ++i) {
				m_Data[i].X = UChar2Float(data[i * 4]);
				m_Data[i].Y = UChar2Float(data[i * 4 + 1]);
				m_Data[i].Z = UChar2Float(data[i * 4 + 2]);
				m_Data[i].W = UChar2Float(data[i * 4 + 3]);
			}
		} else if (channels == 3) {
			//������ɫͨ�����洢����
			for (int i = 0; i < size; ++i) {
				m_Data[i].X = UChar2Float(data[i * 3]);
				m_Data[i].Y = UChar2Float(data[i * 3 + 1]);
				m_Data[i].Z = UChar2Float(data[i * 3 + 2]);
				m_Data[i].W = 0.0f;
			}
		} else if (channels == 2) {
			//������ɫͨ�����洢����
			for (int i = 0; i < size; ++i) {
				m_Data[i].X = UChar2Float(data[i * 2]);
				m_Data[i].Y = UChar2Float(data[i * 2 + 1]);
				m_Data[i].Z = 0.0f;
				m_Data[i].W = 0.0f;
			}
		} else 	if (channels == 1) {
				//������ɫͨ�����洢����
				for (int i = 0; i < size; ++i) {
					m_Data[i].X = UChar2Float(data[i]);
					m_Data[i].Y = 0.0f;
					m_Data[i].Z = 0.0f;
					m_Data[i].W = 0.0f;
				}
			}
	}
}