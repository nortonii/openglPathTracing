#pragma once
#define M_PI 3.141592653589793f
using namespace cv;
class Ray
{
	unsigned int VAO;
	unsigned int VBO,VBO1;
	GLuint TBO,buf ;
	float* vertices;
	int width;
	int height;
	int bvhSize;
	int triSize;
	Camera camera;
	Shader shader;
	TBOTool texture;
	TBOTool texture1;
	TBOTool texture2;
	float *randFloat;
public:
	inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }
	Ray(unsigned int width,unsigned int height,
		Camera scene,Shader shader,float* bvh,float* tri,int bvhSize,int triSize)
		:height(height),width(width),shader(shader), bvhSize(bvhSize), triSize(triSize)
	{
		camera = scene;
		vertices = new float[width * height * 5];
		randFloat = new float[width * height * 4 * 7];
		float scale = tan(deg2rad(scene.fov * 0.5));
		float imageAspectRatio = width / (float)height;
		camera.Position=glm::vec3(278, 273, -800);
		int m = 0;
		for (uint32_t j = 0; j < height; ++j) {
			for (uint32_t i = 0; i < width; ++i) {
				// generate primary ray direction
				float x = ( 2 * (i + 0.5) / (float)width -1) *
					imageAspectRatio * scale;
				float y = ( 2 * (j + 0.5) / (float)height -1) * scale;
				glm::vec3 dir = normalize(glm::vec3(-x, y, 1));
				for (int t = 0; t < 3; t++){
					vertices[m + t] = dir[t];
				}
				vertices[m + 3] = (float)i;
				vertices[m + 4] = (float)j;
				m += 5;
			}
		}	
		std::cout << bvh[6] << std::endl;
		setUpTray(bvh, tri);
	}
	void Random(){
		srand(time(0));  //设置时间种子
		for (int i = 0; i < width * height * 4 * 7; i++) {
			randFloat[i] = (float)rand()/ RAND_MAX;//生成区间r~l的随机数 
		}
		texture2 = TBOTool(randFloat, height*width * 4 * 7, GL_R32F);
	}
	void setUpTray(float* bvh, float* tri){
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		texture= TBOTool(bvh, bvhSize, GL_RGB32F);
		texture1= TBOTool(tri,triSize, GL_RGB32F);
		Random();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, height*width*sizeof(float)*5, vertices, GL_STATIC_DRAW);
		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	void Draw(Shader &shader){
		// draw mesh
		texture.bindTexture(0);
		shader.setInt("buf", 0); 
		texture1.bindTexture(1);
		shader.setInt("tria", 1); 
		texture2.bindTexture(2);
		Random();
		shader.setInt("rand", 2); 
		glBindVertexArray(VAO);
		glUniform2f(glGetUniformLocation(shader.ID, "screen"), (float)width, (float)height); // 手动设置;
		glUniform3f(glGetUniformLocation(shader.ID, "eye"), camera.Position.x, camera.Position.y, camera.Position.z);
		glDrawArrays(GL_POINTS, 0, height*width);
		glBindVertexArray(0);
	}
};

