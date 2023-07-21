using System;
using System.Collections.Generic;
using Stride.Core.Mathematics;
using Stride.Graphics;

namespace Alembic
{
    public sealed partial class AlembicScene
    {
        #region Sampling Geom Schema

        Dictionary<string, PinnedSequence<Vector3>> _pointsPool = new Dictionary<string, PinnedSequence<Vector3>>();

        public bool GetPoint(string name, out PinnedSequence<Vector3> point, out Matrix transform)
        {
            point = default;
            transform = default;
            AlembicGeom geom = GetGeom(name);

            if(geom.Self != IntPtr.Zero && geom.Type == GeomType.Points)
            {
                var pointGeom = (Points)geom;

                _pointsPool.TryGetValue(name, out point);
                pointGeom.GetSample(ref point);

                _pointsPool[name] = point;

                transform = geom.Transform;

                return true;
            }

            return false;
        }

        public void GetPoints(out IEnumerable<PinnedSequence<Vector3>> points, out IEnumerable<Matrix> transforms)
        {
            var pts = new List<PinnedSequence<Vector3>>();
            var mats = new List<Matrix>();

            bool exist = false;

            foreach(var n in this.Names)
            {
                if(this.GetPoint(n, out var p, out var t))
                {
                    pts.Add(p);
                    mats.Add(t);
                    exist = true;
                }
            }

            if(exist)
            {
                points = pts as IEnumerable<PinnedSequence<Vector3>>;
                transforms = mats as IEnumerable<Matrix>;
            }
            else
            {
                points = null;
                transforms = null;
            }
        }

        public bool GetCurve(string name, out DataPointer curve, out DataPointer indices, out Matrix transform)
        {
            curve = default;
            indices = default;
            transform = default;
            AlembicGeom geom = GetGeom(name);

            if(geom.Self != IntPtr.Zero && geom.Type == GeomType.Curves)
            {
                (curve, indices) = ((Curves)geom).GetSample();
                transform = geom.Transform;

                return true;
            }

            return false;
        }

        public void GetCurves(out IEnumerable<DataPointer> curves, out IEnumerable<DataPointer> indices, out IEnumerable<Matrix> transforms)
        {
            var pts = new List<DataPointer>();
            var inds = new List<DataPointer>();
            var mats = new List<Matrix>();

            bool exist = false;

            foreach(var n in this.Names)
            {
                if(this.GetCurve(n, out var c, out var i, out var t))
                {
                    pts.Add(c);
                    inds.Add(i);
                    mats.Add(t);
                    exist = true;
                }
            }

            if(exist)
            {
                curves = pts as IEnumerable<DataPointer>;
                indices = inds as IEnumerable<DataPointer>;
                transforms = mats as IEnumerable<Matrix>;
            }
            else
            {
                curves = null;
                indices = null;
                transforms = null;
            }
        }

        public bool GetMesh(string name, out DataPointer ptr, out VertexDeclaration layout, out BoundingBox bound, out Matrix transform)
        {
            ptr = default;
            transform= default;
            layout = default;
            bound = default;
            AlembicGeom geom = GetGeom(name);

            if(geom.Self != IntPtr.Zero && geom.Type == GeomType.PolyMesh)
            {
                ptr = ((PolyMesh)geom).GetSample();
                layout = ((PolyMesh)geom).Layout;
                bound = ((PolyMesh)geom).BoundingBox;
                transform = geom.Transform;

                return true;
            }

            return false;
        }

        public void GetMeshes(out IEnumerable<DataPointer> pointers, out IEnumerable<VertexDeclaration> layouts, out IEnumerable<BoundingBox> bounds, out IEnumerable<Matrix> transforms)
        {
            var ptrs = new List<DataPointer>();
            var los = new List<VertexDeclaration>();
            var bds = new List<BoundingBox>();
            var mats = new List<Matrix>();

            bool exist = false;

            foreach(var n in this.Names)
            {
                if(this.GetMesh(n, out var p, out var l, out var b, out var t))
                {
                    ptrs.Add(p);
                    los.Add(l);
                    bds.Add(b);
                    mats.Add(t);
                    exist = true;
                }
            }

            if(exist)
            {
                pointers = ptrs as IEnumerable<DataPointer>;
                layouts = los as IEnumerable<VertexDeclaration>;
                bounds = bds as IEnumerable<BoundingBox>;
                transforms = mats as IEnumerable<Matrix>;
            }
            else
            {
                pointers = null;
                layouts = null;
                bounds = null;
                transforms = null;
            }
        }

        public void GetMeshMaxProperties(string name, out int vertexCount, out BoundingBox boudingBox)
        {
            AlembicGeom geom = GetGeom(name);

            vertexCount = default;
            boudingBox = default;

            if(geom.Type != GeomType.PolyMesh) return;

            vertexCount = NativeMethods.getPolyMeshMaxVertexCount(geom.Self);
            boudingBox = NativeMethods.getPolyMeshMaxSizeBoudingBox(geom.Self);
        }

        public bool GetCamera(string name, out Matrix view, out CameraParam proj)
        {
            view = default;
            proj = default;
            AlembicGeom geom = GetGeom(name);

            if(geom.Self != IntPtr.Zero && geom.Type == GeomType.Camera)
            {
                (view,proj) = ((Camera)geom).GetSample();

                return true;
            }

            return false;
        }

        public void GetCameras(out IEnumerable<Matrix> viewMatrices, out IEnumerable<CameraParam> projectionParams)
        {
            var view = new List<Matrix>();
            var proj = new List<CameraParam>();

            bool exist = false;

            foreach(var n in this.Names)
            {
                if(this.GetCamera(n, out var v, out var p))
                {
                    view.Add(v);
                    proj.Add(p);
                    exist = true;
                }
            }

            if(exist)
            {
                viewMatrices = view as IEnumerable<Matrix>;
                projectionParams = proj as IEnumerable<CameraParam>;
            }
            else
            {
                viewMatrices = null;
                projectionParams = null;
            }
        }

        #endregion // Sampling Geom Schema
    }
}