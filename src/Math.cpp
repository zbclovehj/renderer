#include "Maths.h"

#include "base.h"
#include <cmath>

namespace RGS {

    Vec2 operator+ (const Vec2& left, const Vec2& right)
    {
        return Vec2{ left.X + right.X, left.Y + right.Y };
    }
    Vec2 operator- (const Vec2& left, const Vec2& right)
    {
        return Vec2{ left.X - right.X, left.Y - right.Y };
    }

    Vec3 operator+ (const Vec3& left, const Vec3& right)
    {
        return Vec3{ left.X + right.X, left.Y + right.Y, left.Z + right.Z };
    }
    Vec3 operator- (const Vec3& left, const Vec3& right)
    {
        return left + (-1.0f * right);
    }
    Vec3 operator* (const float left, const Vec3& right)
    {
        return Vec3{ left * right.X, left * right.Y, left * right.Z };
    }
    Vec3 operator* (const Vec3& left, const float right)
    {
        return right * left;
    }
    Vec3 operator* (const Vec3& left, const Vec3& right)
    {
        return { left.X * right.X, left.Y * right.Y , left.Z * right.Z };
    }
    Vec3 operator/ (const Vec3& left, const float right)
    {
        ASSERT(right != 0);
        return left * (1.0f / right);
    }

    float Dot(const Vec3& left, const Vec3& right)
    {
        return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
    }
    Vec3 Cross(const Vec3& left, const Vec3& right)
    {
        float x = left.Y * right.Z - left.Z * right.Y;
        float y = left.Z * right.X - left.X * right.Z;
        float z = left.X * right.Y - left.Y * right.X;
        return { x, y, z };
    }
    Vec3 Normalize(const Vec3& v)
    {
        float len = (float)std::sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
        ASSERT(len != 0);
        return v / len;
    }

    Vec4 operator+ (const Vec4& left, const Vec4& right)
    {
        return Vec4{ left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W };
    }
    Vec4 operator- (const Vec4& left, const Vec4& right)
    {
        return Vec4{ left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W };
    }
    Vec4 operator* (const float left, const Vec4& right)
    {
        return Vec4{ left * right.X, left * right.Y, left * right.Z, left * right.W };
    }
    Vec4 operator* (const Vec4& left, const float right)
    {
        return right * left;
    }
    Vec4 operator/ (const Vec4& left, const float right)
    {
        ASSERT(right != 0);
        return left * (1.0f / right);
    }

    Vec4 operator* (const Mat4& mat4, const Vec4& vec4)
    {
        Vec4 res;
        res.X = mat4.M[0][0] * vec4.X + mat4.M[0][1] * vec4.Y + mat4.M[0][2] * vec4.Z + mat4.M[0][3] * vec4.W;
        res.Y = mat4.M[1][0] * vec4.X + mat4.M[1][1] * vec4.Y + mat4.M[1][2] * vec4.Z + mat4.M[1][3] * vec4.W;
        res.Z = mat4.M[2][0] * vec4.X + mat4.M[2][1] * vec4.Y + mat4.M[2][2] * vec4.Z + mat4.M[2][3] * vec4.W;
        res.W = mat4.M[3][0] * vec4.X + mat4.M[3][1] * vec4.Y + mat4.M[3][2] * vec4.Z + mat4.M[3][3] * vec4.W;
        return res;
    }

    Mat4::Mat4(const Vec4& v0, const Vec4& v1, const Vec4& v2, const Vec4& v3)
    {
        M[0][0] = v0.X; M[1][0] = v0.Y; M[2][0] = v0.Z; M[3][0] = v0.W;
        M[0][1] = v1.X; M[1][1] = v1.Y; M[2][1] = v1.Z; M[3][1] = v1.W;
        M[0][2] = v2.X; M[1][2] = v2.Y; M[2][2] = v2.Z; M[3][2] = v2.W;
        M[0][3] = v3.X; M[1][3] = v3.Y; M[2][3] = v3.Z; M[3][3] = v3.W;
    }
    Mat4 operator* (const Mat4& left, const Mat4& right)
    {
        Mat4 res;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    res.M[i][j] += left.M[i][k] * right.M[k][j];
                }
            }
        }
        return res;
    }
    Mat4& operator*= (Mat4& left, const Mat4& right)
    {
        left = left * right;
        return left;
    }
    //矩阵变换要注意乘的顺序以及左乘还是右乘

    Mat4 Mat4Identity()
    {
        return Mat4({ 1,0,0,0 }, { 0,1,0,0 }, { 0,0,1,0 }, { 0,0,0,1 });
    }
    //通过平移矩阵实现物体在相机坐标系下的移动
    Mat4 Mat4Translate(float tx, float ty, float tz)
    {
        Mat4 m = Mat4Identity();
        m.M[0][3] = tx;
        m.M[1][3] = ty;
        m.M[2][3] = tz;
        return m;
    }
    //通过缩放矩阵，实现物体在相机坐标系下的尺寸改变
    Mat4 Mat4Scale(float sx, float sy, float sz)
    {
        Mat4 m = Mat4Identity();
        ASSERT(sx != 0 && sy != 0 && sz != 0);
        m.M[0][0] = sx;
        m.M[1][1] = sy;
        m.M[2][2] = sz;
        return m;
    }
    //通过旋转矩阵，实现物体在相机坐标系下的物体的旋转
    //绕哪个轴旋转改变的坐标是另外两个轴下的坐标
    //计算旋转矩阵要引入另一个角度，通过和旋转角度计算会消去引入的这个角度，从而的到
    //计算公式
    Mat4 Mat4RotateX(float angle)
    {
        float c = (float)std::cos(angle);
        float s = (float)std::sin(angle);
        Mat4 m = Mat4Identity();
        m.M[1][1] = c;
        m.M[1][2] = -s;
        m.M[2][1] = s;
        m.M[2][2] = c;
        return m;
    }
    Mat4 Mat4RotateY(float angle)
    {
        float c = (float)std::cos(angle);
        float s = (float)std::sin(angle);
        Mat4 m = Mat4Identity();
        m.M[2][2] = c;
        m.M[2][0] = -s;
        m.M[0][2] = s;
        m.M[0][0] = c;
        return m;
    }
    Mat4 Mat4RotateZ(float angle)
    {
        float c = (float)std::cos(angle);
        float s = (float)std::sin(angle);
        //单位矩阵
        Mat4 m = Mat4Identity();
        m.M[0][0] = c;
        m.M[0][1] = -s;
        m.M[1][0] = s;
        m.M[1][1] = c;
        return m;
    }
    //建立相机坐标系
    // 旋转矩阵就是相机的朝向
    //也就是x,y,z轴 每个轴方向
    //当相机移动时，就可以计算在相机移动，但是朝向没有变得情况下，物体在相机坐标系的位置
   //如果相机的朝向变了，那么要重新计算旋转矩阵，位置变了，就是重新计算平移向量
    //通过改变相机的旋转矩阵和平移矩阵（相机的朝向和位置），
    //可以改变物体在屏幕上de呈现方式
    //物体渲染的旋转和移动，其实一般是通过对相机的变换矩阵改变（旋转平移）实现的
    //如果相机朝向或者位置改变，则要重新计算lookat矩阵，
    //然后重新计算顶点在相机坐标系下的位置
    Mat4 Mat4LookAt(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis, const Vec3& eye)
    {
        Mat4 m = Mat4Identity();
        //前面3x3的矩阵，本质上就是计算原来坐标系下的顶点到原点的向量，
        //在新坐标系下的每个轴方向上面的投影，得到这个顶点在新坐标系下的每个轴方向上的坐标，
        //这样就实现了，从世界坐标系到相机坐标系的旋转转换，但是考虑到相机的移动，会导致
        //相机坐标系的原点发生改变，所以又要计算平移对于坐标的影响
        m.M[0][0] = xAxis.X;
        m.M[0][1] = xAxis.Y;
        m.M[0][2] = xAxis.Z;

        m.M[1][0] = yAxis.X;
        m.M[1][1] = yAxis.Y;
        m.M[1][2] = yAxis.Z;

        m.M[2][0] = zAxis.X;
        m.M[2][1] = zAxis.Y;
        m.M[2][2] = zAxis.Z;
        /*
        当相机在空间中平移时，例如从位置 
𝐴 移动到位置 𝐵，相机的坐标系的原点会改变，
但是其轴向量（即相机的朝向）保持不变，除非进行了额外的旋转操作。
这意味着相机看向的方向和定义的“上”方向没有改变，只是其在空间中的位置发生了变化。
        */
        //通过点积来计算平移向量，
        //具体来说是摄像机移动到的位置到原点的向量与摄像机坐标系各轴向量的点积，也就是投影。
        // 这一步骤的目的是确定摄像机在各轴上的位置，投影表示相机在每个轴上面移动多少
        // 以便正确地将场景中的点转换到摄像机视角下的坐标系中。
        //取反表示相机移动，则目标场景的坐标是往相反的方向移动的
        m.M[0][3] = -Dot(xAxis, eye);
        m.M[1][3] = -Dot(yAxis, eye);
        m.M[2][3] = -Dot(zAxis, eye);

        return m;
    }
    //计算视口矩阵，一般是开始固定的，也就是相机的朝向和相机的位置，后续相机的
    Mat4 Mat4LookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
    {
        //初始化朝向，作为相机的旋转矩阵
        //获得z轴，相机位置到目标物体位置的向量作为z轴，也就是两者的距离
        Vec3 zAxis = Normalize(eye - target);
        Vec3 xAxis = Normalize(Cross(up, zAxis));
        Vec3 yAxis = Normalize(Cross(zAxis, xAxis));
        //再通过叉乘获得x轴和y轴，通过三个轴和
        return Mat4LookAt(xAxis, yAxis, zAxis, eye);
    }
    /*
    * fovy: the field of view angle in the y direction, in radians
    * aspect: the aspect ratio, defined as width divided by height
    * near, far: the distances to the near and far depth clipping planes
    *
    * 1/(aspect*tan(fovy/2))              0             0           0
    *                      0  1/tan(fovy/2)             0           0
    *                      0              0  -(f+n)/(f-n)  -2fn/(f-n)
    *                      0              0            -1           0
    *
    * this is the same as
    *     float half_h = near * (float)tan(fovy / 2);
    *     float half_w = half_h * aspect;
    *     mat4_frustum(-half_w, half_w, -half_h, half_h, near, far);
    *
    * see http://www.songho.ca/opengl/gl_projectionmatrix.html
    */
    Mat4 Mat4Perspective(float fovy, float aspect, float near, float far)
    {
        //fovy在y轴上的视锥角度
        float z_range = far - near;
        Mat4 m = Mat4Identity();
        ASSERT(fovy > 0 && aspect > 0);
        ASSERT(near > 0 && far > 0 && z_range > 0);
        // 1/(aspect*tan(fovy/2)) aspect宽纵比，得到在x轴的视锥角度
        m.M[1][1] = 1 / (float)std::tan(fovy / 2);
        m.M[0][0] = m.M[1][1] / aspect;
        m.M[2][2] = -(near + far) / z_range;
        m.M[2][3] = -2 * near * far / z_range;
        m.M[3][2] = -1;
        m.M[3][3] = 0;
        return m;
    }
    //这两部分相加就得到了在 start 和 end 之间，根据 t 插值的结果。
    float Lerp(const float start, const float end, const float t)
    {
        return end * t + start * (1.0f - t);
    }

    Vec3 Lerp(const Vec3& start, const Vec3& end, const float t)
    {
        return end * t + start * (1.0f - t);
    }

    Vec4 Lerp(const Vec4& start, const Vec4& end, const float t)
    {
        return end * t + start * (1.0f - t);
    }

    unsigned char Float2UChar(const float f)
    {
        return (unsigned char)(f * 255.0f);
    }
    float UChar2Float(const unsigned char c)
    {
        return (float)c / 255.0f;
    }

    float Clamp(const float val, const float min, const float max)
    {
        if (val < min)
        {
            return min;
        }
        else if (val > max)
        {
            return max;
        }
        else
        {
            return val;
        }
    }

}