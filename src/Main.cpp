#include<iostream>
#include "Application.h"
#include "Maths.h"
#include "BlinnShader.h"
int main(){


    //在堆上面分配内存，需要使用指针指向
    std::cout << "Hello RGS!" << std::endl;
    RGS::Application app("RGS", 800, 600);
    app.Run();
    std::cout << "Bye 0Bao!" << std::endl;
	

    return 0;
}