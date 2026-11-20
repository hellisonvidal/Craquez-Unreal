# Craquez - Asset Import Guide

This document explains how to import and configure the provided assets (balls and pitch) in Unreal Engine 5.4+ without distortion.

## Assets Provided

### 1. Ball Assets (2 versions)
- **Asset/Ball1**: `Meshy_AI_CraquezSoccerBall_20261116_image-to-3d-texture_fbx`
  - FBX: `Meshy_AI_CraquezSoccerBall_20261116_image-to-3d-texture.fbx`
  - Textures: BaseColor, Normal, Roughness, Metallic

- **Asset/Ball2**: `Meshy_AI_Soccer_Ball_Craquez_Remesh_20261115_texture_remesh_tri_fbx`
  - FBX: `Meshy_AI_Soccer_Ball_Craquez_Remesh_20261115_texture_remesh_tri.fbx`
  - Textures: BaseColor, Normal, Roughness, Metallic, MetallicRoughness

### 2. Pitch Asset
- **Asset/Pitch**: `Meshy_AI_Soccer_Training_Ground_3D_20261114_image-to-3d-texture_fbx`
  - FBX: `Meshy_AI_Soccer_Training_Ground_3D_20261114_image-to-3d-texture.fbx`
  - Textures: BaseColor, Normal, Roughness, Metallic

---

## Import Settings - CRITICAL: No Distortion

### FBX Import Settings (All Assets)

```ini
[FBX Import]
- Import Type: Static Mesh (for pitch/ball) or Skeletal Mesh (if animated)
- Import Uniform Scale: 1.0 (CRITICAL - do not scale)
- Convert Scene Unit: True
- Force Front XAxis: False
- Convert Scene: True
- Import Materials: True
- Import Textures: True
- Material Import Method: Create New Materials
- Texture Import Options:
  - sRGB: True for BaseColor/Emissive
  - sRGB: False for Normal/Roughness/Metallic/Masks
  - Compression: Default (BC7 for BaseColor/Normal, BC4 for Roughness/Metallic)
  - Mip Gen Settings: FromTextureGroup
  - LODGroup: TEXTUREGROUP_World (pitch) / TEXTUREGROUP_Character (ball)
  - Max Texture Size: 4096 (or 2048 for mobile)
  - Never Stream: False
  - Virtual Texture Streaming: True (UE5)
```

### Scale Verification
**DO NOT RESCALE** - The assets should be used at their native scale:
- Ball diameter: ~22cm (0.22m) - FIFA standard
- Pitch dimensions: ~105m x 68m - FIFA standard

If the imported mesh appears wrong size:
1. Check FBX units in 3D software (should be centimeters)
2. UE5 import scale = 1.0
3. Apply scale in material UVs if needed, NOT on mesh

---

## Ball Setup

### Ball1 (Original) - Recommended for Gameplay
```
StaticMesh: /Game/Assets/Ball/Ball1_Mesh
Materials:
  - Slot 0: M_Ball_Material (BaseColor, Normal, Roughness, Metallic)
Collision: Simple Sphere (radius 11cm)
Physics: Simulate Physics = False (custom physics in FootballBallComponent)
```

### Ball2 (Remesh) - Higher Quality Visual
```
StaticMesh: /Game/Assets/Ball/Ball2_Mesh
Materials:
  - Slot 0: M_Ball_Material_Remesh
  - Uses MetallicRoughness combined texture
Collision: Same as Ball1
Use: Replay cameras, close-ups, UI
```

### Ball Material Setup
```glsl
// M_Ball_Material
BaseColor: Ball_BaseColor Texture
Normal: Ball_Normal Texture (set to Normal map mode)
Roughness: Ball_Roughness Texture
Metallic: Ball_Metallic Texture
// For Ball2: Use MetallicRoughness texture:
//   R=Metallic, G=Roughness, B=unused, A=unused

// Subsurface for slight translucency
SubsurfaceColor: 0.9, 0.9, 0.9
SubsurfaceAmount: 0.02

// Deformation support (World Position Offset)
CustomNode: DeformationOffset
  Input: ImpactDirection, ImpactStrength
  Output: VertexOffset along ImpactDirection * ImpactStrength
```

---

## Pitch Setup

### Pitch Mesh
```
StaticMesh: /Game/Assets/Pitch/Pitch_Mesh
Scale: 1.0, 1.0, 1.0 (DO NOT CHANGE)
Materials:
  - Slot 0: M_Pitch_Field (Grass + Lines)
  - Slot 1: M_Pitch_Lines (Optional separate lines mesh)
  - Slot 2: M_Pitch_Grass (Detail grass)

Collision:
  - Complex Collision (Per Poly) for accurate ball roll
  - OR: Simple collision boxes for each area
  - Collision Profile: Custom "FootballPitch"
```

### Pitch Dimensions Verification
After import, verify in UE5:
```
Pitch Length (X): 10500 cm (105m) - between goal lines
Pitch Width (Y): 6800 cm (68m) - between touchlines
Center Circle Radius: 915 cm
Penalty Area: 1650cm x 4030cm
Goal Area: 550cm x 1830cm
Penalty Spot: 1100cm from goal line
Corner Arc: 100cm radius
```

### Pitch Materials
- **M_Pitch_Field**: Main material with grass, lines, wear (see M_Pitch_Field.uasset.txt)
- **M_Pitch_Lines**: Separate material for line markings if using separate mesh
- **M_Pitch_Grass**: Detail grass with wind animation

---

## Texture Setup

### Naming Convention
```
T_<AssetName>_<MapType>
Examples:
- T_Ball1_BaseColor
- T_Ball1_Normal
- T_Ball1_Roughness
- T_Ball1_Metallic
- T_Pitch_BaseColor
- T_Pitch_Normal
- T_Pitch_Roughness
- T_Pitch_Metallic
- T_Jersey_Patterns (Atlas)
- T_Fabric_Normal
```

### Texture Compression Settings
| Map Type | sRGB | Compression | Group |
|----------|------|-------------|-------|
| BaseColor | Yes | BC7 / ASTC | World/Character |
| Normal | No | BC5 / BC7 | World/Character |
| Roughness | No | BC4 / BC7 | World/Character |
| Metallic | No | BC4 / BC7 | World/Character |
| MetallicRoughness | No | BC7 | Character |
| Emissive | Yes | BC7 | Effects |
| Masks | No | BC4 | World/Character |

### Virtual Texturing (UE5)
Enable for large textures:
```ini
[/Script/Engine.VirtualTextureSettings]
bEnableVirtualTextures=True
TileSize=128
MaxPhysicalMipCount=3
```

---

## Material Instances for Variants

Create Material Instances for team colors:
```
M_Ball_Material_Instance_TeamA (Blue tint)
M_Ball_Material_Instance_TeamB (Red tint)
M_Pitch_Field_Instance_Day
M_Pitch_Field_Instance_Night
M_Pitch_Field_Instance_Rain
M_PlayerUniform_Instance_Home
M_PlayerUniform_Instance_Away
M_PlayerUniform_Instance_GK_Home
M_PlayerUniform_Instance_GK_Away
```

---

## LOD Setup

### Ball LODs
```
LOD 0: Full detail (close camera, replays)
LOD 1: 50% triangles (mid distance)
LOD 2: 25% triangles (far)
LOD 3: 12% triangles (very far)
Screen Sizes: 1.0, 0.5, 0.25, 0.1
```

### Pitch LODs
```
LOD 0: Full (near player)
LOD 1: 50% (mid field)
LOD 2: 25% (far stands)
Screen Sizes: 1.0, 0.3, 0.1
```

---

## Android Mobile Optimizations

For Android build:
```ini
# Texture Settings
Max Texture Size: 2048
Texture Compression: ASTC 4x4 (LDR) / ASTC 6x6 (HDR)
Mip Gen: SharpenWithoutColorShift

# Material Simplifications
- Disable Tessellation
- Reduce Normal Map strength
- Simplify Subsurface
- Use cheaper BRDF (MobileBurley)

# Mesh
- Reduce LOD count
- Use simpler collision
- Disable complex per-poly collision
```

---

## Verification Checklist

After import, verify:
- [ ] Ball diameter = 22cm in UE5 units
- [ ] Pitch length = 105m between goal lines
- [ ] No stretching/distortion on textures
- [ ] Normals correct (no inverted faces)
- [ ] UVs non-overlapping (for lightmaps)
- [ ] Collision matches visual mesh
- [ ] Materials render correctly in all lighting
- [ ] No Z-fighting on pitch lines
- [ ] Ball physics work with custom component
- [ ] Uniform cloth sim works separate from body

---

## Troubleshooting

### "Ball looks oval"
- Check FBX export scale (should be cm)
- Verify UE5 import scale = 1.0
- Check for non-uniform scale in 3D software

### "Pitch lines don't align"
- Verify UV layout matches material
- Check material UV scale parameters
- Ensure pitch mesh UVs are planar projected

### "Textures look washed out"
- Verify sRGB settings (Off for Normal/Roughness/Metallic)
- Check color space (Linear vs sRGB)
- Verify tone mapper settings

### "Cloth simulation explodes"
- Increase solver iterations
- Check tether constraints to body
- Verify collision thickness
- Reduce time step

---

## Next Steps

1. Import all FBX files with settings above
2. Create materials from .uasset.txt documentation
3. Set up FootballBallComponent with ball meshes
4. Configure pitch with M_Pitch_Field material
5. Set up uniform cloth simulation (separate mesh!)
6. Test in Stadium level
7. Optimize for Android

---

*Generated for Craquez Football Game - UE5.4+*
*Assets: Meshy AI Generated - Use at native scale*