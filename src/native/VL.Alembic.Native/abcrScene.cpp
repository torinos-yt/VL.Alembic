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