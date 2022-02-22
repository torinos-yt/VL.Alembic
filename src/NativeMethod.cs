using System;
using System.Runtime.InteropServices;
using Stride.Core.Mathematics;

namespace Alembic
{
    internal static class NativeMethod
    {
        [DefaultDllImportSearchPaths(DllImportSearchPath.AssemblyDirectory)]

        #region AlembicScene

        [DllImport("VL.Alembic.Native.dll")]
        public static extern AlembicScene openScene(string path);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern void closeScene(IntPtr ptr);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern float getMinTime(AlembicScene self);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern float getMaxTime(AlembicScene self);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern int getGeomCount(AlembicScene self);

        [DllImport("VL.Alembic.Native.dll")]
        public unsafe static extern char* getName(AlembicScene self, int index);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern IntPtr getGeom(AlembicScene self, string name);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern void updateTime(AlembicScene self, float time);

        #endregion // AlembicScene


        #region AlembicGeom

        [DllImport("VL.Alembic.Native.dll")]
        public static extern GeomType getType(IntPtr self);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern Matrix getTransform(IntPtr self);

        #endregion // AlembicGeom


        #region Points

        [DllImport("VL.Alembic.Native.dll")]
        public static extern void getPointSample(IntPtr self, IntPtr o);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern int getPointCount(IntPtr self);

        #endregion // Points

        #region PolyMesh

        [DllImport("VL.Alembic.Native.dll")]
        public static extern VertexLayout getPolyMeshLayout(IntPtr self);

        [DllImport("VL.Alembic.Native.dll")]
        public static extern IntPtr getPolyMeshSample(IntPtr self, out int size);

        #endregion // PolyMesh

        #region Camera

        [DllImport("VL.Alembic.Native.dll")]
        public static extern void getCameraSample(IntPtr self, out Matrix v, out CameraParam p);

        #endregion // Camera
    }
}