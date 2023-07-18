using System;
using Microsoft.Win32.SafeHandles;
using Stride.Core.Mathematics;
using VL.Lib.Mathematics;

namespace Alembic
{
    public sealed partial class AlembicScene : SafeHandleZeroOrMinusOneIsInvalid
    {
        public AlembicScene() : base(true) {}

        static void debugLogFunc(string str) => Console.WriteLine(str);

        static AlembicScene()
        {
            NativeMethods.RegisterDebugFunc(debugLogFunc);
        }

        protected override bool ReleaseHandle()
        {
            NativeMethods.closeScene(handle);
            return true;
        }

        public static AlembicScene Open(string path)
        {
            if(path == string.Empty) return null;

            var scene = NativeMethods.openScene(path);
            
            if(scene.handle == IntPtr.Zero)
                throw new FormatException("Failed Open : Illigal Format");

            scene._nameArray = new string[scene.ObjectCount];
            for(int i = 0; i < scene._nameArray.Length; i++)
            {
                unsafe
                {
                    scene._nameArray[i] = new string(NativeMethods.getName(scene,i));
                    scene.minTime = Math.Min(NativeMethods.getGeomMinTime(scene.GetGeom(scene._nameArray[i]).Self), scene.minTime);
                    scene.maxTime = Math.Max(NativeMethods.getGeomMaxTime(scene.GetGeom(scene._nameArray[i]).Self), scene.maxTime);
                }
            }

            return scene;
        }

        float minTime = float.PositiveInfinity;
        float maxTime = 0;

        public float MinTime => Math.Min(NativeMethods.getMinTime(this), minTime);
        public float MaxTime => Math.Max(NativeMethods.getMaxTime(this), maxTime);
        public Range<float> TimeRange => new Range<float>(MinTime, MaxTime);

        /// <summary>
        /// Number of all Objects in the Archive (e.g. PolyMesh,Xform...)
        /// </summary>
        public int ObjectCount => NativeMethods.getGeomCount(this);

        string[] _nameArray = new string[0];
        public string[] Names => _nameArray;

        public GeomType Type(string name) => GetGeom(name).Type;

        public Matrix Transform(string name) => NativeMethods.getTransform(GetGeom(name).Self);

        public void SetTime(float time) => NativeMethods.updateTime(this, time);

        public void SetInterpolate(bool interpolate) => NativeMethods.setInterpolate(this, interpolate);


        AlembicGeom GetGeom(string name) => (AlembicGeom)NativeMethods.getGeom(this, name);
    }
}