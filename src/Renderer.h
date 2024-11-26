#pragma once
#include "Framebuffer.h"
#include "ShaderBase.h"
#include "base.h"
#include "Maths.h"
#include <type_traits>
#include <cmath>

namespace RGS {
	template<typename vertex_t>
	struct Triangle {
		static_assert(std::is_base_of_v<RGS::VertexBase, vertex_t>, "vertex_t ����̳�VertexBase");
		vertex_t Vertex[3];//�������㣬����������
		//���ڽṹ��Triangle��˵ ����[]�������i,���ص���vertex_t���͵ĵ�i���������ζ��㣩
		//�������ǲ��������غ���������ʹ���±������ [] ������ Triangle �Ķ��㡣
		//��һ���汾����һ�����޸ĵ����ã����ڷǳ��� Triangle ����
		vertex_t& operator[](size_t i) { return Vertex[i]; }
		//�ڶ����汾����һ���������ã����ڳ��� Triangle ����
		const vertex_t& operator[](size_t i) const { return Vertex[i]; }
		Triangle() = default;
		//��Ԫ���������� Triangle ����ʹ�� << ������������������������� Triangle ����ϸ��Ϣ��
		//����ÿ����������ݡ�ѭ�������������㣬��ʹ�� << ��������ÿ���������Ϣ����� std::ostream ���� os �С�
		friend std::ostream& operator<<(std::ostream & os, const Triangle<vertex_t>&triangle)
		{
			os << "Triangle: {\n";
			for (int i = 0; i < 3; ++i) {
				os << "  Vertex[" << i << "]: " << triangle.Vertex[i] << "\n";
			}
			os << "}";
			return os;
		}
	};
	//��Ȳ��Ե�����
	enum class DepthFuncType {
		LESS,
		LEQUAL,
		ALWAYS,
	};
	template<typename vertex_t,typename uniforms_t,typename varyings_t>
	struct Program {
		bool EnableDepthTest = true;
		bool EnableWriteDepth = true;
		bool EnableDoubleSided = false;
		bool EnableBlend = false;
		DepthFuncType DepthFunc = DepthFuncType::LESS;
		//����һ����Ϊ vertex_shader_t �����ͱ���������һ��ָ������ָ�롣��������������������������� void
		using vertex_shader_t = void(*)(varyings_t&, const vertex_t&, const uniforms_t&);
		vertex_shader_t VertexShader;
		using fragment_shader_t = Vec4(*)(bool& discard,const varyings_t&, const uniforms_t&);
		fragment_shader_t FragmentShader;
		//����������ɫ�������� ������ɫ����Ƭ����ɫ��
		Program(const vertex_shader_t vertexShader,const fragment_shader_t fragmentShader):VertexShader(vertexShader),FragmentShader(fragmentShader){}

	};
	class Renderer {
		/*
static������ؼ��ֱ�ʾ RGS_MAX_VARYINGS ��һ����̬��Ա��������̬��Ա���������౾������������κ��ض���������ζ������ʵ������ͬһ�� RGS_MAX_VARYINGS ������
constexpr������ؼ������ڶ��峣�����ʽ�����ڱ���ʱ����ȷ����ֵ�ı��ʽ�������ȷ�� RGS_MAX_VARYINGS ��ֵ�ڱ���ʱ����֪�ģ�����������С��ģ������ȳ����ǳ����á�
		*/

	private:
		static constexpr int RGS_MAX_VARYINGS = 9;
	private:
		//������������ά�ռ�������ڸ���������������ƽ�档
		enum class Plane
		{
			POSITIVE_W,
			POSITIVE_X,
			NEGATIVE_X,
			POSITIVE_Y,
			NEGATIVE_Y,
			POSITIVE_Z,
			NEGATIVE_Z,
		};
		struct BoundingBox
		{
			int MinX, MaxX, MinY, MaxY;
		};
		static bool IsVertexVisible(const Vec4& clipPos);
		static bool IsInsidePlane(const Vec4& clipPos, const Plane plane);
		static bool IsInsideTriangle(float(&weights)[3]);
		static bool IsBackFacing(const Vec4& a, const Vec4& b, const Vec4& c);
		static bool PassDepthTest(const float writeDepth, const float fDepth, const DepthFuncType depthFunc);
		static float GetIntersectRatio(const Vec4& prev, const Vec4& curr, const Plane plane);
		//Ƭ����ɫ���İ�Χ��
		static BoundingBox GetBoundingBox(const Vec4(&fragCoords)[3], const int width, const int height);
		static void CalculateWeights(float(&screenWeights)[3], float(&weights)[3], const Vec4(&fragCoords)[3], const Vec2& screenPoint);
		//�Ա����������Բ�ֵ ����ü��Ķ�������
		template<typename varyings_t>
		static void LerpVaryings(varyings_t& out, const varyings_t& start, const varyings_t& end, const float ratio) {

			constexpr uint32_t floatNum = sizeof(varyings_t) / sizeof(float);

			float* startFloat = (float*)&start;
			float* endFloat = (float*)&end;
			float* outFloat = (float*)&out;
			for (int i = 0; i < (int)floatNum; ++i) {
				//���Բ�ֵ
				outFloat[i] = Lerp(startFloat[i], endFloat[i], ratio);
			}
		}
		//����Ȩ�ز�ֵ��������ü��ռ䣬��һ�������Լ���Ļ���ؿռ�� ��ֵ���� ����Ǽ����������ڲ�������
		template<typename varyings_t>
		static void LerpVaryings(
			varyings_t& out,
			const varyings_t(&varyings)[3],
			const float(&weights)[3],
			const int width,
			const int height) {

			/*
			��ֵ���㣺
out.ClipPos�������ֵ��Ĳü��ռ�λ�ã�Clip Position��������ͨ����ÿ������� ClipPos ����Ӧ��Ȩ����˲������ʵ�ֵġ�
��һ���豸���꣨NDC�����㣺
out.NdcPos�������һ���豸���꣨Normalized Device Coordinates��������ͨ������ֵ��Ĳü��ռ����������������� W ��ʵ�ֵģ����ڽ�����ת������׼�����豸����ռ䡣
out.NdcPos.W������ NdcPos �� W ������������NDC�ռ��У�W Ӧ����1������ͨ��ȡ ClipPos.W �ĵ������������á�
			*/
			out.ClipPos = varyings[0].ClipPos * weights[0] +
				varyings[1].ClipPos * weights[1] +
				varyings[2].ClipPos * weights[2];

			out.NdcPos = out.ClipPos / out.ClipPos.W;
			out.NdcPos.W = 1.0f / out.ClipPos.W;
			//��NDC����ת��Ϊ��Ļ�ռ����ꡣX �� Y ����ͨ��ӳ�䵽 [0, 1] ��Χ�����Կ�Ⱥ͸߶������㡣Z ����Ҳӳ�䵽 [0, 1] ��Χ��
			//��[0, 1]��Χ��X����ӳ�䵽[0, width]��Χ������Ļ�Ŀ�ȣ��Ӷ��õ�ʵ�ʵ���Ļ�������ꡣ
			//��[0, 1]��Χ��Y����ӳ�䵽[0, height]��Χ������Ļ�ĸ߶ȣ��Ӷ��õ�ʵ�ʵ���Ļ�������ꡣ
			out.FragPos.X = ((out.NdcPos.X + 1.0f) * 0.5 * width);
			out.FragPos.Y = ((out.NdcPos.Y + 1.0f) * 0.5 * height);
			out.FragPos.Z = (out.NdcPos.Z + 1.0f) * 0.5f;
			out.FragPos.W = out.NdcPos.W;
			//������Щ�����ƫ��
			constexpr uint32_t floatOffset = sizeof(Vec4) * 3 / (sizeof(float));
			//�����������ʣ�����ԣ����������в�ֵ  �������꣬�������ڲ������Ӧ����������
			constexpr uint32_t floatNum = sizeof(varyings_t) / sizeof(float);
			float* v0 = (float*)&varyings[0];
			float* v1 = (float*)&varyings[1];
			float* v2 = (float*)&varyings[2];
			float* outFloat = (float*)&out;
			//���ѭ�������������������Խ��в�ֵ��ʹ���� ClipPos ��ͬ��Ȩ�ط�����
			for (int i = floatOffset; i < (int)floatNum; i++)
			{
				outFloat[i] = v0[i] * weights[0] + v1[i] * weights[1] + v2[i] * weights[2];
			}
		}
		//���ڲü�����ƽ����Ұ�ڵĶ���,�õ��ü���Ķ������ݣ���X, Y, Z��������ɵ�����ƽ��
		template<typename varyings_t>
		static int ClipAgainstPlane(varyings_t(&outVaryings)[RGS_MAX_VARYINGS],
			const varyings_t(&inVaryings)[RGS_MAX_VARYINGS],
			const Plane plane,
			const int inVertexNum)
		{
			ASSERT(inVertexNum >= 3);
			//����Ķ��㣬�ü��󶥵����
			int outVertexNum = 0;
			for (int i = 0; i < inVertexNum; i++)
			{
				int prevIndex = (inVertexNum - 1 + i) % inVertexNum;
				int curIndex = i;
				const varyings_t& preVaryings = inVaryings[prevIndex];
				const varyings_t& curVaryings = inVaryings[curIndex];
				const bool prevInside = IsInsidePlane(preVaryings.ClipPos, plane);
				const bool curInside = IsInsidePlane(curVaryings.ClipPos, plane);
				//��ǰ������ǰһ�������жϲ�һ�µ�����£��϶���һ����������ƽ���ⲿ������Ҫͨ����ֵ�ķ�ʽ�õ���������֮��Ĳ�ֵ�㣬��Ϊ��ƽ���ⲿ�Ĳü�����
				//�����ǰ�����ǰһ��������ƽ��Ĳ�ͬ�࣬�������ǵĽ��㣬��ʹ�����Բ�ֵ�������µĶ������ݡ�
				if (curInside != prevInside) {
					float ratio = GetIntersectRatio(preVaryings.ClipPos, curVaryings.ClipPos, plane);
					LerpVaryings(outVaryings[outVertexNum], preVaryings, curVaryings, ratio);
					outVertexNum++;
				}
				//�����жϵ�ǰ�����Ƿ���ƽ���ڣ��������ֱ�Ӽ���
				if (curInside) {
					outVaryings[outVertexNum++] = inVaryings[curIndex];
				}
			}
			ASSERT(outVertexNum <= RGS_MAX_VARYINGS);
			return outVertexNum;
		}
		//�ж���������Ƭ�Ķ����ڲü��ռ�֮��,���õ��ü�֮��Ķ���
		template<typename varyings_t>
		static int Clip(varyings_t(&varyings)[RGS_MAX_VARYINGS]) {
			//������������Ƿ�����׶���ڡ�IsVertexVisible ���ڶ���Ĳü��ռ�λ�ã�ClipPos�����ж϶����Ƿ�ɼ���
			bool v0_Visible = IsVertexVisible(varyings[0].ClipPos);
			bool v1_Visible = IsVertexVisible(varyings[1].ClipPos);
			bool v2_Visible = IsVertexVisible(varyings[2].ClipPos);
			//������׶��֮����ֱ�ӷ���
			if (v0_Visible && v1_Visible && v2_Visible) {
				return 3;
			}
			int vertexNum = 3;
			//��������ü�ƽ�棨 X, Y, Z�����������ظ�ִ�У�ȷ������������ȫ����׶���ڡ���������պ���һ���ռ��ڣ���X, Y, Z���������
			varyings_t varyings_[RGS_MAX_VARYINGS];
			//Plane::POSITIVE_W ��һ���ؼ��Ĳü�ƽ�棬����ȷ�����ж��㶼����һ�����԰�ȫ����͸��ͶӰ��λ�á�
			vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::POSITIVE_W, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_X, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_X, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_Y, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_Y, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_Z, vertexNum);
			if (vertexNum == 0) return 0;
			vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_Z, vertexNum);
			if (vertexNum == 0) return 0;
			//ʹ�� memcpy ����ʱ���� varyings_ �е����ݸ��ƻ�ԭ���� varyings�����¶������ݡ�
			//�����ǰ������������һ�������ڲü��ռ�֮�ڵģ���Ҫͨ��ClipAgainstPlane��������еĲü����㣬���������������ü�֮��Ķ������
			memcpy(varyings, varyings_, sizeof(varyings_));
			return vertexNum;
		}
		//���𽫶�������Ӳü��ռ�ת������һ���豸���꣨NDC��
		template<typename varyings_t>
		static void CaculateNdcPos(varyings_t(&varyings)[RGS_MAX_VARYINGS], const int vertexNum) {
			for (int i = 0; i < vertexNum; i++)
			{
				float w = varyings[i].ClipPos.W;
				varyings[i].NdcPos = varyings[i].ClipPos / w;
				varyings[i].NdcPos.W = 1.0f / w;
			}
		}
		//��NDCת����Ƭ�����꣨��Ļ�ռ����꣩z�����ж�Ϊ���
		template<typename varyings_t>
		static void CaculateFragPos(varyings_t(&varyings)[RGS_MAX_VARYINGS], const int vertexNum, const int width, const int height) {
			for (int i = 0; i < vertexNum; i++)
			{
				float x = ((varyings[i].NdcPos.X + 1.0f) * 0.5f * width);
				float y = ((varyings[i].NdcPos.Y + 1.0f) * 0.5f * height);
				float z = (varyings[i].NdcPos.Z + 1.0f) * 0.5f;
				float w = varyings[i].NdcPos.W;

				varyings[i].FragPos.X = x;
				varyings[i].FragPos.Y = y;
				varyings[i].FragPos.Z = z;
				varyings[i].FragPos.W = w;
			}
		}
		//�õ�ÿ��λ�õ�����ֵ�����ֵ,��Ⱦ����
		template<typename vertex_t, typename uniforms_t, typename varyings_t>
		static void ProcessPixel(Framebuffer& framebuffer,
			const int x,
			const int y,
			const Program<vertex_t, uniforms_t, varyings_t>& program,
			const varyings_t& varyings,
			const uniforms_t& uniforms)
		{
			/* Pixel Shading */
			bool discard = false;
			Vec4 color{ 0.0f, 0.0f, 0.0f, 0.0f };
			//��������������꣬����Ƭ����ɫ���õ���ɫֵ��ͨ����������õ���Ӧ������ֵ
			color = program.FragmentShader(discard, varyings, uniforms);
			if (discard)
			{
				return;
			}
			//����ɫ��ÿ������������0��1֮�䣬ȷ�����������ɫֵ�����
			color.X = Clamp(color.X, 0.0f, 1.0f);
			color.Y = Clamp(color.Y, 0.0f, 1.0f);
			color.Z = Clamp(color.Z, 0.0f, 1.0f);
			color.W = Clamp(color.W, 0.0f, 1.0f);

			/* Blend */
			//����Դ��ɫ��͸������Ŀ����ɫ�������Բ�ֵ����ϣ�����ͨ������ʵ��͸��Ч�������δ���û�ϣ���ֱ�ӽ��������ɫֵд��֡��������
			if (program.EnableBlend)
			{
				Vec3 dstColor = framebuffer.GetColor(x, y);
				Vec3 srcColor = color;
				float alpha = color.W;
				color = { Lerp(dstColor, srcColor, alpha), 1.0f };
				framebuffer.SetColor(x, y, color);
			}
			else
			{
				framebuffer.SetColor(x, y, color);
			}
			//����ǰƬ�ε����ֵд����Ȼ������������ں�������Ȳ��ԣ���ȷ�����صĿɼ��ԡ�����һ���൱����������ǿɼ����ˣ�Ҫ�������ֵ
			if (program.EnableWriteDepth)
			{
				float depth = varyings.FragPos.Z;
				framebuffer.SetDepth(x, y, depth);
			}
		}
		template<typename vertex_t, typename uniforms_t, typename varyings_t>
		static void RasterizeTriangle(Framebuffer& framebuff,
			const Program<vertex_t, uniforms_t, varyings_t>& program,
			const varyings_t(&varyings)[3],
			const uniforms_t& uniforms) {
			//�����޳����������Ⱦ˫�棬����������Ƿ�Ϊ���档��ͨ��ͨ�����������εķ����������߷���ĵ�����жϡ�����Ǳ��棬�򲻽��н�һ������
			// �����ε����������ʵ�֣��ж�˳��ʱ��
			//ͨ����һ���豸����������
			if (!program.EnableDoubleSided)
			{
				bool isBackFacing = false;
				isBackFacing = IsBackFacing(varyings[0].NdcPos, varyings[1].NdcPos, varyings[2].NdcPos);
				if (isBackFacing)
				{
					return;
				}
			}
			int width = framebuff.GetWidth();
			int height = framebuff.GetHeight();
			//�õ���ǰ�����ε���Ļ�ռ����꣬������������
			Vec4 fragCoords[3];//��Ļ�ռ�����
			fragCoords[0] = varyings[0].FragPos;
			fragCoords[1] = varyings[1].FragPos;
			fragCoords[2] = varyings[2].FragPos;
			//�����Χ�����ε���С�߽���Ա�ֻ������ܸ��������ε����ء���������֮������ز���Ⱦ
			BoundingBox bBox = GetBoundingBox(fragCoords, width, height);
			//������Χ���ڵ�ÿ�����أ����ǳ�ʼֻ����������������꣬��������Ƭ�ڲ���������û����������ģ�ֻ��ͨ��������������õ��������꣬��ȥ�õ�����
			for (int y = bBox.MinY; y <= bBox.MaxY; ++y) {
				for (int x = bBox.MinX; x <= bBox.MaxX; ++x) {
					float screenWeights[3];
					float weights[3];
					//ÿ�����صĴ�СΪ1x1�ķ�������ÿ�����ص����ĵ��ǵ�ǰ��������x,y���� +0.5
					Vec2 screenPoint((float)x + 0.5f, (float)y + 0.5);
					//ͨ��������Χ�е��������������������������������꣬ȥ����õ���������
					CalculateWeights(screenWeights, weights, fragCoords, screenPoint);
					//��������֮������ز���Ⱦ��Ȩ��ֵ����и�������������������
					if (!IsInsideTriangle(weights)) {
						continue;
					}
					varyings_t pixVaryings;
					//�õ�ͨ�����������ֵ��� �õ����������ڲ��е��������꣬���������ֵ����������
					LerpVaryings(pixVaryings, varyings, weights, width, height);

					//�õ���������֮�󣬽�����Ȳ��ԣ��жϵ�ǰ�����Ƿ�ɼ�
			//���ͨ����Ȳ��ԣ���ǰ���ص���ɫ�����ֵ������������֡������(����)����Ȼ���������ȣ�����Ӧλ�õ�ֵ�����ͨ��ͬһλ�õ���Ȳ����򸲸����أ�
			// �����ͨ�����ԣ���ǰƬ�ν��������������֡����������Ȼ���������Ӱ�졣
			//�����ַ��������˵�������������
					if (program.EnableDepthTest) {
						float depth = pixVaryings.FragPos.Z;//�õ���ǰ���ص����
						float fDepth = framebuff.GetDepth(x, y);//��û���֡�����λ���ܿ��������ض�Ӧ����ȣ������жϵ�ǰ�����Ƿ�ɼ�
						DepthFuncType depthFunc = program.DepthFunc;
						//�ж��Ƿ�ͨ����Ȳ��ԣ���ǰ�������Ƿ��б����ǣ����û��ͨ������Ⱦ���Ƚ�depth fDepth���ӵ����
						if (!PassDepthTest(depth, fDepth, depthFunc)) {
							continue;
						}
					}
					//ͨ������Ȳ�������µ�ǰ֡��x,y���� ����ֵ�Լ����ֵ
					//�������ڲ������ض�Ӧ����������ͨ����ͨ������������������������꣨Ҳ��Ϊ���Ĳ�ֵ�����������ֵ��������õ���
					//ͨ����������õ�����
					ProcessPixel(framebuff, x, y, program, pixVaryings, uniforms);
				}
			}

		}
public:
		template<typename vertex_t, typename uniforms_t, typename varyings_t>
		static void Draw(Framebuffer& framebuffer, const Program<vertex_t, uniforms_t, varyings_t>& program, const Triangle<vertex_t>& triangle, const uniforms_t& uniforms)
		{
			static_assert(std::is_base_of_v<VertexBase, vertex_t>, "vertex_t ����̳��� RGS::VertexBase");
			static_assert(std::is_base_of_v<VaryingsBase, varyings_t>, "varyings_t ����̳��� RGS::VaryingsBase");
			varyings_t varyings[RGS_MAX_VARYINGS];
			//ͨ��������ɫ������������ת�����õ�clip���꣬��������ȣ�ͶӰ����Ļ�ռ�
			for (int i = 0; i < 3; i++)
			{
				program.VertexShader(varyings[i], triangle[i], uniforms);
			}
			//��������вü����õ��ü��ռ��еĿɼ������������Ͳü�֮��Ķ�������
			int vertexNum = Clip(varyings);
			//�����һ���豸����
			CaculateNdcPos(varyings, vertexNum);
			int fWidth = framebuffer.GetWidth();
			int fHeight = framebuffer.GetHeight();
			//������Ļ�ռ����꣬�������꣬ÿ����������Ļ�϶�Ӧ������λ��
			CaculateFragPos(varyings, vertexNum, (float)fWidth, (float)fHeight);
			//�õ��ü�֮��������ζ�Ӧ�Ķ������ԣ���������ϵ�µ����꣩������������������,�ü�֮��Ķ�������������ĸ�
			//������Ⱦ�ü��ռ䵽��Ļ�ռ��ϵ������Σ����й�դ�����������εĶ���ת��Ϊ��Ļ�ռ��е�����
			//��Ⱦ��Ļ�ռ��������
			for (int i = 0; i < vertexNum - 2; ++i) {
				varyings_t triVaryings[3];//�����ε���������
				triVaryings[0] = varyings[0];
				triVaryings[1] = varyings[i + 1];
				triVaryings[2] = varyings[i + 2];
				RasterizeTriangle(framebuffer, program, triVaryings, uniforms);
			}
		}

	};
}