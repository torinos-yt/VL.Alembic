#include "AlembicReader.h"
#include "abcrUtils.h"

abcrAPI abcrScene* openScene(const char* path)
{
	auto* scene = new abcrScene();
	return scene->open(path) ? scene : nullptr;
}

abcrAPI void closeScene(abcrScene* scene)
{
	if(scene) delete scene;
}

abcrAPI float getMinTime(abcrScene* scene)
{
	return scene ? scene->getMinTime() : -1;
}

abcrAPI float getMaxTime(abcrScene* scene)
{
	return scene ? scene->getMaxTime() : -1;
}

abcrAPI int getGeomCount(abcrScene* scene)
{
	return scene ? scene->getGeomCount() : 0;
}

abcrAPI const char* getName(abcrScene* scene, int index)
{
	return scene ? scene->getFullName(index) : "";
}

abcrAPI abcrGeom* getGeom(abcrScene* scene, const char* name)
{
	return scene ? scene->getGeom(name) : nullptr;
}

abcrAPI void updateTime(abcrScene* scene, float time)
{
	if (scene) scene->updateSample(time);
}

abcrAPI AlembicType::Type getType(abcrGeom* geom)
{
	return geom ? geom->getType() : AlembicType::UNKNOWN;
}

abcrAPI Matrix4x4 getTransform(abcrGeom* geom)
{
	return geom ? geom->getTransform() : Matrix4x4();
}

abcrAPI void getPointSample(Points* points, float* o)
{
	if (points) points->get(o);
}

abcrAPI int getPointCount(Points* points)
{
	return points ? points->getPointCount() : -1;
}

abcrAPI VertexLayout getPolyMeshLayout(PolyMesh* mesh)
{
	return mesh ? mesh->getVertexLayout() : VertexLayout::Unknown;
}

abcrAPI float* getPolyMeshSample(PolyMesh* mesh, int* size)
{
	return mesh ? mesh->get(size) : nullptr;
}

abcrAPI void getCameraSample(Camera* camera, Matrix4x4* v, CameraParam* p)
{
	if (camera) camera->get(v, p);
}