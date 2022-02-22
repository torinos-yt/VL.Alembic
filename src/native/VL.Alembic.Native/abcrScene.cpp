#include "abcrScene.h"

abcrScene::abcrScene() {}

abcrScene::~abcrScene() 
{
    this->nameMap.clear();
    this->fullnameMap.clear();

    if (m_top) m_top.reset();
    if (m_archive.valid()) m_archive.reset();
}

bool abcrScene::open(const string& path)
{
    m_archive = IArchive(AbcCoreOgawa::ReadArchive(), path,
        Alembic::Abc::ErrorHandler::kQuietNoopPolicy);

    if (!m_archive.valid()) return false;

    m_top.reset( new abcrGeom(m_archive.getTop()) );
        
    this->nameMap.clear();
    this->fullnameMap.clear();
    abcrGeom::setUpDocRecursive(m_top, nameMap, fullnameMap);
        
    m_minTime = m_top->m_minTime;
    m_maxTime = m_top->m_maxTime;

    return true;
}

bool abcrScene::updateSample(chrono_t time)
{
    if (!m_top) return false;

    ISampleSelector ss(time, ISampleSelector::kNearIndex);

    Imath::M44f m;
    m.makeIdentity();
    m_top->updateTimeSample(time, m);

    return true;
}

////XForm
//bool abcrScene::getSample(const string& path, Matrix4x4& xform)
//{
//    return false;
//}
//
////Points
//bool abcrScene::getSample(const string& path, ISpread<Vector3D>^& points)
//{
//    return false;
//}
//
////Curves
//bool abcrScene::getSample(const string& path, ISpread<ISpread<Vector3D>^>^& curves)
//{
//    return false;
//}
//
////PolyMesh
//bool abcrScene::getSample(const string& path, DX11VertexGeometry^ geom)
//{
//    return false;
//}
//
////Camera
//bool abcrScene::getSample(const string& path, Matrix4x4& view, Matrix4x4& proj)
//{
//    return false;
//}