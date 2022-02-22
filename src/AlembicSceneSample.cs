using System;
using System.Collections.Generic;
using Stride.Core.Mathematics;
using Stride.Graphics;

namespace Alembic
{
    public sealed partial class AlembicScene
    {
        #region Sampling Geom Schema

        public bool GetPoint(string name, out Vector3[] point, out Matrix transform)
        {
            point = null;
            transform = default;
            AlembicGeom geom = GetGeom(name);

            if(geom.Self != IntPtr.Zero && geom.Type == GeomType.Points)
            {
                point = ((Points)geom).GetSample();
                transform = geom.Transform;

                return true;
            }

            return false;
        }

        public void GetPoints(out IEnumerable<Vector3[]> points, out IEnumerable<Matrix> transforms)
        {
            var pts = new List<Vector3[]>();
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
                points = pts as IEnumerable<Vector3[]>;
                transforms = mats as IEnumerable<Matrix>;
            }
            else
            {
                points = null;
                transforms = null;
            }
        }

        public bool GetMesh(string name, out DataPointer ptr, out VertexDeclaration layout, out Matrix transform)
        {
            ptr = default;
            transform= default;
            layout = default;
            AlembicGeom geom = GetGeom(name);

            if(geom.Self != IntPtr.Zero && geom.Type == GeomType.PolyMesh)
            {
                ptr = ((PolyMesh)geom).GetSample();
                layout = ((PolyMesh)geom).Layout;
                transform = geom.Transform;

                return true;
            }

            return false;
        }

        public void GetMeshes(out IEnumerable<DataPointer> pointers, out IEnumerable<VertexDeclaration> layouts, out IEnumerable<Matrix> transforms)
        {
            var ptrs = new List<DataPointer>();
            var los = new List<VertexDeclaration>();
            var mats = new List<Matrix>();

            bool exist = false;

            foreach(var n in this.Names)
            {
                if(this.GetMesh(n, out var p, out var l, out var t))
                {
                    ptrs.Add(p);
                    los.Add(l);
                    mats.Add(t);
                    exist = true;
                }
            }

            if(exist)
            {
                pointers = ptrs as IEnumerable<DataPointer>;
                layouts = los as IEnumerable<VertexDeclaration>;
                transforms = mats as IEnumerable<Matrix>;
            }
            else
            {
                pointers = null;
                layouts = null;
                transforms = null;
            }
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