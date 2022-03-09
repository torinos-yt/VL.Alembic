#include "abcrScene.h"

abcrScene::abcrScene() {}

abcrScene::~abcrScene() 
{
    this->_nameMap.clear();
    this->_fullnameMap.clear();

    if (_top) _top.reset();
    if (_archive.valid()) _archive.reset();
}

bool abcrScene::open(const string& path)
{
    _archive = IArchive(AbcCoreOgawa::ReadArchive(), path,
        Alembic::Abc::ErrorHandler::kQuietNoopPolicy);

    if (!_archive.valid()) return false;

    _top.reset( new abcrGeom(_archive.getTop()) );
        
    this->_nameMap.clear();
    this->_fullnameMap.clear();
    abcrGeom::setUpDocRecursive(_top, _nameMap, _fullnameMap);
        
    _minTime = _top->_minTime;
    _maxTime = _top->_maxTime;

    return true;
}

bool abcrScene::updateSample(chrono_t time)
{
    if (!_top) return false;

    ISampleSelector ss(time, ISampleSelector::kNearIndex);

    Imath::M44f m;
    m.makeIdentity();
    _top->updateTimeSample(time, m);

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