#pragma once
#include <string>
#include "Window.h"
#include "Renderer.h"
#include "BlinnShader.h"
#include <chrono>
#include <vector>
namespace RGS {
	struct Camera {
		Vec4 Pos = { 0.0f,0.0f,2.0f,1.0f };//初始化相机的位置
		Vec4 Right = { 1.0f,0.0f,0.0f,0.0f };//初始化相机坐标系的x轴
		Vec4 Up = { 0.0f,1.0f,0.0f,0.0f };//初始化相机的y轴
		Vec4 Dir = { 0.0f,0.0f,-1.0f,0.0f };//初始化相机的方向
		float Aspect = 4.0f / 3.0f;//相机参数宽高比
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
		//渲染帧 每一帧更新相机状态和应用程序状态。
		void OnUpdate(float time);
		//更新相机参数
		void OnCmeraUpdate(float time);
		//导入模型，输入  加载网格数据到应用程序中
		void LoadMesh(const char* fileName);
	private:
		std::string m_Name;
		int m_Width;
		int m_Height;
		//用于计算帧间隔时间，帮助同步动画和更新。
		std::chrono::steady_clock::time_point m_LastFrameTime;
		Window* m_Window = nullptr;
		Camera m_Camera;//相机数据
		std::vector<Triangle<BlinnVertex>>m_Mesh;//模型数据，三角形，顶点
		BlinnUniforms m_Uniforms;//统一变量，光照，等等
	};


}