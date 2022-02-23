#pragma once
#include "abcrGeom.h"

abcrGeom::abcrGeom() : type(AlembicType::UNKNOWN), 
                    m_minTime(std::numeric_limits<float>::infinity()), m_maxTime(0) {}

abcrGeom::abcrGeom(IObject obj)
    : m_obj(obj), type(AlembicType::UNKNOWN), constant(false), 
    m_minTime(std::numeric_limits<float>::infinity()), m_maxTime(0)
{
    this->setUpNodeRecursive(m_obj);
}

abcrGeom::~abcrGeom()
{
    m_children.clear();

    if (m_obj) m_obj.reset();
}

void abcrGeom::setUpNodeRecursive(IObject obj)
{
    size_t nChildren = obj.getNumChildren();

    for (size_t i = 0; i < nChildren; ++i)
    {
        const ObjectHeader& head = obj.getChildHeader(i);

        shared_ptr<abcrGeom> geom;

        if (AbcGeom::IXform::matches(head))
        {
            AbcGeom::IXform xform(obj.getChild(i));
            geom.reset( new XForm(xform));
        }
        else if (AbcGeom::IPoints::matches(head))
        {
            AbcGeom::IPoints points(obj.getChild(i));
            geom.reset( new Points(points));
        }
        else if (AbcGeom::ICurves::matches(head))
        {
            AbcGeom::ICurves curves(obj.getChild(i));
            geom.reset( new Curves(curves));
        }
        else if (AbcGeom::IPolyMesh::matches(head))
        {
            AbcGeom::IPolyMesh pmesh(obj.getChild(i));
            geom.reset( new PolyMesh(pmesh));
        }
        else if (AbcGeom::ICamera::matches(head))
        {
            AbcGeom::ICamera camera(obj.getChild(i));
            geom.reset( new Camera(camera));
        }
        else
        {
            geom.reset( new abcrGeom(obj.getChild(i)));
        }

        if (geom && geom->valid())
        {
            geom->index = m_children.size();
            this->m_children.emplace_back(geom);
            this->m_minTime = std::min(this->m_minTime, geom->m_minTime);
            this->m_maxTime = std::max(this->m_maxTime, geom->m_maxTime);
        }

    }
}

void abcrGeom::setUpDocRecursive(shared_ptr<abcrGeom>& obj, map<string, abcrPtr>& nameMap, map<string, abcrPtr>& fullnameMap)
{
    if (!obj->isTypeOf(AlembicType::UNKNOWN))
    {
        nameMap[obj->getName()] = abcrPtr(obj.get());
        fullnameMap[obj->getFullName()] = abcrPtr(obj.get());
    }

    for (size_t i = 0; i < obj->m_children.size(); i++)
        setUpDocRecursive(obj->m_children[i], nameMap, fullnameMap);
}


template<typename T>
void abcrGeom::setMinMaxTime(T& obj)
{
    TimeSamplingPtr tptr = obj.getSchema().getTimeSampling();
    size_t nSamples = obj.getSchema().getNumSamples();

    if (nSamples > 0)
    {
        m_minTime = tptr->getSampleTime(0);
        m_maxTime = tptr->getSampleTime(nSamples - 1);
    }   
}

void abcrGeom::updateTimeSample(chrono_t time, Imath::M44f& transform)
{
    set(time, transform);
    this->transform = transform;

    for (size_t i = 0; i < m_children.size(); ++i)
    {
        Imath::M44f m = transform;
        m_children[i]->updateTimeSample(time, m);
    }
}

XForm::XForm(AbcGeom::IXform xform)
    : abcrGeom(xform), m_xform(xform)
{
    type = AlembicType::XFORM;
    setMinMaxTime(m_xform);

    m_samplingPtr = m_xform.getSchema().getTimeSampling();

    if (m_xform.getSchema().isConstant())
    {
        this->set(m_minTime, this->transform);
        this->constant = true;
    }
}

void XForm::set(chrono_t time, Imath::M44f& transform)
{
    if (!this->constant)
    {
        ISampleSelector ss(time, ISampleSelector::kNearIndex);

        const Imath::M44d& m = m_xform.getSchema().getValue(ss).getMatrix();
        const double* src = m.getValue();
        float* dst = this->mat.getValue();

        for (size_t i = 0; i < 16; ++i) dst[i] = src[i];
    }

    transform = this->mat * transform;
}

Points::Points(AbcGeom::IPoints points)
    : abcrGeom(points), m_points(points)
{
    type = AlembicType::POINTS;
    setMinMaxTime(m_points);

    m_samplingPtr = m_points.getSchema().getTimeSampling();

    if (m_points.getSchema().isConstant())
    {
        this->set(this->m_minTime, this->transform);
        this->constant = true;
    }
}

void Points::set(chrono_t time, Imath::M44f& transform)
{
    if (this->constant) return;

    AbcGeom::IPointsSchema ptSchema = m_points.getSchema();
    AbcGeom::IPointsSchema::Sample pts_sample;

    ISampleSelector ss(time, ISampleSelector::kNearIndex);

    ptSchema.get(pts_sample, ss);

    m_positions = pts_sample.getPositions();
    m_count = m_positions->size();
}

bool Points::get(float* o)
{
    const V3f* src = m_positions->get();

    memcpy(o, src, this->getPointCount() * sizeof(V3f));
    return true;
}

Curves::Curves(AbcGeom::ICurves curves)
    : abcrGeom(curves), m_curves(curves)
{
    type = AlembicType::CURVES;
    setMinMaxTime(m_curves);

    m_samplingPtr = m_curves.getSchema().getTimeSampling();

    if (m_curves.getSchema().isConstant())
    {
        this->set(m_minTime, this->transform);
        this->constant = true;
    }
}

void Curves::set(chrono_t time, Imath::M44f& transform)
{
    return; // TODO
    /*if (this->constant) return;

    AbcGeom::ICurvesSchema curvSchema = m_curves.getSchema();
    AbcGeom::ICurvesSchema::Sample curve_sample;

    ISampleSelector ss(time, ISampleSelector::kNearIndex);

    curvSchema.get(curve_sample, ss);

    P3fArraySamplePtr m_positions = curve_sample.getPositions();

    size_t nCurves = curve_sample.getNumCurves();
    const V3f* src = m_positions->get();

    const Alembic::Util::int32_t* nVertices = curve_sample.getCurvesNumVertices()->get();

    size_t nPts = 0;
    for (size_t i = 0; i < nCurves; ++i)
    {
        nPts += nVertices[i];
    }

    if (this->curves == nullptr)
    {
        this->curves = new float[nPts*3];
    }
    else
    {
        delete this->curves;
        this->curves = new float[nPts*3];
    }

    auto index = new int[nPts];

    int cnt = 0;
    for (size_t i = 0; i < nCurves; ++i)
    {
        const int num = nVertices[i];

        for (size_t j = 0; j < num; ++j)
        {
            const V3f& v = *src;
            this->curves[cnt * 3 + 0] = v.x;
            this->curves[cnt * 3 + 1] = v.y;
            this->curves[cnt * 3 + 2] = v.z;
            src++;

            index[cnt++] = (int)(j / (float)(num - 1));
        }
    }

    this->index = index;*/
}

PolyMesh::PolyMesh(AbcGeom::IPolyMesh pmesh)
    : abcrGeom(pmesh), m_polymesh(pmesh), hasRGB(false), hasRGBA(false), hasNormal(true), hasUV(true),
    m_capacity(0), m_vertexCount(0), vertexSize(0)
{
    type = AlembicType::POLYMESH;
    setMinMaxTime(m_polymesh);

    m_samplingPtr = m_polymesh.getSchema().getTimeSampling();

    AbcGeom::IPolyMeshSchema mesh = m_polymesh.getSchema();
    auto geomParam = m_polymesh.getSchema().getArbGeomParams();

    { // normal valid
        AbcGeom::IN3fGeomParam N = mesh.getNormalsParam();
        if (!N.valid() || N.getNumSamples() <= 0 || N.getScope() == AbcGeom::kUnknownScope)
            this->hasNormal = false;
    }

    { // uvs valid
        AbcGeom::IV2fGeomParam UV = mesh.getUVsParam();
        if (!UV.valid() || UV.getNumSamples() <= 0 || UV.getScope() == AbcGeom::kUnknownScope)
            this->hasUV = false;
    }

    this->vertexSize = VertexPositionNormalTexture::VertexSize();
    this->layout = VertexLayout::PosNormTex;

    if (geomParam.valid())
    {
        size_t nParam = geomParam.getNumProperties();
        for (size_t i = 0; i < nParam; ++i)
        {
            auto& head = geomParam.getPropertyHeader(i);

            if (AbcGeom::IC3fGeomParam::matches(head))
            {
                hasRGB = true;
                vertexSize = VertexPositionNormalColorTexture::VertexSize();
                this->layout = VertexLayout::PosNormColTex;
                m_rgb = AbcGeom::IC3fGeomParam(geomParam, head.getName());
            }
            else if (AbcGeom::IC4fGeomParam::matches(head))
            {
                hasRGBA = true;
                vertexSize = VertexPositionNormalColorTexture::VertexSize();
                this->layout = VertexLayout::PosNormColTex;
                m_rgba = AbcGeom::IC4fGeomParam(geomParam, head.getName());
            }
        }
    }

    if (m_polymesh.getSchema().isConstant() &&
        ((hasRGB && m_rgb.isConstant()) ||
            (hasRGBA && m_rgba.isConstant())))
    {
        this->set(m_minTime, this->transform);
        this->constant = true;
    }
}

void PolyMesh::resize(size_t size)
{
    if (size > m_capacity)
    {
        size = std::max<size_t>(size, (size_t)m_vertexCount * (vertexSize/4) * 2);

        if(this->geom != nullptr) delete[] this->geom;

        this->geom = new float[size];
        m_capacity = size;
    }
}

void PolyMesh::set(chrono_t time, Imath::M44f& transform)
{
    if (this->constant) return;

    AbcGeom::IPolyMeshSchema mesh = m_polymesh.getSchema();
    AbcGeom::IN3fGeomParam N = mesh.getNormalsParam();
    AbcGeom::IV2fGeomParam UV = mesh.getUVsParam();

    ISampleSelector ss(time, ISampleSelector::kNearIndex);

    mesh.get(m_mesh_samp, ss);
    m_norm_samp = N.getExpandedValue(ss);
    m_uv_samp = UV.getExpandedValue(ss);

    if (hasRGB) m_rgb_samp = m_rgb.getExpandedValue(ss);
    else if(hasRGBA) m_rgba_samp = m_rgba.getExpandedValue(ss);
}

float* PolyMesh::get(int* size)
{
    //sample some property
    P3fArraySamplePtr m_points = m_mesh_samp.getPositions();
    Int32ArraySamplePtr m_indices = m_mesh_samp.getFaceIndices();
    Int32ArraySamplePtr m_faceCounts = m_mesh_samp.getFaceCounts();

    N3fArraySamplePtr m_norms;
    if(hasNormal) m_norms = m_norm_samp.getVals();

    V2fArraySamplePtr m_uvs;
    if(hasUV) m_uvs = m_uv_samp.getVals();

    size_t nPts = m_points->size();
    size_t nInds = m_indices->size();
    size_t nFace = m_faceCounts->size();
    if (nPts < 1 || nInds < 1 || nFace < 1)
    {
        *size = 0;
        return nullptr;
    }

    using tri = Imath::Vec3<unsigned int>;
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
                m_triangles.push_back(tri((unsigned int)fBegin + 0,
                    (unsigned int)fBegin + 1,
                    (unsigned int)fBegin + 2));
                for (size_t c = 3; c < count; ++c)
                {
                    m_triangles.push_back(tri((unsigned int)fBegin + 0,
                        (unsigned int)fBegin + c - 1,
                        (unsigned int)fBegin + c));
                }
            }
        }
    }

    size_t sizeInBytes = m_triangles.size() * 3 * vertexSize;
    this->resize(sizeInBytes / 4);
    m_vertexCount = m_triangles.size() * 3;

    {
        const V3f* points = m_points->get();
        const N3f* norms = m_norms->get();
        const V2f* uvs = m_uvs->get();
        const int32_t* indices = m_indices->get();

        float* stream = this->geom;

        if (hasRGB)
        {
            const auto cols_ptr = m_rgb_samp.getVals();
            auto cdCount = cols_ptr->size();
            bool isIndexedColor = cdCount == m_points->size();

            const C3f* cols = cols_ptr->get();
            for (size_t j = 0; j < m_triangles.size(); ++j)
            {
                tri& t = m_triangles[j];

                const V3f& v0 = points[indices[t[0]]];
                const N3f& n0 = norms[t[0]];
                const V2f& uv0 = uvs[t[0]];
                const C3f& col0 = isIndexedColor ? cols[indices[t[0]]] : cols[t[0]];

                const V3f& v1 = points[indices[t[1]]];
                const N3f& n1 = norms[t[1]];
                const V2f& uv1 = uvs[t[1]];
                const C3f& col1 = isIndexedColor ? cols[indices[t[1]]] : cols[t[1]];

                const V3f& v2 = points[indices[t[2]]];
                const N3f& n2 = norms[t[2]];
                const V2f& uv2 = uvs[t[2]];
                const C3f& col2 = isIndexedColor ? cols[indices[t[2]]] : cols[t[2]];

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
        else if (hasRGBA)
        {
            const auto cols_ptr = m_rgba_samp.getVals();
            auto cdCount = cols_ptr->size();
            bool isIndexedColor = cdCount == m_points->size();

            const C4f* cols = m_rgba_samp.getVals()->get();
            for (size_t j = 0; j < m_triangles.size(); ++j)
            {
                tri& t = m_triangles[j];

                const V3f& v0 = points[indices[t[0]]];
                const N3f& n0 = norms[t[0]];
                const V2f& uv0 = uvs[t[0]];
                const C4f& col0 = isIndexedColor ? cols[indices[t[0]]] : cols[t[0]];

                const V3f& v1 = points[indices[t[1]]];
                const N3f& n1 = norms[t[1]];
                const V2f& uv1 = uvs[t[1]];
                const C4f& col1 = isIndexedColor ? cols[indices[t[1]]] : cols[t[1]];

                const V3f& v2 = points[indices[t[2]]];
                const N3f& n2 = norms[t[2]];
                const V2f& uv2 = uvs[t[2]];
                const C4f& col2 = isIndexedColor ? cols[indices[t[2]]] : cols[t[2]];

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

                const V3f& v0 = points[indices[t[0]]];
                const N3f& n0 = norms[t[0]];
                const V2f& uv0 = uvs[t[0]];

                const V3f& v1 = points[indices[t[1]]];
                const N3f& n1 = norms[t[1]];
                const V2f& uv1 = uvs[t[2]];

                const V3f& v2 = points[indices[t[2]]];
                const N3f& n2 = norms[t[2]];
                const V2f& uv2 = uvs[t[2]];

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
    return this->geom;
}

Camera::Camera(AbcGeom::ICamera camera)
    : abcrGeom(camera), m_camera(camera), View(), Proj()
{
    type = AlembicType::CAMERA;
    setMinMaxTime(m_camera);

    if (camera.getSchema().isConstant())
    {
        this->set(m_minTime, this->transform);
        this->constant = true;
    }
}

void Camera::set(chrono_t time, Imath::M44f& transform)
{
    if (!this->constant)
    {
        ISampleSelector ss(time, ISampleSelector::kNearIndex);

        AbcGeom::CameraSample cam_samp;
        AbcGeom::ICameraSchema camSchema = m_camera.getSchema();

        camSchema.get(cam_samp);

        float Aperture = cam_samp.getVerticalAperture();
        float Near = std::max(cam_samp.getNearClippingPlane(), .001);
        float Far = std::min(cam_samp.getFarClippingPlane(), 100000.0);
        float ForcalLength = cam_samp.getFocalLength();
        float FoV = 2.0 * (atan(Aperture * 10.0 / (2.0 * ForcalLength))) * (180.0f / M_PI);

        this->Proj = CameraParam(Aperture, Near, Far, ForcalLength, FoV * (1.0f / 360));
    }

    this->View = toVVVV(transform.invert());
}