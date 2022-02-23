#pragma once

#include <Alembic\Abc\All.h>
#include <Alembic\AbcGeom\IPolyMesh.h>
#include <Alembic\AbcGeom\IPoints.h>
#include <Alembic\AbcGeom\ICurves.h>
#include <Alembic\AbcGeom\IXForm.h>
#include <Alembic\AbcGeom\ICamera.h>

#include "abcrUtils.h"
#include "abcrlayout.h"
#include "abcrTypes.h"

using namespace std;

using namespace Alembic;
using namespace Alembic::Abc;

class XForm;
class Points;
class Curves;
class PolyMesh;
class Camera;

class abcrScene;

struct abcrPtr;

namespace AlembicType
{
    enum Type
    {
        POINTS = 0,
        CURVES,
        POLYMESH,
        CAMERA,
        XFORM,
        UNKNOWN
    };
}

template <typename T>
inline AlembicType::Type type2enum() { return AlembicType::UNKNOWN; }

template <>
inline AlembicType::Type type2enum<XForm>() { return AlembicType::XFORM; }

template <>
inline AlembicType::Type type2enum<Points>() { return AlembicType::POINTS; }

template <>
inline AlembicType::Type type2enum<Curves>() { return AlembicType::CURVES; }

template <>
inline AlembicType::Type type2enum<PolyMesh>() { return AlembicType::POLYMESH; }

template <>
inline AlembicType::Type type2enum<Camera>() { return AlembicType::CAMERA; }

class abcrGeom
{
    friend class abcrScene;

public:

    abcrGeom();
    abcrGeom(IObject obj);
    virtual ~abcrGeom();

    virtual bool valid() const { return m_obj; };

    size_t getIndex() const { return index; };

    string getName() const { return m_obj.getName(); };

    string getFullName() const { return m_obj.getFullName(); };

    IObject getIObject() const { return m_obj; };

    Matrix4x4 getTransform() const { return toVVVV(transform); };

    virtual const char* getTypeName() const { return ""; };

    AlembicType::Type getType() const { return type; };

    inline bool isTypeOf(AlembicType::Type t) const { return type == t; };

    template<typename T>
    inline bool isTypeOf() const { return type == type2enum<T>(); };

    void setUpNodeRecursive(IObject obj);
    static void setUpDocRecursive(shared_ptr<abcrGeom>& obj, map<string, abcrPtr>& nameMap, map<string, abcrPtr>& fullnameMap);

protected:

    AlembicType::Type type;

    size_t index;

    chrono_t m_minTime;
    chrono_t m_maxTime;

    Imath::M44f transform;

    bool constant;

    IObject m_obj;
    vector<shared_ptr<abcrGeom>> m_children;

    virtual void updateTimeSample(chrono_t time, Imath::M44f& transform);
    virtual void set(chrono_t time, Imath::M44f& transform) {};

    template<typename T>
    void setMinMaxTime(T& obj);

    bool isUpdate = true;
    index_t lastSampleIndex = 0;

    TimeSamplingPtr m_samplingPtr;
};

class XForm : public abcrGeom
{
public:

    Imath::M44f mat;

    XForm(AbcGeom::IXform xform);
    ~XForm()
    {
        if (m_xform) m_xform.reset();
    }

    const char* getTypeNmae() const { return "XForm"; }

    void set(chrono_t time, Imath::M44f& transform) override;

private:

    AbcGeom::IXform m_xform;

};

class Points : public abcrGeom
{
public:

    Points(AbcGeom::IPoints points);
    ~Points() 
    {
        if (m_points) m_points.reset();
    }

    const char* getTypeNmae() const { return "Points"; }
    int getPointCount() const { return m_count; }
    void set(chrono_t time, Imath::M44f& transform) override;

    bool get(float* o);

private:

    AbcGeom::IPoints m_points;

    P3fArraySamplePtr m_positions;
    int m_count;
};

class Curves : public abcrGeom
{
public:

    int* index = nullptr;
    float* curves = nullptr;

    Curves(AbcGeom::ICurves curves);
    ~Curves()
    {
        if (m_curves) m_curves.reset();
        if(this->index)  delete this->index;
        if(this->curves) delete this->curves;
    }

    const char* getTypeNmae() const { return "Curves"; }
    int getCurveCount() const { return m_count; }
    inline void getIndexSpread(int* o) const 
    {
        memcpy(o, this->index, m_indexCount);
    }

    void set(chrono_t time, Imath::M44f& transform) override;

    inline bool get(float* o)
    {
        memcpy(o, this->curves, this->getCurveCount() * sizeof(V3f));
        return true;
    }

private:

    AbcGeom::ICurves m_curves;
    int m_count;
    int m_indexCount;
};

class PolyMesh : public abcrGeom
{
public:

    float* geom = nullptr;

    PolyMesh(AbcGeom::IPolyMesh pmesh);
    ~PolyMesh()
    {
        if (m_polymesh) m_polymesh.reset();
        if(this->geom) delete this->geom;
    }

    const char* getTypeNmae() const { return "PolyMesh"; }
    int getVertexCount() const { return m_vertexCount; }
    size_t getVertexSize() const { return vertexSize; }
    VertexLayout getVertexLayout() const { return layout; }
    void set(chrono_t time, Imath::M44f& transform) override;

    void resize(size_t size);

    float* get(int* size);


private:

    bool hasNormal;
    bool hasUV;
    bool hasRGB;
    bool hasRGBA;

    AbcGeom::IPolyMesh m_polymesh;
    AbcGeom::IC3fGeomParam m_rgb;
    AbcGeom::IC4fGeomParam m_rgba;

    AbcGeom::IPolyMeshSchema::Sample m_mesh_samp;
    AbcGeom::IV2fGeomParam::Sample m_uv_samp;
    AbcGeom::IN3fGeomParam::Sample m_norm_samp;

    AbcGeom::IC3fGeomParam::Sample m_rgb_samp;
    AbcGeom::IC4fGeomParam::Sample m_rgba_samp;

    VertexLayout layout;
        
    size_t vertexSize;
    int m_vertexCount;
    int m_capacity;
};

class Camera : public abcrGeom
{
public:
    Matrix4x4 View;
    CameraParam Proj;

    Camera(AbcGeom::ICamera camera);
    ~Camera()
    {
        if (m_camera) m_camera.reset();
    }

    const char* getTypeNmae() const { return "Camera"; }

    void set(chrono_t time, Imath::M44f& transform) override;

    inline bool get(Matrix4x4* ov, CameraParam* op)
    {
        *ov = this->View;
        *op = this->Proj;
        return true;
    }

private:

    AbcGeom::ICamera m_camera;

};

struct abcrPtr
{
    abcrGeom* m_ptr;

    abcrPtr() { m_ptr = nullptr; }
    abcrPtr(abcrGeom* ptr) : m_ptr(ptr) {}
};