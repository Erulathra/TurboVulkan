module Hash;

public uint WangHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

public float WangHashFloat(uint seed, out uint outSeed)
{
    outSeed = WangHash(seed);
    return outSeed / 0xFFFFFFFF;
}

public float3 RandomColor(uint seed, out uint outSeed)
{
    float3 result;

    outSeed = seed;
    result.x = WangHashFloat(outSeed, outSeed);
    result.y = WangHashFloat(outSeed, outSeed);
    result.z = WangHashFloat(outSeed, outSeed);

    return result;
}