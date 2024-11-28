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
		//只要窗口没有关闭九一直渲染
		while (!m_Window->Closed())
		{
			//当前帧的渲染时刻
			auto nowFrameTime = std::chrono::steady_clock::now();
			//每一帧渲染的时间间隔
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(nowFrameTime - m_LastFrameTime);
			//时间参数
			float deltaTime = duration.count() * 0.001f * 0.001f;
			m_LastFrameTime = nowFrameTime;
			//更新参数
			OnUpdate(deltaTime);
			Window::PollInputEvents();
		}
	}

	void Application::Init()
	{
		Window::Init();
		m_Window = RGS::Window::Create(m_Name, m_Width, m_Height);//创建的窗口尺寸
		//记录当前帧的时间
		m_LastFrameTime = std::chrono::high_resolution_clock::now();
		//导入模型
		LoadMesh("D:\\c++\\rgs_renderer\\assets\\box.obj");
		//创建纹理数据
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
		//相机参数的改变
		OnCmeraUpdate(time);//参数增长的步长
		//初始化帧缓冲区
		Framebuffer framebuffer(m_Width, m_Height);//窗口大小
		Program program(BlinnVertexShader, BlinnFragmentShader);
		//计算视口矩阵，传入相机的参数，相机位置和目标位置，y轴，传入相机外参
		Mat4 view = Mat4LookAt(m_Camera.Pos, m_Camera.Pos + m_Camera.Dir, { 0.0f,1.0f,0.0f });
		//计算透视矩阵，传入相机内参
		Mat4 proj = Mat4Perspective(90.0f / 360.0f * 2.0f * PI, m_Camera.Aspect, 0.1f, 100.0f);
		Mat4 model = Mat4Identity();//模型矩阵，将模型顶点坐标转化到世界坐标系中
		m_Uniforms.MVP = proj * view * model;
		m_Uniforms.CamerasPos = m_Camera.Pos;
		m_Uniforms.Model = model;
		m_Uniforms.ModelNormalToWorld = Mat4Identity();
		//光照随着每一帧发生变化
		//m_Uniforms.Shininess *= std::pow(2, time * 2.0f);
		if (m_Uniforms.Shininess > 256.0f) {
			m_Uniforms.Shininess = 2.0f;
		}
		//初始化各种需要在渲染用得到的参数之后，进行每个三角形的绘制，着色渲染,写入帧缓冲区当中，存储每帧的 像素颜色以及深度值
		for (auto tri : m_Mesh) {
			//渲染每个三角形
			Renderer::Draw(framebuffer, program, tri, m_Uniforms);
		}
		//渲染每一帧画面
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
		//只有相机的方向发生改变，位置并没有改变
		//相机绕y轴旋转
		if (m_Window->GetKey(RGS_KEY_Q) == RGS_PRESS)
		{
			rotation = Mat4RotateY(time * rotateSpeed);
			m_Camera.Dir = rotation * m_Camera.Dir;//z轴发生改变了
			m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
			m_Camera.Right = rotation * m_Camera.Right;
			m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
		}

		if (m_Window->GetKey(RGS_KEY_E) == RGS_PRESS)
		{
			rotation = Mat4RotateY(-time * rotateSpeed);
			m_Camera.Dir = rotation * m_Camera.Dir;//z轴发生改变了
			m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
			m_Camera.Right = rotation * m_Camera.Right;
			m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
		}
		//相机的旋转
		if (m_Window->GetKey(RGS_KEY_Z) == RGS_PRESS)
		{
			rotation = Mat4RotateZ(time * rotateSpeed);
			m_Camera.Up = rotation * m_Camera.Up;
			m_Camera.Up = { Normalize(m_Camera.Up), 0.0f };
			m_Camera.Right = rotation * m_Camera.Right;
			m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
		}
		//相机的旋转
		if (m_Window->GetKey(RGS_KEY_B) == RGS_PRESS)
		{
			rotation = Mat4RotateZ(-time * rotateSpeed);
			m_Camera.Up = rotation * m_Camera.Up;
			m_Camera.Up = { Normalize(m_Camera.Up), 0.0f };
			m_Camera.Right = rotation * m_Camera.Right;
			m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
		}
		//相机的旋转
		if (m_Window->GetKey(RGS_KEY_X) == RGS_PRESS)
		{
			rotation = Mat4RotateX(time * rotateSpeed);
			m_Camera.Up = rotation * m_Camera.Up;
			m_Camera.Up = { Normalize(m_Camera.Up), 0.0f };
			m_Camera.Dir = rotation * m_Camera.Dir;//z轴发生改变了
			m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
		}
		//相机的旋转
		if (m_Window->GetKey(RGS_KEY_C) == RGS_PRESS)
		{
			rotation = Mat4RotateX(-time * rotateSpeed);
			m_Camera.Up = rotation * m_Camera.Up;
			m_Camera.Up = { Normalize(m_Camera.Up), 0.0f };
			m_Camera.Dir = rotation * m_Camera.Dir;//z轴发生改变了
			m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
		}
		//z轴发生改变了只有z轴的方向发生改变才会导致渲染效果发生变化
	}

	void Application::LoadMesh(const char* fileName)
	{
		std::ifstream file(fileName);
		ASSERT(file);
		std::vector<Vec3>positions;//坐标
		std::vector<Vec2>texCoords;//纹理坐标
		std::vector<Vec3>normals;//顶点法向量
		//三种数据的下标索引
		std::vector<int>posIndices;
		std::vector<int>texIndices;
		std::vector<int>normalIndices;
		std::string line;
		//读取到文件末尾
		while (!file.eof()) {
			//读取每行数据
			std::getline(file, line);
			int items = -1;
			if (line.find("v ") == 0)//顶点坐标
			{
				Vec3 Position;
				items = sscanf(line.c_str(), "v %f %f %f", &Position.X, &Position.Y, &Position.Z);
				ASSERT(items == 3);
				positions.push_back(Position);
			}
			else if (line.find("vt ") == 0) {//纹理坐标
				Vec2 texCoord;
				items = sscanf(line.c_str(), "vt %f %f", &texCoord.X, &texCoord.Y);
				ASSERT(items == 2);
				texCoords.push_back(texCoord);
			}
			else if (line.find("vn ") == 0) {//法向量
				Vec3 normal;
				items = sscanf(line.c_str(), "vn %f %f %f", &normal.X, &normal.Y, &normal.Z);
				ASSERT(items == 3);
				normals.push_back(normal);
			}
			else if (line.find("f ") == 0) {//面片
				int pIndices[3], uvIndices[3], nIndices[3];
				//得到面片对应的顶点，纹理，法向量的索引
				items = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
					&pIndices[0], &uvIndices[0], &nIndices[0],
					&pIndices[1], &uvIndices[1], &nIndices[1],
					&pIndices[2], &uvIndices[2], &nIndices[2]);
				ASSERT(items == 9);
				//每三个都是一个三角形的属性索引，三维顶点索引，纹理坐标索引，顶点法向量
				for (int i = 0; i < 3; i++)
				{
					posIndices.push_back(pIndices[i] - 1);
					texIndices.push_back(uvIndices[i] - 1);
					normalIndices.push_back(nIndices[i] - 1);
				}
			}
		}
		file.close();
		int triNum = posIndices.size() / 3;//三角形的数量
		for (int i = 0; i < triNum; ++i) {
			Triangle<BlinnVertex> triangle;//传入纹理坐标和模型坐标，法向量
			//依次遍历三角形的三个顶点属性
			for (int j = 0; j < 3; ++j) {
				int index = 3 * i + j;
				int posIndex = posIndices[index];
				int texIndex = texIndices[index];
				int nIndex = normalIndices[index];
				triangle[j].ModelPos = { positions[posIndex],1 };
				triangle[j].TexCoord = texCoords[texIndex];
				triangle[j].ModelNormal = normals[nIndex];
			}
			//存储每个三角形
			m_Mesh.push_back(triangle);
		}
	}

}