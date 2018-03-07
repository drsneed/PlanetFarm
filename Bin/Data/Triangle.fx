float4 VS( float4 Position : POSITION ) : SV_POSITION
{
    return Position;
}

float4 PS( float4 Position : SV_POSITION ) : SV_Target
{
    return float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1
}
