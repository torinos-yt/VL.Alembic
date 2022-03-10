using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;

namespace Alembic
{
    // Array pool for AlembicPoint to avoid large array reallocate.
    public struct PinnedSequence<T> : IDisposable, IEnumerable<T>
        where T : struct
    {
        GCHandle _handle;

        T[] _array;
        int _length;
        int _capacity;

        public PinnedSequence(int size)
        {
            _array = new T[size];
            _length = _capacity = size;

            _handle = GCHandle.Alloc(_array, GCHandleType.Pinned);
        }

        public void Resize(int size)
        {
            if(_capacity >= size)
            {
                Console.WriteLine("No Alloc");
                _length = size;
                return;
            }
                Console.WriteLine("Alloc");

            int newSize = Math.Max(size, _capacity * 2);

            if(_handle.IsAllocated) _handle.Free();
            _array = new T[newSize];

            _handle = GCHandle.Alloc(_array, GCHandleType.Pinned);

            _capacity = newSize;
            _length = size;
        }

        public T this[int i] => _array[i];
        public IntPtr Ptr => _array.Length == 0 ? IntPtr.Zero : _handle.AddrOfPinnedObject();
        public int Count => _length;
        public int Stride => Marshal.SizeOf(typeof(T));

        public void Dispose()
        {
            if(_handle.IsAllocated) _handle.Free();
            GC.SuppressFinalize(this);
        }


        #region IEnumerable impl
        
        IEnumerator<T> IEnumerable<T>.GetEnumerator() => new Enumerator(_array, _length);
        IEnumerator IEnumerable.GetEnumerator() => new Enumerator(_array, _length);

        public struct Enumerator : IEnumerator<T>
        {
            int _index;
            T[] _array;
            int _length;

            internal Enumerator(T[] array, int length) => (_array, _length, _index) = (array, length, -1);

            public T Current => _array[_index];
            object IEnumerator.Current => Current;
            public bool MoveNext() => ++_index < _length;
            public void Reset() => _index = -1;
            public void Dispose() => _array = null;
        }

        #endregion // IEnumerable impl
    }
}