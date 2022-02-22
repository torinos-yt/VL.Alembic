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

        bool valid() const { return m_top->valid(); };

        inline float getMaxTime() const { return m_maxTime; };
        inline float getMinTime() const { return m_minTime; };
        inline size_t getGeomCount() const { return nameMap.size(); };

        inline map<string, abcrPtr>::const_iterator getGeomIterator() const
        {
            return nameMap.cbegin();
        }

        inline abcrGeom* getGeom(const string& name) const
        {
            return fullnameMap.at(name).m_ptr;
        }

        const char* getFullName(size_t index) const
        {
            auto ite = fullnameMap.cbegin();
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

        IArchive m_archive;
        shared_ptr<abcrGeom> m_top;

        chrono_t m_minTime;
        chrono_t m_maxTime;

        map<string, abcrPtr> nameMap;
        map<string, abcrPtr> fullnameMap;
};