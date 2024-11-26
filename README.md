# 软光栅化渲染器
从三维模型，纹理，到光栅化，坐标转换，裁剪顶点，过滤不可见顶点，重心坐标插值，纹理采样，纹理坐标插值，像素选取等等。
可以直接通过visual studio打开文件夹 编译器会自动cmake生成文件，直接点击Main.cpp运行即可
## 1. 几何处理
几何处理阶段主要负责处理3D模型数据，包括顶点的变换、裁剪和投影。
### 顶点变换
#### 模型变换：将模型从模型空间转换到世界空间。
#### 视图变换：将世界空间中的坐标转换到观察者视角的相机视图空间。
#### 投影变换：将视图空间中的坐标转换到裁剪空间，通常使用透视投影或正交投影。
### 裁剪
在投影变换后，顶点可能位于视锥体外。这些顶点对应的图元（如三角形）需要被裁剪或丢弃。
### 视口变换
将裁剪空间中的坐标映射到屏幕空间，这涉及到将标准化设备坐标（NDC）转换为屏幕像素坐标。
## 2. 光栅化
光栅化是将几何图形（通常是三角形）转换为屏幕上像素的过程。
#### 边界扫描：确定三角形覆盖哪些像素。
#### 重心坐标插值：对于三角形内的每个像素，使用重心坐标来插值顶点属性（如颜色、纹理坐标、法线等）。
## 3. 着色
着色阶段根据光照模型、材质属性和插值得到的顶点属性来计算每个像素的颜色。
#### 像素着色：根据光照模型（如Phong模型、Blinn-Phong模型）计算像素的颜色。
#### 纹理映射：如果使用纹理，需要根据插值得到的纹理坐标从纹理图像中采样颜色。
## 4. 显示
最后，将计算得到的像素颜色值写入到帧缓冲区，最终输出到屏幕上。
实现细节
实现一个基本的光栅化渲染器通常需要以下几个组件：
#### 数据结构：定义顶点、三角形、网格、材质和光源的数据结构。
#### 加载器：实现一个模型加载器，用于从文件中读取3D模型数据。
#### 变换管线：实现模型、视图和投影变换。
#### 光栅化器：实现三角形的光栅化算法，包括扫描转换和属性插值。
#### 着色器：实现顶点着色器和片元着色器，用于处理顶点属性和计算像素颜色。
#### 帧缓冲区：实现一个帧缓冲区，用于存储渲染结果。


![1732628977018](https://github.com/user-attachments/assets/6b72ac27-d074-4738-9c2e-28ec93057316)


![Uploading aca6574bc114fa3f61ff30a88b97f2e.png…]()

