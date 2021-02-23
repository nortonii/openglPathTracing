#pragma once

class BVH{
public:
	glm::vec3 minVertex;
	glm::vec3 maxVertex;
	bool isLeaf;
	triangle t;
	BVH* left = NULL;
	BVH* right = NULL;
	BVH(glm::vec3 minv, glm::vec3 maxv, bool isroot = false, triangle trian = triangle())
	{
		minVertex = minv; maxVertex = maxv; isLeaf = isroot; t = trian; right = NULL; left = NULL;
	}
	int getLeftChild(int father)
	{
		return father * 2 + 3;
	}
	int getRightChild(int father)
	{
		return father * 2 + 6;
	}
};
void vec2float(float *offset,glm::vec3 content) {
	*offset = content[0];
	*(offset + 1) = content[1];
	*(offset + 2) = content[2];
}
bool cmpTriangleX(triangle t1, triangle t2) {
	return (t1.a.Position.x + t1.b.Position.x + t1.c.Position.x) / 3.0f > (t2.a.Position.x + t2.b.Position.x + t2.c.Position.x) / 3.0f;
}
bool cmpTriangleY(triangle t1, triangle t2) {
	return (t1.a.Position.y + t1.b.Position.y + t1.c.Position.y) / 3.0f > (t2.a.Position.y + t2.b.Position.y + t2.c.Position.y) / 3.0f;
}
bool cmpTriangleZ(triangle t1, triangle t2) {
	return (t1.a.Position.z + t1.b.Position.z + t1.c.Position.z) / 3.0f > (t2.a.Position.z + t2.b.Position.z + t2.c.Position.z) / 3.0f;
}
BVH* recursionBVH(vector<triangle> &triangleList, int begin, int end, xyz axis){
	if (begin+1==end)
	{
		return new BVH(triangleList[begin].minTriangle(), triangleList[begin].maxTriangle(),true,triangleList[begin]);
	}
	switch (axis)
	{
	case xAxis:
		sort(triangleList.begin() + begin, triangleList.begin() + end, cmpTriangleX);
		break;
	case yAxis:
		sort(triangleList.begin() + begin, triangleList.begin() + end, cmpTriangleY);
		break;
	case zAxis:
		sort(triangleList.begin() + begin, triangleList.begin() + end, cmpTriangleZ);
		break;
	default:
		break;
	}
	glm::vec3 minV = triangleList[begin].minTriangle(), maxV = triangleList[begin].maxTriangle();
	for (int i = begin; i < end; i++)
	{
		minV = triangleList[i].minVec3(minV);
		maxV = triangleList[i].maxVec3(maxV);
	}
	BVH* bvh = new BVH(minV,maxV);
	switch (axis)
	{
	case xAxis:
		bvh->left = recursionBVH(triangleList, begin, begin + (end - begin) / 2, yAxis);
		bvh->right = recursionBVH(triangleList,begin + (end - begin) / 2, end , yAxis);
		break;
	case yAxis:
		bvh->left = recursionBVH(triangleList, begin, begin + (end - begin) / 2, zAxis);
		bvh->right = recursionBVH(triangleList,begin + (end - begin) / 2, end , zAxis);
		break;
	case zAxis:
		bvh->left = recursionBVH(triangleList, begin, begin + (end - begin) / 2, xAxis);
		bvh->right = recursionBVH(triangleList, begin + (end - begin) / 2, end, xAxis);
		break;
	default:
		break;
	}
	return bvh;
}
void recursionWriteTexture(float *textureMat, float *triangleMat,BVH *root, int index,int &idx){
	if (root == NULL){
		return;
	}
	textureMat[index * 3] = root->minVertex.x ;
	textureMat[index * 3 + 1] = root->minVertex.y;
	textureMat[index * 3 + 2] = root->minVertex.z;
	textureMat[(index + 1) * 3] = root->maxVertex.x;
	textureMat[(index + 1) * 3 + 1] = root->maxVertex.y;
	textureMat[(index + 1) * 3 + 2] = root->maxVertex.z;
	textureMat[(index + 2) * 3] = root->isLeaf ? (float)idx : -1.0f;
	textureMat[(index + 2) * 3 + 1] = 0.0f;
	textureMat[(index + 2) * 3 + 2] = 0.0f;
	if (root->isLeaf){
		vec2float(triangleMat + idx * 8 * 3, root->t.a.Position);
		vec2float(triangleMat + (idx * 8 + 1) * 3, root->t.b.Position);
		vec2float(triangleMat + (idx * 8 + 2) * 3, root->t.c.Position);
		vec2float(triangleMat + (idx * 8 + 3) * 3, root->t.a.Normal);
		vec2float(triangleMat + (idx * 8 + 4) * 3, root->t.b.Normal);
		vec2float(triangleMat + (idx * 8 + 5) * 3, root->t.c.Normal);
		vec2float(triangleMat + (idx * 8 + 6) * 3, root->t.color);
		vec2float(triangleMat + (idx * 8 + 7) * 3, glm::vec3(root->t.isLight ? 1.0f : -1.0f,0.0f,0.0f));
		idx++;
	}
	recursionWriteTexture(textureMat, triangleMat,root->left, root->getLeftChild(index), idx);
	recursionWriteTexture(textureMat, triangleMat,root->right, root->getRightChild(index), idx);
}
int numberOfLeafs(BVH* root) {
	int nodes = 0;
	if (root == NULL)
		return 0;
	else if (root->left == NULL && root->right == NULL)
		return 1;
	else
		nodes = numberOfLeafs(root->left) + numberOfLeafs(root->right);
	return nodes;
}
int nubmerOfNodes(BVH* root) {
	int nodes = 0;
	if (root == NULL)
		return 1;
	else {
		nodes = 1 + nubmerOfNodes(root->left) + nubmerOfNodes(root->right);
	}
	return nodes;
}
int getTreeDeepth(BVH *pnode){
	if (pnode == NULL){
		return 0;
	}
	int left_h = getTreeDeepth(pnode->left);
	int right_h = getTreeDeepth(pnode->right);

	return (left_h > right_h) ? (1 + left_h) : (1 + right_h);
}
void createTexture(BVH* root,float *&textureMat, float *&triangleMat,int &bvhSize,int &triSize)
{
	bvhSize = (pow(2, getTreeDeepth(root)) - 1) * 3 * 3;
	triSize = numberOfLeafs(root) * 8 * 3 ;
	textureMat = (float*)malloc(bvhSize * sizeof(float));
	memset(textureMat, 0, bvhSize * sizeof(float));
	triangleMat = (float*)malloc(triSize * sizeof(float));
	memset(triangleMat, 0, triSize * sizeof(float));
	int idx = 0;
	recursionWriteTexture(textureMat,triangleMat,root,0,idx);
}
