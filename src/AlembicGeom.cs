using System;
using System.Runtime.InteropServices;
using Stride.Core.Mathematics;
using Stride.Graphics;

namespace Alembic
{
    public enum GeomType
    {
        Points = 0,
        Curves,
        PolyMesh,
        Camera,
        Xform,
        Unknown
    }

    internal abstract class GeomPtr
    {
        protected IntPtr self;
        public IntPtr Self => self;
    }

    internal class AlembicGeom : GeomPtr
    {
        public AlembicGeom(IntPtr geom) { self = geom; }

        public GeomType Type => NativeMethods.getType(self);
        public Matrix Transform => NativeMethods.getTransform(self);

        public static explicit operator AlembicGeom(IntPtr ptr) => new AlembicGeom(ptr);
    }

    internal class Points : GeomPtr
    {
        public Points(IntPtr ptr) { self = ptr; }
        public Points(AlembicGeom geom) { self = geom.Self; }

        public Vector3[] GetSample()
        {
            var count = NativeMethods.getPointCount(this.self);

            var o = new Vector3[count];
            GCHandle handle = GCHandle.Alloc(o, GCHandleType.Pinned);

            NativeMethods.getPointSample(this.self, handle.AddrOfPinnedObject());

            handle.Free();

            return o;
        }

        public static explicit operator Points(AlembicGeom geom) => new Points(geom);
    }

    internal class Curves : GeomPtr
    {
        public Curves(IntPtr ptr) { self = ptr; }
        public Curves(AlembicGeom geom) { self = geom.Self; }

        public static explicit operator Curves(AlembicGeom geom) => new Curves(geom);
    }

    internal enum VertexLayout
    {
        PosNormTex = 0,
        PosNormColTex,
        Unknown
    }

    internal class PolyMesh : GeomPtr
    {
        public PolyMesh(IntPtr ptr) { self = ptr; }
        public PolyMesh(AlembicGeom geom) { self = geom.Self; }

        public VertexDeclaration Layout
        {
            get
            {
                VertexLayout l = NativeMethods.getPolyMeshLayout(this.self);

                switch (l)
                {
                    case VertexLayout.PosNormTex :
                        return new VertexDeclaration(VertexElement.Position<Vector3>(),
                                        VertexElement.Normal<Vector3>(),
                                        VertexElement.TextureCoordinate<Vector2>());
                    case VertexLayout.PosNormColTex :
                        return new VertexDeclaration(VertexElement.Position<Vector3>(),
                                        VertexElement.Normal<Vector3>(),
                                        VertexElement.Color<Vector4>(),
                                        VertexElement.TextureCoordinate<Vector2>());
                    case VertexLayout.Unknown :
                    default :
                        throw new InvalidOperationException();
                }
            }
        }

        public DataPointer GetSample()
        {
            var ptr = NativeMethods.getPolyMeshSample(this.self, out var size);
            if(ptr == IntPtr.Zero || size <= 0)
                throw new InvalidOperationException();

            return new DataPointer(ptr, size);
        }

        public static explicit operator PolyMesh(AlembicGeom geom) => new PolyMesh(geom);
    }

    public readonly struct CameraParam
    {
        public readonly float Aperture, Near, Far, FocalLength, FoV;
    }

    internal class Camera : GeomPtr
    {
        public Camera(IntPtr ptr) { self = ptr; }
        public Camera(AlembicGeom geom) { self = geom.Self; }

        public (Matrix, CameraParam) GetSample()
        {
            NativeMethods.getCameraSample(this.self, out var v, out var p);
            return (v,p);
        }

        public static explicit operator Camera(AlembicGeom geom) => new Camera(geom);
    }

    internal class Xform : GeomPtr
    {
        public Xform(IntPtr ptr) { self = ptr; }
        public Xform(AlembicGeom geom) { self = geom.Self; }

        public static explicit operator Xform(AlembicGeom geom) => new Xform(geom);
    }
}