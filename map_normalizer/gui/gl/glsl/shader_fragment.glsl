
// Note that this code _will not work as is_

uniform uint currentTerrainType;

usampler2D province;
sampler2D tileTexture;

usampler1D provinceInfos;

vec2 textureCoords = /* calculate from vertex pos */;
uvec4 sample = texture(province, textureCoords);
uint provinceid = sample.r << 16 + sample.g << 8 + sample.b;
bool isBorder = sample.a > 0;

uvec4 l_provinceInfo = texelFetch(provinceInfos, provinceid);

uint terrainType = l_provinceInfo.x;
uvec3 prov_color = l_provinceInfo.yzw;

if(terrainType != currentTerrainType) {
    discard;
}

if(isBorder) {
    color = black;
    return;
}

