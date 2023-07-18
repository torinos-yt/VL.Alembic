#pragma once
#include "abcrGeom.h"

abcrGeom::abcrGeom() : _type(AlembicType::UNKNOWN), 
                    _minTime(std::numeric_limits<float>::infinity()), _maxTime(0) {}

abcrGeom::abcrGeom(IObject obj)
    : _obj(obj), _type(AlembicType::UNKNOWN), _constant(false), 
    _minTime(std::numeric_limits<float>::infinity()), _maxTime(0)
{
    this->setUpNodeRecursive(_obj);
}

abcrGeom::~abcrGeom()
{
    _children.clear();

    if (_obj) _obj.reset();
}

void abcrGeom::setUpNodeRecursive(IObject obj)
{
    size_t nChildren = obj.getNumChildren();

    for (size_t i = 0; i < nChildren; ++i)
    {
        const ObjectHeader& head = obj.getChildHeader(i);

        shared_ptr<abcrGeom> _geom;

        if (AbcGeom::IXform::matches(head))
        {
            AbcGeom::IXform xform(obj.getChild(i));
            _geom.reset( new XForm(xform));
        }
        else if (AbcGeom::IPoints::matches(head))
        {
            AbcGeom::IPoints points(obj.getChild(i));
            _geom.reset( new Points(points));
        }
        else if (AbcGeom::ICurves::matches(head))
        {
            AbcGeom::ICurves curves(obj.getChild(i));
            _geom.reset( new Curves(curves));
        }
        else if (AbcGeom::IPolyMesh::matches(head))
        {
            AbcGeom::IPolyMesh pmesh(obj.getChild(i));
            _geom.reset( new PolyMesh(pmesh));
        }
        else if (AbcGeom::ICamera::matches(head))
        {
            AbcGeom::ICamera camera(obj.getChild(i));
            _geom.reset( new Camera(camera));
        }
        else
        {
            _geom.reset( new abcrGeom(obj.getChild(i)));
        }

        if (_geom && _geom->valid())
        {
            _geom->_index = _children.size();
            this->_children.emplace_back(_geom);
            this->_minTime = std::min(this->_minTime, _geom->_minTime);
            this->_maxTime = std::max(this->_maxTime, _geom->_maxTime);
        }

    }
}

void abcrGeom::setUpDocRecursive(shared_ptr<abcrGeom>& obj, map<string, shared_ptr<abcrGeom>>& nameMap, map<string, shared_ptr<abcrGeom>>& fullnameMap)
{
    if (!obj->isTypeOf(AlembicType::UNKNOWN))
    {
        nameMap[obj->getName()] = obj;
        fullnameMap[obj->getFullName()] = obj;
    }

    for (size_t i = 0; i < obj->_children.size(); i++)
        setUpDocRecursive(obj->_children[i], nameMap, fullnameMap);
}

void abcrGeom::getInterpolateSampleSelector(chrono_t time, ISampleSelector& ss0, ISampleSelector& ss1, chrono_t& t)
{
    auto prevTime = _samplingPtr->getSampleTime(_lastSampleIndex);

    ss0 = ISampleSelector(time, prevTime < time ? ISampleSelector::kFloorIndex : ISampleSelector::kCeilIndex);
    ss1 = ISampleSelector(time, prevTime < time ? ISampleSelector::kCeilIndex : ISampleSelector::kFloorIndex);

    auto time0 = _samplingPtr->getSampleTime(ss0.getIndex(_samplingPtr, _numSamples));
    auto time1 = _samplingPtr->getSampleTime(ss1.getIndex(_samplingPtr, _numSamples));

    t = (time - time0) / (time1 - time0);
}


template<typename T>
void abcrGeom::setMinMaxTime(T& obj)
{
    TimeSamplingPtr tptr = obj.getSchema().getTimeSampling();
    size_t nSamples = obj.getSchema().getNumSamples();

    if (nSamples > 0)
    {
        _minTime = tptr->getSampleTime(0);
        _maxTime = tptr->getSampleTime(nSamples - 1);
    }   
}

void abcrGeom::updateTimeSample(chrono_t time, Imath::M44f& transform)
{
    set(time, transform);
    _transform = transform;

    for (size_t i = 0; i < _children.size(); ++i)
    {
        Imath::M44f m = transform;
        _children[i]->setInterpolate(_isInterpolate);
        _children[i]->updateTimeSample(time, m);
    }
}

XForm::XForm(AbcGeom::IXform xform)
    : abcrGeom(xform), _xform(xform)
{
    _type = AlembicType::XFORM;
    setMinMaxTime(_xform);

    _samplingPtr = _xform.getSchema().getTimeSampling();
    _numSamples = _xform.getSchema().getNumSamples();

    if (_xform.getSchema().isConstant())
    {
        this->set(_minTime, _transform);
        _constant = true;
    }
}

void XForm::set(chrono_t time, Imath::M44f& transform)
{
    if (!_constant)
    {
        if (_isInterpolate)
        {
            ISampleSelector ss0, ss1;
            getInterpolateSampleSelector(time, ss0, ss1, _t);

            const Imath::M44d& m0 = _xform.getSchema().getValue(ss0).getMatrix();
            const Imath::M44d& m1 = _xform.getSchema().getValue(ss1).getMatrix();

            Imath::V3d t0, t1;
            Imath::V3d s0, s1;
            Imath::V3d sh0, sh1;
            Imath::Quatd r0, r1;

            decomposeMatrix(m0, s0, sh0, r0, t0);
            decomposeMatrix(m1, s1, sh1, r1, t1);

            Imath::M44d m;
            m.makeIdentity();
            m.scale(s0 * (1 - _t) + s1 * _t);
            m *= Imath::slerpShortestArc(r0, r1, _t).toMatrix44();

            Imath::V3d t2 = t0 * (1 - _t) + t1 * _t;
            m[3][0] = t2.x;
            m[3][1] = t2.y;
            m[3][2] = t2.z;

            const double* src = m.getValue();
            float* dst = _matrix.getValue();

            for (size_t i = 0; i < 16; ++i) dst[i] = src[i];

            _lastSampleIndex = ss0.getIndex(_samplingPtr, _numSamples);
        }
        else
        {
            ISampleSelector ss(time, ISampleSelector::kNearIndex);

            const Imath::M44d& m = _xform.getSchema().getValue(ss).getMatrix();
            const double* src = m.getValue();
            float* dst = _matrix.getValue();

            for (size_t i = 0; i < 16; ++i) dst[i] = src[i];

            _lastSampleIndex = ss.getIndex(_samplingPtr, _numSamples);
        }
    }

    transform = _matrix * transform;
}

Points::Points(AbcGeom::IPoints points)
    : abcrGeom(points), _points(points)
{
    _type = AlembicType::POINTS;
    setMinMaxTime(_points);

    _samplingPtr = _points.getSchema().getTimeSampling();
    _numSamples = _points.getSchema().getNumSamples();

    if (_points.getSchema().isConstant())
    {
        this->set(_minTime, _transform);
        _constant = true;
    }
}

void Points::set(chrono_t time, Imath::M44f& transform)
{
    if (_constant) return;

    AbcGeom::IPointsSchema ptSchema = _points.getSchema();
    AbcGeom::IPointsSchema::Sample pts_sample;

    ISampleSelector ss(time, ISampleSelector::kNearIndex);

    ptSchema.get(pts_sample, ss);

    _positions = pts_sample.getPositions();
    _pointCount = _positions->size();
}

bool Points::get(float* o)
{
    const V3f* src = _positions->get();

    memcpy(o, src, this->getPointCount() * sizeof(V3f));
    return true;
}

Curves::Curves(AbcGeom::ICurves curves)
    : abcrGeom(curves), _curves(curves)
{
    _type = AlembicType::CURVES;
    setMinMaxTime(curves);

    _samplingPtr = curves.getSchema().getTimeSampling();
    _numSamples = curves.getSchema().getNumSamples();

    if (curves.getSchema().isConstant())
    {
        this->set(_minTime, _transform);
        _constant = true;
    }
}

void Curves::set(chrono_t time, Imath::M44f& transform)
{
    if (this->_constant) return;

    AbcGeom::ICurvesSchema curvSchema = _curves.getSchema();
    AbcGeom::ICurvesSchema::Sample curve_sample;

    ISampleSelector ss(time, ISampleSelector::kNearIndex);

    curvSchema.get(_curveSample, ss);
}

void Curves::resize(size_t size)
{
    if (size > _capacity)
    {
        size = std::max<size_t>(size, (size_t)_indexCount * 2);

        if (_index != nullptr) delete[] _index;

        _index = new uint32_t[size];
        _capacity = size;
    }
}

void Curves::get(DataPointer* ocurve, DataPointer* oidx)
{
    P3fArraySamplePtr positions = _curveSample.getPositions();

    size_t nCurves = _curveSample.getNumCurves();
    const Alembic::Util::int32_t* nVertices = _curveSample.getCurvesNumVertices()->get();

    _indexCount = 0;
    for (size_t i = 0; i < nCurves; ++i)
    {
        _indexCount += nVertices[i] * 2 - 2;
    }

    this->resize(_indexCount);

    int cnt = 0;
    int cnt2 = 0;
    for (size_t i = 0; i < nCurves; ++i)
    {
        const int num = nVertices[i];

        for (size_t j = 0; j < num - (size_t)1; ++j)
        {
            int idx = cnt2 + j;
            _index[cnt++] = (uint32_t)idx + 0;
            _index[cnt++] = (uint32_t)idx + 1;
        }
        cnt2 += num;
    }

    *ocurve = DataPointer((void*)positions->get(), (int)positions->size() * 4 * 3);
    *oidx  = DataPointer(_index, _indexCount * 4);
}


PolyMesh::PolyMesh(AbcGeom::IPolyMesh pmesh)
    : abcrGeom(pmesh), _polymesh(pmesh), _hasRGB(false), _hasRGBA(false), _hasNormal(true), _hasUV(true),
    _capacity(0), _vertexCount(0), _vertexSize(0)
{
    _type = AlembicType::POLYMESH;
    setMinMaxTime(_polymesh);

    _samplingPtr = _polymesh.getSchema().getTimeSampling();
    _numSamples = _polymesh.getSchema().getNumSamples();

    AbcGeom::IPolyMeshSchema mesh = _polymesh.getSchema();
    auto geomParam = _polymesh.getSchema().getArbGeomParams();

    _topologyVariance = mesh.getTopologyVariance();

    { // normal valid
        AbcGeom::IN3fGeomParam N = mesh.getNormalsParam();
        if (!N.valid() || N.getNumSamples() <= 0 || N.getScope() == AbcGeom::kUnknownScope)
            _hasNormal = false;
    }

    { // uvs valid
        AbcGeom::IV2fGeomParam UV = mesh.getUVsParam();
        if (!UV.valid() || UV.getNumSamples() <= 0 || UV.getScope() == AbcGeom::kUnknownScope)
            _hasUV = false;
    }

    _vertexSize = VertexPositionNormalTexture::VertexSize();
    _layout = VertexLayout::PosNormTex;

    if (geomParam.valid())
    {
        size_t nParam = geomParam.getNumProperties();
        for (size_t i = 0; i < nParam; ++i)
        {
            auto& head = geomParam.getPropertyHeader(i);

            if (AbcGeom::IC3fGeomParam::matches(head))
            {
                _hasRGB = true;
                _vertexSize = VertexPositionNormalColorTexture::VertexSize();
                _layout = VertexLayout::PosNormColTex;
                _rgbParam = AbcGeom::IC3fGeomParam(geomParam, head.getName());
            }
            else if (AbcGeom::IC4fGeomParam::matches(head))
            {
                _hasRGBA = true;
                _vertexSize = VertexPositionNormalColorTexture::VertexSize();
                _layout = VertexLayout::PosNormColTex;
                _rgbaParam = AbcGeom::IC4fGeomParam(geomParam, head.getName());
            }
        }
    }

    if (_polymesh.getSchema().isConstant() &&
        ((_hasRGB && _rgbParam.isConstant()) ||
            (_hasRGBA && _rgbaParam.isConstant())))
    {
        this->set(_minTime, _transform);
        _constant = true;
    }

    size_t numSamples = _polymesh.getSchema().getNumSamples();
    AbcGeom::IPolyMeshSchema::Sample sample;
    size_t maxSize = 0;
    for (index_t i = 0; i < numSamples; ++i)
    {
        ISampleSelector ss(i);
        _polymesh.getSchema().get(sample, ss);
        maxSize = std::max<size_t>(maxSize, sample.getPositions()->size());
    }

    this->resize(maxSize * _vertexSize / 4 * 2);
}

void PolyMesh::resize(size_t size)
{
    if (size > _capacity)
    {
        size = std::max<size_t>(size, (size_t)_vertexCount * (_vertexSize/4) * 2);

        if(_geom != nullptr) delete[] _geom;

        _geom = new float[size];
        _capacity = size;
    }
}

void PolyMesh::set(chrono_t time, Imath::M44f& transform)
{
    if (_constant) return;
    //_isInterpolate = false;

    AbcGeom::IPolyMeshSchema mesh = _polymesh.getSchema();
    AbcGeom::IN3fGeomParam N = mesh.getNormalsParam();
    AbcGeom::IV2fGeomParam UV = mesh.getUVsParam();

    if (_isInterpolate)
    {
        ISampleSelector ss0, ss1;
        getInterpolateSampleSelector(time, ss0, ss1, _t);

        mesh.get(_meshSample, ss0);
        mesh.get(_meshSample2, ss1);

        if (_hasNormal)
        {
            _normSample  = N.getExpandedValue(ss0);
            _normSample2 = N.getExpandedValue(ss1);
        }

        if (_hasUV)
        {
            _uvSample  = UV.getExpandedValue(ss0);
            _uvSample2 = UV.getExpandedValue(ss1);
        }

        if (_hasRGB)
        {
            _rgbSample  = _rgbParam.getExpandedValue(ss0);
            _rgbSample2 = _rgbParam.getExpandedValue(ss1);
        }
        else if (_hasRGBA)
        {
            _rgbaSample  = _rgbaParam.getExpandedValue(ss0);
            _rgbaSample2 = _rgbaParam.getExpandedValue(ss1);
        }
    }
    else
    {
        ISampleSelector ss(time, ISampleSelector::kNearIndex);

        mesh.get(_meshSample, ss);
        if (_hasNormal) _normSample = N.getExpandedValue(ss);
        if (_hasUV) _uvSample = UV.getExpandedValue(ss);

        if (_hasRGB) _rgbSample = _rgbParam.getExpandedValue(ss);
        else if (_hasRGBA) _rgbaSample = _rgbaParam.getExpandedValue(ss);
    }
}

float* PolyMesh::get(int* size)
{
    //_isInterpolate = false;
    //sample some property
    P3fArraySamplePtr m_points, m_points2;
    m_points = _meshSample.getPositions();
    Int32ArraySamplePtr m_indices = _meshSample.getFaceIndices();
    Int32ArraySamplePtr m_faceCounts = _meshSample.getFaceCounts();

    N3fArraySamplePtr m_norms, m_norms2;
    if (_hasNormal) m_norms = _normSample.getVals();

    V2fArraySamplePtr m_uvs, m_uvs2;
    if (_hasUV) m_uvs  = _uvSample.getVals();

    if (_isInterpolate)
    {
        m_points2 = _meshSample2.getPositions();
        if(_hasNormal) m_norms2 = _normSample2.getVals();
        if(_hasUV) m_uvs2 = _uvSample2.getVals();
    }

    size_t nPts = m_points->size();
    size_t nInds = m_indices->size();
    size_t nFace = m_faceCounts->size();
    if (nPts < 1 || nInds < 1 || nFace < 1)
    {
        *size = 0;
        return nullptr;
    }

    using tri = Imath::Vec3<uint32_t>;
    using triArray = std::vector<tri>;
    triArray m_triangles;
    std::vector<int32_t> inds;

    {
        size_t fBegin = 0;
        size_t fEnd = 0;
        for (size_t face = 0; face < nFace; ++face)
        {
            fBegin = fEnd;
            size_t count = (*m_faceCounts)[face];
            fEnd = fBegin + count;

            if (fEnd > nInds || fEnd < fBegin)
            {
                *size = 0;
                return nullptr;
            }

            if (count >= 3)
            {
                m_triangles.push_back(tri((uint32_t)fBegin + 0,
                    (uint32_t)fBegin + 1,
                    (uint32_t)fBegin + 2));
                for (size_t c = 3; c < count; ++c)
                {
                    m_triangles.push_back(tri((uint32_t)fBegin + 0,
                        (uint32_t)fBegin + c - 1,
                        (uint32_t)fBegin + c));
                }
            }
        }
    }

    size_t sizeInBytes = m_triangles.size() * 3 * _vertexSize;
    this->resize(sizeInBytes / 4);
    _vertexCount = m_triangles.size() * 3;

    {
        const V3f *points, *points2 = nullptr;
        points = m_points->get();

        const N3f *norms, *norms2 = nullptr;
        norms = _hasNormal ? m_norms->get() : nullptr;

        const V2f *uvs, *uvs2 = nullptr;
        uvs = _hasUV ? m_uvs->get() : nullptr;

        if (_isInterpolate)
        {
            points2 = m_points2->get();
            norms2 = _hasNormal ? m_norms2->get() : nullptr;
            uvs2 = _hasUV ? m_uvs2->get() : nullptr;
        }

        const int32_t* indices = m_indices->get();

        float* stream = _geom;

        if (_hasRGB)
        {
            const auto cols_ptr = _rgbSample.getVals();
            const auto cols_ptr2 = _rgbSample2.getVals();
            auto cdCount = cols_ptr->size();
            bool isIndexedColor = cdCount == m_points->size();

            const C3f* cols = cols_ptr->get();
            const C3f* cols2 = cols_ptr2->get();
            for (size_t j = 0; j < m_triangles.size(); ++j)
            {
                tri& t = m_triangles[j];

                V3f v0 = points[indices[t[0]]];
                V2f uv0 = _hasUV ? uvs[t[0]] : V2f(0);
                C3f col0 = isIndexedColor ? cols[indices[t[0]]] : cols[t[0]];

                V3f v1 = points[indices[t[1]]];
                V2f uv1 = _hasUV ? uvs[t[1]] : V2f(0);
                C3f col1 = isIndexedColor ? cols[indices[t[1]]] : cols[t[1]];

                V3f v2 = points[indices[t[2]]];
                V2f uv2 = _hasUV ? uvs[t[2]] : V2f(0);
                C3f col2 = isIndexedColor ? cols[indices[t[2]]] : cols[t[2]];
                
                const N3f faceNormal = _hasNormal ? N3f(0) : computeFaceNormal(v0, v1, v2);
                N3f n0 = _hasNormal ? norms[t[0]] : faceNormal;
                N3f n1 = _hasNormal ? norms[t[1]] : faceNormal;
                N3f n2 = _hasNormal ? norms[t[2]] : faceNormal;

                if (_isInterpolate)
                {
                    v0 += (points2[indices[t[0]]] - v0) * _t;
                    uv0 += _hasUV ? (uvs2[t[0]] - uv0) * _t : V2f(0);
                    col0 += isIndexedColor ? (cols2[indices[t[0]]] - col0) * _t : (cols2[t[0]] - col0) * _t;

                    v1 += (points2[indices[t[1]]] - v1) * _t;
                    uv2 += _hasUV ? (uvs2[t[1]] - uv1) * _t : V2f(0);
                    col1 += isIndexedColor ? (cols2[indices[t[1]]] - col1) * _t : (cols2[t[1]] - col1) * _t;

                    v2 += (points2[indices[t[2]]] - v2) * _t;
                    uv2 += _hasUV ? (uvs2[t[2]] - uv2) * _t : V2f(0);
                    col2 += isIndexedColor ? (cols2[indices[t[2]]] - col2) * _t : (cols2[t[2]] - col2) * _t;

                    const N3f faceNormal2 = _hasNormal ? N3f(0) : computeFaceNormal(v0, v1, v2);
                    n0 = _hasNormal ? n0 + (norms2[t[0]] - n0) * _t : faceNormal2;
                    n1 = _hasNormal ? n1 + (norms2[t[1]] - n1) * _t : faceNormal2;
                    n2 = _hasNormal ? n2 + (norms2[t[2]] - n2) * _t : faceNormal2;
                }

                copyTo(stream, v0);
                copyTo(stream, n0);
                copyTo(stream, col0);
                copyTo(stream, uv0);

                copyTo(stream, v1);
                copyTo(stream, n1);
                copyTo(stream, col1);
                copyTo(stream, uv1);

                copyTo(stream, v2);
                copyTo(stream, n2);
                copyTo(stream, col2);
                copyTo(stream, uv2);
            }
        }
        else if (_hasRGBA)
        {
            const auto cols_ptr = _rgbaSample.getVals();
            auto cdCount = cols_ptr->size();
            bool isIndexedColor = cdCount == m_points->size();

            const C4f* cols = _rgbaSample.getVals()->get();
            const C4f* cols2 = _rgbaSample2.getVals()->get();
            for (size_t j = 0; j < m_triangles.size(); ++j)
            {
                tri& t = m_triangles[j];

                V3f v0 = points[indices[t[0]]];
                V2f uv0 = _hasUV ? uvs[t[0]] : V2f(0);
                C4f col0 = isIndexedColor ? cols[indices[t[0]]] : cols[t[0]];

                V3f v1 = points[indices[t[1]]];
                V2f uv1 = _hasUV ? uvs[t[1]] : V2f(0);
                C4f col1 = isIndexedColor ? cols[indices[t[1]]] : cols[t[1]];

                V3f v2 = points[indices[t[2]]];
                V2f uv2 = _hasUV ? uvs[t[2]] : V2f(0);
                C4f col2 = isIndexedColor ? cols[indices[t[2]]] : cols[t[2]];

                N3f faceNormal = _hasNormal ? N3f(0) : computeFaceNormal(v0, v1, v2);
                N3f n0 = _hasNormal ? norms[t[0]] : faceNormal;
                N3f n1 = _hasNormal ? norms[t[1]] : faceNormal;
                N3f n2 = _hasNormal ? norms[t[2]] : faceNormal;

                if (_isInterpolate)
                {
                    v0 += (points2[indices[t[0]]] - v0) * _t;
                    uv0 += _hasUV ? (uvs2[t[0]] - uv0) * _t : V2f(0);
                    col0 += isIndexedColor ? (cols2[indices[t[0]]] - col0) * _t : (cols2[t[0]] - col0) * _t;

                    v1 += (points2[indices[t[1]]] - v1) * _t;
                    uv2 += _hasUV ? (uvs2[t[1]] - uv1) * _t : V2f(0);
                    col1 += isIndexedColor ? (cols2[indices[t[1]]] - col1) * _t : (cols2[t[1]] - col1) * _t;

                    v2 += (points2[indices[t[2]]] - v1) * _t;
                    uv2 += _hasUV ? (uvs2[t[2]] - uv1) * _t : V2f(0);
                    col2 += isIndexedColor ? (cols2[indices[t[2]]] - col2) * _t : (cols2[t[2]] - col2) * _t;

                    const N3f faceNormal2 = _hasNormal ? N3f(0) : computeFaceNormal(v0, v1, v2);
                    n0 = _hasNormal ? n0 + (norms2[t[0]] - n0) * _t : faceNormal2;
                    n1 = _hasNormal ? n1 + (norms2[t[1]] - n1) * _t : faceNormal2;
                    n2 = _hasNormal ? n2 + (norms2[t[2]] - n2) * _t : faceNormal2;
                }

                copyTo(stream, v0);
                copyTo(stream, n0);
                copyTo(stream, col0);
                copyTo(stream, uv0);

                copyTo(stream, v1);
                copyTo(stream, n1);
                copyTo(stream, col1);
                copyTo(stream, uv1);

                copyTo(stream, v2);
                copyTo(stream, n2);
                copyTo(stream, col2);
                copyTo(stream, uv2);
            }
        }
        else
        {
            for (size_t j = 0; j < m_triangles.size(); ++j)
            {
                tri& t = m_triangles[j];

                V3f v0 = points[indices[t[0]]];
                V2f uv0 = _hasUV ? uvs[t[0]] : V2f(0);

                V3f v1 = points[indices[t[1]]];
                V2f uv1 = _hasUV ? uvs[t[1]] : V2f(0);

                V3f v2 = points[indices[t[2]]];
                V2f uv2 = _hasUV ? uvs[t[2]] : V2f(0);

                N3f faceNormal = _hasNormal ? N3f(0) : computeFaceNormal(v0, v1, v2);
                N3f n0 = _hasNormal ? norms[t[0]] : faceNormal;
                N3f n1 = _hasNormal ? norms[t[1]] : faceNormal;
                N3f n2 = _hasNormal ? norms[t[2]] : faceNormal;

                if (_isInterpolate)
                {
                    v0 += (points2[indices[t[0]]] - v0) * _t;
                    uv0 += _hasUV ? (uvs2[t[0]] - uv0) * _t : V2f(0);

                    v1 += (points2[indices[t[1]]] - v1) * _t;
                    uv2 += _hasUV ? (uvs2[t[1]] - uv1) * _t : V2f(0);

                    v2 += (points2[indices[t[2]]] - v2) * _t;
                    uv2 += _hasUV ? (uvs2[t[2]] - uv2) * _t : V2f(0);

                    const N3f faceNormal2 = _hasNormal ? N3f(0) : computeFaceNormal(v0, v1, v2);
                    n0 = _hasNormal ? n0 + (norms2[t[0]] - n0) * _t : faceNormal2;
                    n1 = _hasNormal ? n1 + (norms2[t[1]] - n1) * _t : faceNormal2;
                    n2 = _hasNormal ? n2 + (norms2[t[2]] - n2) * _t : faceNormal2;
                }

                copyTo(stream, v0);
                copyTo(stream, n0);
                copyTo(stream, uv0);

                copyTo(stream, v1);
                copyTo(stream, n1);
                copyTo(stream, uv1);

                copyTo(stream, v2);
                copyTo(stream, n2);
                copyTo(stream, uv2);
            }
        }
    }

    *size = sizeInBytes;
    return _geom;
}

Camera::Camera(AbcGeom::ICamera camera)
    : abcrGeom(camera), _camera(camera), _view(), _proj()
{
    _type = AlembicType::CAMERA;
    setMinMaxTime(_camera);

    _samplingPtr = _camera.getSchema().getTimeSampling();
    _numSamples = _camera.getSchema().getNumSamples();

    if (camera.getSchema().isConstant())
    {
        this->set(_minTime, _transform);
        _constant = true;
    }
}

void Camera::set(chrono_t time, Imath::M44f& transform)
{
    if (!_constant)
    {
        float Aperture, Near, Far, ForcalLength, FoV;
        if(_isInterpolate)
        { 
            ISampleSelector ss0, ss1;
            getInterpolateSampleSelector(time, ss0, ss1, _t);

            AbcGeom::CameraSample cam_samp, cam_samp2;
            AbcGeom::ICameraSchema camSchema = _camera.getSchema();

            camSchema.get(cam_samp, ss0);
            camSchema.get(cam_samp2, ss1);

            Aperture = cam_samp.getVerticalAperture() * (1 - _t) + cam_samp2.getVerticalAperture() * _t;
            Near = std::max(cam_samp.getNearClippingPlane() * (1 - _t) + cam_samp2.getNearClippingPlane() * _t, .001);
            Far = std::min(cam_samp.getFarClippingPlane() * (1 - _t) + cam_samp2.getFarClippingPlane() * _t, 100000.0);
            ForcalLength = cam_samp.getFocalLength() * (1 - _t) + cam_samp2.getFocalLength() * _t;
        }
        else
        {
            ISampleSelector ss(time, ISampleSelector::kNearIndex);

            AbcGeom::CameraSample cam_samp;
            AbcGeom::ICameraSchema camSchema = _camera.getSchema();

            camSchema.get(cam_samp, ss);

            Aperture = cam_samp.getVerticalAperture();
            Near = std::max(cam_samp.getNearClippingPlane(), .001);
            Far = std::min(cam_samp.getFarClippingPlane(), 100000.0);
            ForcalLength = cam_samp.getFocalLength();
        }

        FoV = 2.0 * (atan(Aperture * 10.0 / (2.0 * ForcalLength))) * (180.0f / M_PI);
        _proj = CameraParam(Aperture, Near, Far, ForcalLength, FoV * (1.0f / 360));
    }

    _view = toVVVV(transform.invert());
}