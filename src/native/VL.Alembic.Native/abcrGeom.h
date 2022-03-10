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

    virtual bool valid() const { return _obj; };

    size_t getIndex() const { return _index; };

    string getName() const { return _obj.getName(); };

    string getFullName() const { return _obj.getFullName(); };

    IObject getIObject() const { return _obj; };

    Matrix4x4 getTransform() const { return toVVVV(_transform); };

    virtual const char* getTypeName() const { return ""; };

    AlembicType::Type getType() const { return _type; };

    inline bool isTypeOf(AlembicType::Type t) const { return _type == t; };

    template<typename T>
    inline bool isTypeOf() const { return _type == type2enum<T>(); };

    void setUpNodeRecursive(IObject obj);
    static void setUpDocRecursive(shared_ptr<abcrGeom>& obj, map<string, shared_ptr<abcrGeom>>& nameMap, map<string, shared_ptr<abcrGeom>>& fullnameMap);

protected:

    AlembicType::Type _type;

    size_t _index;

    chrono_t _minTime;
    chrono_t _maxTime;

    Imath::M44f _transform;

    bool _constant;

    IObject _obj;
    vector<shared_ptr<abcrGeom>> _children;

    virtual void updateTimeSample(chrono_t time, Imath::M44f& transform);
    virtual void set(chrono_t time, Imath::M44f& transform) {};

    template<typename T>
    void setMinMaxTime(T& obj);

    bool _isUpdate = true;
    index_t _lastSampleIndex = 0;

    TimeSamplingPtr _samplingPtr;
};

class XForm : public abcrGeom
{
public:

    Imath::M44f _matrix;

    XForm(AbcGeom::IXform xform);
    ~XForm()
    {
        if (_xform) _xform.reset();
    }

    const char* getTypeNmae() const { return "XForm"; }

    void set(chrono_t time, Imath::M44f& transform) override;

private:

    AbcGeom::IXform _xform;

};

class Points : public abcrGeom
{
public:

    Points(AbcGeom::IPoints points);
    ~Points() 
    {
        if (_points) _points.reset();
    }

    const char* getTypeNmae() const { return "Points"; }
    int getPointCount() const { return _pointCount; }
    void set(chrono_t time, Imath::M44f& transform) override;

    bool get(float* o);

private:

    AbcGeom::IPoints _points;

    P3fArraySamplePtr _positions;
    int _pointCount;
};

class Curves : public abcrGeom
{
public:

    uint32_t* _index = nullptr;

    Curves(AbcGeom::ICurves _curves);
    ~Curves()
    {
        if (_curves) _curves.reset();
        if(_index != nullptr)  delete[] _index;
    }

    const char* getTypeNmae() const { return "Curves"; }
    int getCurveCount() const { return _pointCount; }

    void set(chrono_t time, Imath::M44f& transform) override;

    void resize(size_t size);

    void get(DataPointer* ogeom, DataPointer* oidx);

private:

    AbcGeom::ICurves _curves;
    AbcGeom::ICurvesSchema::Sample _curveSample;

    int _pointCount;
    int _indexCount;

    int _capacity;
};

class PolyMesh : public abcrGeom
{
public:

    float* _geom = nullptr;

    PolyMesh(AbcGeom::IPolyMesh pmesh);
    ~PolyMesh()
    {
        if (_polymesh) _polymesh.reset();
        if(this->_geom) delete this->_geom;
    }

    const char* getTypeNmae() const { return "PolyMesh"; }
    int getVertexCount() const { return _vertexCount; }
    size_t getVertexSize() const { return _vertexSize; }
    VertexLayout getVertexLayout() const { return _layout; }
    void set(chrono_t time, Imath::M44f& transform) override;

    void resize(size_t size);

    float* get(int* size);


private:

    bool _hasNormal;
    bool _hasUV;
    bool _hasRGB;
    bool _hasRGBA;

    AbcGeom::IPolyMesh _polymesh;
    AbcGeom::IC3fGeomParam _rgbParam;
    AbcGeom::IC4fGeomParam _rgbaParam;

    AbcGeom::IPolyMeshSchema::Sample _meshSample;
    AbcGeom::IV2fGeomParam::Sample _uvSample;
    AbcGeom::IN3fGeomParam::Sample _normSample;

    AbcGeom::IC3fGeomParam::Sample _rgbSample;
    AbcGeom::IC4fGeomParam::Sample _rgbaSample;

    VertexLayout _layout;
        
    size_t _vertexSize;
    int _vertexCount;
    int _capacity;
};

class Camera : public abcrGeom
{
public:
    Matrix4x4 _view;
    CameraParam _proj;

    Camera(AbcGeom::ICamera camera);
    ~Camera()
    {
        if (_camera) _camera.reset();
    }

    const char* getTypeNmae() const { return "Camera"; }

    void set(chrono_t time, Imath::M44f& transform) override;

    inline bool get(Matrix4x4* ov, CameraParam* op)
    {
        *ov = this->_view;
        *op = this->_proj;
        return true;
    }

private:

    AbcGeom::ICamera _camera;

};