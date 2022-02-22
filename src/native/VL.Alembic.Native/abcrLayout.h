#pragma once

struct VertexPositionNormalTexture
{
public:
    V3f Position;
    N3f Normal;
    V2f TexureCoordinate;
        
    const static int VertexSize()
    {
        return sizeof(VertexPositionNormalTexture);
    }
};

struct VertexPositionNormalColorTexture
{
public:
    V3f Position;
    N3f Normal;
    C4f Color;
    V2f TexureCoordinate;

    const static int VertexSize()
    {
        return sizeof(VertexPositionNormalColorTexture);
    }
};

enum VertexLayout
{
    PosNormTex = 0,
    PosNormColTex,
    Unknown
};

// TODO : calc Tangents at runtime
// https://github.com/stride3d/stride/blob/master/sources/engine/Stride.Graphics/VertexHelper.cs#L219-L393

#define TANGENT 4 // (xyz:tangent, w:handed)