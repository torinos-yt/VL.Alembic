using System;
using Microsoft.Win32.SafeHandles;
using VL.Lib.Mathematics;

namespace Alembic
{
    public sealed partial class AlembicScene : SafeHandleZeroOrMinusOneIsInvalid
    {
        public AlembicScene() : base(true) {}

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

            scene._nameArray = new string[scene.Count];
            for(int i = 0; i < scene._nameArray.Length; i++)
            {
                unsafe
                {
                    scene._nameArray[i] = new string(NativeMethods.getName(scene,i));
                }
            }

            return scene;
        }

        public float MinTime => NativeMethods.getMinTime(this);
        public float MaxTime => NativeMethods.getMaxTime(this);
        public Range<float> TimeRange => new Range<float>(MinTime, MaxTime);

        /// <summary>
        /// Number of all Objects in the Archive (e.g. PolyMesh,Xform...)
        /// </summary>
        public int Count => NativeMethods.getGeomCount(this);

        string[] _nameArray = new string[0];
        public string[] Names => _nameArray;

        public GeomType Type(string name) => GetGeom(name).Type;

        public void SetTime(float time) => NativeMethods.updateTime(this, time);


        AlembicGeom GetGeom(string name) => (AlembicGeom)NativeMethods.getGeom(this, name);
    }
}