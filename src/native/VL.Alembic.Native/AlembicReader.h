#pragma once
#include "abcr.h"
#include "abcrScene.h"

using namespace std;

using namespace Alembic;
using namespace Alembic::Abc;

abcrAPI abcrScene* openScene(const char* path);

abcrAPI void closeScene(abcrScene* scene);

abcrAPI float getMinTime(abcrScene* scene);

abcrAPI float getMaxTime(abcrScene* scene);

abcrAPI int getGeomCount(abcrScene* scene);

abcrAPI const char* getName(abcrScene* scene, int index);

abcrAPI abcrGeom* getGeom(abcrScene* scene, const char* name);

abcrAPI void updateTime(abcrScene* scene, float time);

abcrAPI AlembicType::Type getType(abcrGeom* geom);

abcrAPI Matrix4x4 getTransform(abcrGeom* geom);

abcrAPI void getPointSample(Points* points, float* o);

abcrAPI int getPointCount(Points* points);

abcrAPI VertexLayout getPolyMeshLayout(PolyMesh* mesh);

abcrAPI float* getPolyMeshSample(PolyMesh* mesh, int* size);

abcrAPI void getCameraSample(Camera* camera, Matrix4x4* v, CameraParam* p);