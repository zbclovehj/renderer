#include<iostream>
#include "Application.h"
#include "Maths.h"
#include "BlinnShader.h"
int main(){


    //�ڶ���������ڴ棬��Ҫʹ��ָ��ָ��
    std::cout << "Hello RGS!" << std::endl;
    RGS::Application app("RGS", 800, 600);
    app.Run();
    std::cout << "Bye 0Bao!" << std::endl;
	

    return 0;
}