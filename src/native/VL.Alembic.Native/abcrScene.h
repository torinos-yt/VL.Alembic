#pragma once

#include <Alembic\Abc\All.h>
#include <Alembic\AbcCoreOgawa\All.h>

#include "abcrGeom.h"

using namespace std;
using namespace Alembic;
using namespace Alembic::Abc;

class abcrScene
{
    public:

        abcrScene();
        ~abcrScene();

        bool open(const string& path);

        bool updateSample(chrono_t time);

        bool valid() const { return _top->valid(); };

        inline float getMaxTime() const { return _maxTime; };
        inline float getMinTime() const { return _minTime; };
        inline size_t getGeomCount() const { return _nameMap.size(); };

        inline map<string, shared_ptr<abcrGeom>>::const_iterator getGeomIterator() const
        {
            return _nameMap.cbegin();
        }

        inline abcrGeom* getGeom(const string& name) const
        {
            return _fullnameMap.at(name).get();
        }

        const char* getFullName(size_t index) const
        {
            auto ite = _fullnameMap.cbegin();
            for (size_t i = 0; i < index; i++) ite++;

            char* result = new char[255];
            copyCharsWithStride(result, ite->first, 255);

            return result;
        }
            
        //bool getSample(const string& name, Matrix4x4* xform);                     //XForm
        //bool getSample(const string& name, float* points);                        //Points
        //bool getSample(const string& name, float* curves);                        //Cueves
        //bool getSample(const string& name, float* geom);                          //PolyMesh
        //bool getSample(const string& name, Matrix4x4* view, Matrix4x4* proj);     //Camera

    private:

        IArchive _archive;
        shared_ptr<abcrGeom> _top;

        chrono_t _minTime;
        chrono_t _maxTime;

        map<string, shared_ptr<abcrGeom>> _nameMap;
        map<string, shared_ptr<abcrGeom>> _fullnameMap;
};