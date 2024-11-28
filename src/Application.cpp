#include "Application.h"
#include "base.h"
#include "Window.h"
#include "Maths.h"
#include "BlinnShader.h"
#include "Renderer.h"
#include "Framebuffer.h"
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <fstream>
#include <stdio.h>
namespace RGS {

	Application::Application(const std::string name, const int width, const int height) :m_Name(name), m_Width(width), m_Height(height)
	{
		Init();
	}

	Application::~Application()
	{
		Terminate();
	}

	void Application::Run()
	{
		//ֻҪ����û�йرվ�һֱ��Ⱦ
		while (!m_Window->Closed())
		{
			//��ǰ֡����Ⱦʱ��
			auto nowFrameTime = std::chrono::steady_clock::now();
			//ÿһ֡��Ⱦ��ʱ����
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(nowFrameTime - m_LastFrameTime);
			//ʱ�����
			float deltaTime = duration.count() * 0.001f * 0.001f;
			m_LastFrameTime = nowFrameTime;
			//���²���
			OnUpdate(deltaTime);
			Window::PollInputEvents();
		}
	}

	void Application::Init()
	{
		Window::Init();
		m_Window = RGS::Window::Create(m_Name, m_Width, m_Height);//�����Ĵ��ڳߴ�
		//��¼��ǰ֡��ʱ��
		m_LastFrameTime = std::chrono::high_resolution_clock::now();
		//����ģ��
		LoadMesh("D:\\c++\\rgs_renderer\\assets\\box.obj");
		//������������
		m_Uniforms.Diffuse = new Texture("D:\\c++\\rgs_renderer\\assets\\container2.png");
		m_Uniforms.Specular = new Texture("D:\\c++\\rgs_renderer\\assets\\container2_specular.png");
	}

	void Application::Terminate()
	{
		delete m_Uniforms.Diffuse;
		delete m_Uniforms.Specular;

		delete m_Window;
		Window::Terminate();
	}

	void Application::OnUpdate(float time)
	{
		//��������ĸı�
		OnCmeraUpdate(time);//���������Ĳ���
		//��ʼ��֡������
		Framebuffer framebuffer(m_Width, m_Height);//���ڴ�С
		Program program(BlinnVertexShader, BlinnFragmentShader);
		//�����ӿھ��󣬴�������Ĳ��������λ�ú�Ŀ��λ�ã�y�ᣬ����������
		Mat4 view = Mat4LookAt(m_Camera.Pos, m_Camera.Pos + m_Camera.Dir, { 0.0f,1.0f,0.0f });
		//����͸�Ӿ��󣬴�������ڲ�
		Mat4 proj = Mat4Perspective(90.0f / 360.0f * 2.0f * PI, m_Camera.Aspect, 0.1f, 100.0f);
		Mat4 model = Mat4Identity();//ģ�;��󣬽�ģ�Ͷ�������ת������������ϵ��
		m_Uniforms.MVP = proj * view * model;
		m_Uniforms.CamerasPos = m_Camera.Pos;
		m_Uniforms.Model = model;
		m_Uniforms.ModelNormalToWorld = Mat4Identity();
		//��������ÿһ֡�����仯
		//m_Uniforms.Shininess *= std::pow(2, time * 2.0f);
		if (m_Uniforms.Shininess > 256.0f) {
			m_Uniforms.Shininess = 2.0f;
		}
		//��ʼ��������Ҫ����Ⱦ�õõ��Ĳ���֮�󣬽���ÿ�������εĻ��ƣ���ɫ��Ⱦ,д��֡���������У��洢ÿ֡�� ������ɫ�Լ����ֵ
		for (auto tri : m_Mesh) {
			//��Ⱦÿ��������
			Renderer::Draw(framebuffer, program, tri, m_Uniforms);
		}
		//��Ⱦÿһ֡����
		m_Window->DrawFramebuffer(framebuffer);

	}
	void Application::OnCmeraUpdate(float time)
	{
		constexpr float speed = 1.0f;
		if (m_Window->GetKey(RGS_KEY_SPACE) == RGS_PRESS)
			m_Camera.Pos = m_Camera.Pos + speed * time * m_Camera.Up;
		if (m_Window->GetKey(RGS_KEY_LEFT_SHIFT) == RGS_PRESS)
			m_Camera.Pos = m_Camera.Pos - speed * time * m_Camera.Up;
		if (m_Window->GetKey(RGS_KEY_D) == RGS_PRESS)
			m_Camera.Pos = m_Camera.Pos + speed * time * m_Camera.Right;
		if (m_Window->GetKey(RGS_KEY_A) == RGS_PRESS)
			m_Camera.Pos = m_Camera.Pos - speed * time * m_Camera.Right;
		if (m_Window->GetKey(RGS_KEY_W) == RGS_PRESS)
			m_Camera.Pos = m_Camera.Pos + speed * time * m_Camera.Dir;
		if (m_Window->GetKey(RGS_KEY_S) == RGS_PRESS)
			m_Camera.Pos = m_Camera.Pos - speed * time * m_Camera.Dir;

		constexpr float rotateSpeed = 1.0f;
		Mat4 rotation = Mat4Identity();
		//ֻ������ķ������ı䣬λ�ò�û�иı�
		//�����y����ת
		if (m_Window->GetKey(RGS_KEY_Q) == RGS_PRESS)
		{
			rotation = Mat4RotateY(time * rotateSpeed);
			m_Camera.Dir = rotation * m_Camera.Dir;//z�ᷢ���ı���
			m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
			m_Camera.Right = rotation * m_Camera.Right;
			m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
		}

		if (m_Window->GetKey(RGS_KEY_E) == RGS_PRESS)
		{
			rotation = Mat4RotateY(-time * rotateSpeed);
			m_Camera.Dir = rotation * m_Camera.Dir;//z�ᷢ���ı���
			m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
			m_Camera.Right = rotation * m_Camera.Right;
			m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
		}
		//�������ת
		if (m_Window->GetKey(RGS_KEY_Z) == RGS_PRESS)
		{
			rotation = Mat4RotateZ(time * rotateSpeed);
			m_Camera.Up = rotation * m_Camera.Up;
			m_Camera.Up = { Normalize(m_Camera.Up), 0.0f };
			m_Camera.Right = rotation * m_Camera.Right;
			m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
		}
		//�������ת
		if (m_Window->GetKey(RGS_KEY_B) == RGS_PRESS)
		{
			rotation = Mat4RotateZ(-time * rotateSpeed);
			m_Camera.Up = rotation * m_Camera.Up;
			m_Camera.Up = { Normalize(m_Camera.Up), 0.0f };
			m_Camera.Right = rotation * m_Camera.Right;
			m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
		}
		//�������ת
		if (m_Window->GetKey(RGS_KEY_X) == RGS_PRESS)
		{
			rotation = Mat4RotateX(time * rotateSpeed);
			m_Camera.Up = rotation * m_Camera.Up;
			m_Camera.Up = { Normalize(m_Camera.Up), 0.0f };
			m_Camera.Dir = rotation * m_Camera.Dir;//z�ᷢ���ı���
			m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
		}
		//�������ת
		if (m_Window->GetKey(RGS_KEY_C) == RGS_PRESS)
		{
			rotation = Mat4RotateX(-time * rotateSpeed);
			m_Camera.Up = rotation * m_Camera.Up;
			m_Camera.Up = { Normalize(m_Camera.Up), 0.0f };
			m_Camera.Dir = rotation * m_Camera.Dir;//z�ᷢ���ı���
			m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
		}
		//z�ᷢ���ı���ֻ��z��ķ������ı�Żᵼ����ȾЧ�������仯
	}

	void Application::LoadMesh(const char* fileName)
	{
		std::ifstream file(fileName);
		ASSERT(file);
		std::vector<Vec3>positions;//����
		std::vector<Vec2>texCoords;//��������
		std::vector<Vec3>normals;//���㷨����
		//�������ݵ��±�����
		std::vector<int>posIndices;
		std::vector<int>texIndices;
		std::vector<int>normalIndices;
		std::string line;
		//��ȡ���ļ�ĩβ
		while (!file.eof()) {
			//��ȡÿ������
			std::getline(file, line);
			int items = -1;
			if (line.find("v ") == 0)//��������
			{
				Vec3 Position;
				items = sscanf(line.c_str(), "v %f %f %f", &Position.X, &Position.Y, &Position.Z);
				ASSERT(items == 3);
				positions.push_back(Position);
			}
			else if (line.find("vt ") == 0) {//��������
				Vec2 texCoord;
				items = sscanf(line.c_str(), "vt %f %f", &texCoord.X, &texCoord.Y);
				ASSERT(items == 2);
				texCoords.push_back(texCoord);
			}
			else if (line.find("vn ") == 0) {//������
				Vec3 normal;
				items = sscanf(line.c_str(), "vn %f %f %f", &normal.X, &normal.Y, &normal.Z);
				ASSERT(items == 3);
				normals.push_back(normal);
			}
			else if (line.find("f ") == 0) {//��Ƭ
				int pIndices[3], uvIndices[3], nIndices[3];
				//�õ���Ƭ��Ӧ�Ķ��㣬����������������
				items = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
					&pIndices[0], &uvIndices[0], &nIndices[0],
					&pIndices[1], &uvIndices[1], &nIndices[1],
					&pIndices[2], &uvIndices[2], &nIndices[2]);
				ASSERT(items == 9);
				//ÿ��������һ�������ε�������������ά���������������������������㷨����
				for (int i = 0; i < 3; i++)
				{
					posIndices.push_back(pIndices[i] - 1);
					texIndices.push_back(uvIndices[i] - 1);
					normalIndices.push_back(nIndices[i] - 1);
				}
			}
		}
		file.close();
		int triNum = posIndices.size() / 3;//�����ε�����
		for (int i = 0; i < triNum; ++i) {
			Triangle<BlinnVertex> triangle;//�������������ģ�����꣬������
			//���α��������ε�������������
			for (int j = 0; j < 3; ++j) {
				int index = 3 * i + j;
				int posIndex = posIndices[index];
				int texIndex = texIndices[index];
				int nIndex = normalIndices[index];
				triangle[j].ModelPos = { positions[posIndex],1 };
				triangle[j].TexCoord = texCoords[texIndex];
				triangle[j].ModelNormal = normals[nIndex];
			}
			//�洢ÿ��������
			m_Mesh.push_back(triangle);
		}
	}

}