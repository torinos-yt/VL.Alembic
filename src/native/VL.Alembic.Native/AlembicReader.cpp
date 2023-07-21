#include "AlembicReader.h"

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

abcrAPI void setInterpolate(abcrScene* scene, bool interpolate)
{
	if (scene) scene->setInterpolate(interpolate);
}

abcrAPI AlembicType::Type getType(abcrGeom* geom)
{
	return geom ? geom->getType() : AlembicType::UNKNOWN;
}

abcrAPI Matrix4x4 getTransform(abcrGeom* geom)
{
	return geom ? geom->getTransform() : Matrix4x4();
}

abcrAPI float getGeomMinTime(abcrGeom* geom)
{
	return geom ? geom->getMinTime() : -1;
}

abcrAPI float getGeomMaxTime(abcrGeom* geom)
{
	return geom ? geom->getMaxTime() : -1;
}

abcrAPI void getPointSample(Points* points, float* o)
{
	if (points) points->get(o);
}

abcrAPI int getPointCount(Points* points)
{
	return points ? points->getPointCount() : -1;
}

abcrAPI void getCurveSample(Curves* curves, DataPointer* curvePtr, DataPointer* idxPtr)
{
	if (curves) curves->get(curvePtr, idxPtr);
}

abcrAPI VertexLayout getPolyMeshLayout(PolyMesh* mesh)
{
	return mesh ? mesh->getVertexLayout() : VertexLayout::Unknown;
}

abcrAPI int getPolyMeshTopologyVariance(PolyMesh* mesh)
{
	return mesh ? mesh->getTopologyVariance() : -1;
}

abcrAPI float* getPolyMeshSample(PolyMesh* mesh, int* size)
{
	return mesh ? mesh->get(size) : nullptr;
}

abcrAPI BoundingBox getPolyMeshBoundingBox(PolyMesh* mesh)
{
	return mesh ? mesh->getBounds() : BoundingBox();
}

abcrAPI int getPolyMeshMaxVertexCount(PolyMesh* mesh)
{
	return mesh ? mesh->getMaxVertexCount() : -1;
}

abcrAPI BoundingBox getPolyMeshMaxSizeBoudingBox(PolyMesh* mesh)
{
	return mesh ? mesh->getMaxSizeBoudingBox() : BoundingBox();
}

abcrAPI void getCameraSample(Camera* camera, Matrix4x4* v, CameraParam* p)
{
	if (camera) camera->get(v, p);
}

abcrAPI void RegisterDebugFunc(DebugFunction fp)
{
	RegisterDebugFunction(fp);
}