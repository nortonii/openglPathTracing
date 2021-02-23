#version 450 core
out vec4 FragColor;
in vec3 direction;
in vec3 origin;
in vec2 point;
in vec2 wnh;
uniform samplerBuffer buf;
uniform samplerBuffer tria;
uniform samplerBuffer rand;
int stackBuffer[gl_MaxVertexUniformVectors];
vec3 colorBuffer[gl_MaxVertexUniformVectors];
struct Ray {	
    vec3 ori;	
    vec3 dir; 
	vec3 invDir;
	vec3 dirIsNeg;
};
struct BVH {	
    vec3 pMin;	
    vec3 pMax; 
};
struct Inter{
	vec3 normal;
	vec3 position;
	float distanceI;
	bool isIntersect;
	vec3 color;
	bool isLight;
	float pdf;
};
struct Triangle{
	vec3 a;
	vec3 b;
	vec3 c;
	vec3 aNormal;
	vec3 bNormal;
	vec3 cNormal;
	vec3 color;
	bool isLight;
};
float random(int iterationTime,int index){
	return texelFetch(rand,int((point.y*wnh[0]+point.x)*28+7*iterationTime+index)).r;
}
bool intersectP(Ray ray,BVH bvh){
    vec3 counTempMin=(bvh.pMin-ray.ori)*ray.invDir;
    vec3 counTempMax=(bvh.pMax-ray.ori)*ray.invDir;
    vec3 beginTime=  vec3(ray.dirIsNeg.x>=0?counTempMin.x:counTempMax.x,ray.dirIsNeg.y>=0?counTempMin.y:counTempMax.y,ray.dirIsNeg.z>=0?counTempMin.z:counTempMax.z);
    vec3 endTime  =  vec3(ray.dirIsNeg.x>=0?counTempMax.x:counTempMin.x,ray.dirIsNeg.y>=0?counTempMax.y:counTempMin.y,ray.dirIsNeg.z>=0?counTempMax.z:counTempMin.z);
    float maxBeginTime=max(beginTime.x,max(beginTime.y,beginTime.z));
    float minEndTime  =min(endTime.x,min(endTime.y,endTime.z));
    if(minEndTime>=maxBeginTime && minEndTime>=0){
        return true;
    }
    return false;
}
Inter intersectT(Ray ray,Triangle tria){
    Inter inter;
	inter.color=vec3(0);
	inter.isIntersect=false;
	inter.distanceI=-7;
	vec3 K,L;
	vec3 e1e2;
	float u, v, t_tmp = 0;
	float det;
	float det_inv;
	vec3 tvec;
	vec3 qvec;
	vec3 pvec;
	vec3 normal = cross(tria.c-tria.a, tria.b-tria.a);
    if (dot(ray.dir, normal) >= 0)
        return inter;
    pvec = cross(-ray.dir, tria.c-tria.a);
    det = dot(tria.b-tria.a, pvec);
    if (abs(dot(normalize(tria.b-tria.a), normalize(pvec))) < 0.0001)
        return inter;
    det_inv = 1. / det;
    tvec = ray.ori - tria.a;
    u = dot(tvec, pvec) * det_inv;
    if (u < 0 || u > 1)
        return inter;
    qvec = cross(tvec, tria.b-tria.a);
    v = dot(-ray.dir, qvec) * det_inv;
    if (v < 0 || u + v > 1)
        return inter;
    t_tmp = -dot(tria.c-tria.a, qvec) * det_inv;
    if(t_tmp<0)
        return inter;
    inter.isIntersect=true;
    inter.position=tria.a*(1-u-v)+tria.b*u+tria.c*v;
    inter.normal=normalize(normal);//tria.aNormal*(1-u-v)+tria.bNormal*u+tria.cNormal*v;
    inter.distanceI=length(ray.ori-inter.position)/2;
	inter.color=tria.color;
	inter.isLight=tria.isLight;
    return inter;
}
vec3 sampleRay(vec3 n,vec3 dir,int iterationTime){
	float x_1 = random(iterationTime,3), 
		  x_2 = random(iterationTime,4);
	float z = abs(1.0f - 2.0f * x_1);
    float r = sqrt(1.0f - z * z), phi = 2 * 3.1415926535897932* x_2;
	vec3 a=vec3(r*cos(phi), r*sin(phi), z);
	vec3 A, B, C;
	float invLen = 1.0f / sqrt(n.x * n.x + n.z * n.z);
    if (abs(n.x) >= abs(n.y)){
        C = vec3(n.z * invLen, 0.0f, -n.x *invLen);
    }
    else {
        invLen = 1.0f / sqrt(n.y * n.y + n.z * n.z);
        C = vec3(0.0f, n.z * invLen, -n.y *invLen);
    }
    B = cross(C, n);
    return normalize(a.x * B + a.y * C + a.z * n);
}
float triangleArea(vec3 a,vec3 b,vec3 c){
	return length(cross(c-a, b-a))*0.5;
}
Inter sampleLightArea(Inter pos,Triangle trian,int iterationTime){
	Inter inter;
    float x = sqrt(random(iterationTime,1)),
	y= random(iterationTime,2);
	vec2 result;
	inter.color=trian.color;
    inter.position = trian.a * (1.0f - x) + trian.b * (x * (1.0f - y)) + trian.c * (x * y);
    inter.normal = normalize(cross(trian.c-trian.a,trian.b-trian.a));
	inter.isLight=true;
    inter.pdf = 1.0f / abs(triangleArea(trian.a,trian.b,trian.c));
	return inter;
}
Inter sampleLight(Inter pos,int iterationTime){
    float emit_area_sum = 0;
	float p;
	float pdf;
	Triangle trian;
    Inter inter;
	inter.color=vec3(0);
	inter.isIntersect=false;
	inter.distanceI=-1;
    for (int idx = 0; idx < 64; ++idx) {
		trian=Triangle(texelFetch(tria,idx*8  ).rgb,texelFetch(tria,idx*8+1).rgb,
				 texelFetch(tria,idx*8+2).rgb,texelFetch(tria,idx*8+3).rgb,
				 texelFetch(tria,idx*8+4).rgb,texelFetch(tria,idx*8+5).rgb,
				 texelFetch(tria,idx*8+6).rgb,texelFetch(tria,idx*8+7).r>0);
        if (trian.isLight){
            emit_area_sum += triangleArea(trian.a,trian.b,trian.c);
        }
    }
	pdf=1.0f/emit_area_sum;
    p = random(iterationTime,5) * emit_area_sum;
    emit_area_sum = 0;
    for (int idx = 0; idx < 64; ++idx) {
		trian=Triangle(texelFetch(tria,idx*8  ).rgb,texelFetch(tria,idx*8+1).rgb,
				 texelFetch(tria,idx*8+2).rgb,texelFetch(tria,idx*8+3).rgb,
				 texelFetch(tria,idx*8+4).rgb,texelFetch(tria,idx*8+5).rgb,
				 texelFetch(tria,idx*8+6).rgb,texelFetch(tria,idx*8+7).r>0);
        if (trian.isLight){
            emit_area_sum += triangleArea(trian.a,trian.b,trian.c);
            if (p <= emit_area_sum){
				inter=sampleLightArea(pos,trian,iterationTime);
				inter.distanceI=length(inter.position-pos.position);
				inter.pdf=pdf;
                break;
            }
        }
    }
	return inter;
}
Inter bvhMethod(Ray ray){
	int stack=0;
	int pnode=0;
	stackBuffer[stack]=0;
	float minDistance=-1;
	Inter inter=Inter(vec3(0),vec3(0),-1,false,vec3(0),false,0);
	while(stack>=0){
		pnode=stackBuffer[stack];
		stack-=1;
		BVH root=BVH(texelFetch(buf,pnode).rgb,texelFetch(buf,pnode+1).rgb);
		bool isIntersect=intersectP(ray,root);
		int idx=int(texelFetch(buf,pnode+2).r);
		if( isIntersect && (idx<0)){
			stackBuffer[stack+1]=2*pnode+3;
			stackBuffer[stack+2]=2*pnode+6;
			stack+=2;
		}
		else if(isIntersect || idx>=0){
			Triangle trian=
			Triangle(texelFetch(tria,idx*8  ).rgb,texelFetch(tria,idx*8+1).rgb,
					 texelFetch(tria,idx*8+2).rgb,texelFetch(tria,idx*8+3).rgb,
					 texelFetch(tria,idx*8+4).rgb,texelFetch(tria,idx*8+5).rgb,
					 texelFetch(tria,idx*8+6).rgb,texelFetch(tria,idx*8+7).r>0);
			Inter judge=intersectT(ray,trian);
			if(judge.isIntersect&&(minDistance==-1||judge.distanceI<minDistance)){
				minDistance=judge.distanceI;
				inter=judge;
			}
		}
	}
	return inter;
}
Inter normMethod(Ray ray){
	Inter inter=Inter(vec3(0),vec3(0),-8,false,vec3(0),false,0);
	float minDistance=-9;
	Inter judge;
	for(int idx=0;idx<64;idx++){
		Triangle tria=Triangle(texelFetch(tria,idx*8  ).rgb,texelFetch(tria,idx*8+1).rgb,
				 texelFetch(tria,idx*8+2).rgb,texelFetch(tria,idx*8+3).rgb,
				 texelFetch(tria,idx*8+4).rgb,texelFetch(tria,idx*8+5).rgb,
				 texelFetch(tria,idx*8+6).rgb,texelFetch(tria,idx*8+7).r>0);
		judge=intersectT(ray,tria);
		if(judge.isIntersect&&(judge.distanceI<=minDistance||minDistance<0)){
			inter=judge;
			minDistance=judge.distanceI;
		}
	}
	return inter;
}
vec3 eval(vec3 wo, vec3 N,vec3 Kd){
	vec3 diffuse=vec3(0);
	float cosalpha = dot(N, wo);
	if (cosalpha >= 0.0f) {
	     diffuse= 1/3.1415926535897932*Kd;
	}
	return diffuse;
}
vec3 lightResult(Ray ray,Inter p,int iterationTime){
	Inter lightInter;
	vec3 ws;
	vec3 emit=8.0f*vec3(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * vec3(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *vec3(0.737f+0.642f,0.737f+0.159f,0.737f);
	vec3 lightColor=vec3(0,0,0);
	Inter test;
	Triangle tria1;
	Ray raylight;
	lightInter=sampleLight(p,iterationTime);
	ws=normalize(lightInter.position-p.position);
	raylight=Ray(p.position,ws,1/ws,sign(ws));
	test=normMethod(raylight);
	if(test.distanceI>=lightInter.distanceI-0.00001||test.isLight){
		lightColor=emit*eval(ws,p.normal,p.color)
		*dot(normalize(-ws),normalize(lightInter.normal)*(-1)
		*dot(normalize(-ws),normalize(p.normal)))	
		/lightInter.pdf/pow(lightInter.distanceI,2);
	}
	return lightColor;
}
Inter shade(Ray ray){
	Inter inter;
	inter=normMethod(ray);
	return inter;
}
float pdf_m(vec3 wi,  vec3 wo, vec3 N){
	// uniform sample probability 1 / (2 * PI)
	if (dot(wo, N) >= 0.0f)
		return 0.5f / 3.141592653589732;
	else
		return 0.0f;
}
void main(){   
	Ray ray;
	Ray rayTmp;
	Inter inter;
	Inter interTmp;
	vec3 dirTemp;
	int iterationTime=0;
	int colorIndex=0;
	vec3 lightColor=vec3(0);
	vec3 nolightColor=vec3(0);
	vec3 color=vec3(0.0f);
	vec3 allColor=vec3(0.0f);
	bool endRandom=false;
	float russiaRate=0.8;
	rayTmp=Ray(origin,direction,1/direction,sign(direction));
	inter=shade(rayTmp);
	iterationTime=-1;
	if(inter.isIntersect){
		do{
			iterationTime+=1;
			ray=rayTmp;
			if(inter.isLight){
				colorBuffer[colorIndex+1] = 8.0f * vec3(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * vec3(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *vec3(0.737f+0.642f,0.737f+0.159f,0.737f);
				colorBuffer[colorIndex] = vec3(0.0f);
				colorIndex+=2;
				break;
			}
			lightColor=lightResult(ray,inter,iterationTime);
			colorBuffer[colorIndex+1]=lightColor;
			colorBuffer[colorIndex]=vec3(0.0f);
			colorIndex+=2;
			if(random(iterationTime,0)>russiaRate){
				break;
			}
			dirTemp=sampleRay(inter.normal,ray.dir,iterationTime);
			rayTmp=Ray(inter.position,dirTemp,1/dirTemp,sign(dirTemp));
			interTmp=shade(rayTmp);
			nolightColor=vec3(0);
			if(!interTmp.isLight){
				nolightColor=eval(rayTmp.dir,inter.normal,inter.color)
				*dot(normalize(rayTmp.dir),normalize(inter.normal))
				/pdf_m(ray.dir,rayTmp.dir,inter.normal)/russiaRate;
			}
			colorBuffer[colorIndex-2]=nolightColor;
			inter=interTmp;
		}while(iterationTime<30&&inter.isIntersect);
		for(int i=colorIndex;i>=2;i-=2){
			color=colorBuffer[i-1]+colorBuffer[i-2]*color;
		}
	}
	if(true){
		FragColor=vec4(color,1.0f);
	}else{
		FragColor=vec4(0.0f,0.0f,0.0f,1.0f);
	}
}