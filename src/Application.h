#pragma once
#include <string>
#include "Window.h"
#include "Renderer.h"
#include "BlinnShader.h"
#include <chrono>
#include <vector>
namespace RGS {
	struct Camera {
		Vec4 Pos = { 0.0f,0.0f,2.0f,1.0f };//��ʼ�������λ��
		Vec4 Right = { 1.0f,0.0f,0.0f,0.0f };//��ʼ���������ϵ��x��
		Vec4 Up = { 0.0f,1.0f,0.0f,0.0f };//��ʼ�������y��
		Vec4 Dir = { 0.0f,0.0f,-1.0f,0.0f };//��ʼ������ķ���
		float Aspect = 4.0f / 3.0f;//���������߱�
	};
	class Application
	{
	public:
		Application(const std::string name, const int width, const int height);
		~Application();
		void Run();
	private:
		void Init();
		void Terminate();
		//��Ⱦ֡ ÿһ֡�������״̬��Ӧ�ó���״̬��
		void OnUpdate(float time);
		//�����������
		void OnCmeraUpdate(float time);
		//����ģ�ͣ�����  �����������ݵ�Ӧ�ó�����
		void LoadMesh(const char* fileName);
	private:
		std::string m_Name;
		int m_Width;
		int m_Height;
		//���ڼ���֡���ʱ�䣬����ͬ�������͸��¡�
		std::chrono::steady_clock::time_point m_LastFrameTime;
		Window* m_Window = nullptr;
		Camera m_Camera;//�������
		std::vector<Triangle<BlinnVertex>>m_Mesh;//ģ�����ݣ������Σ�����
		BlinnUniforms m_Uniforms;//ͳһ���������գ��ȵ�
	};


}