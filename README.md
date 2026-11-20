# Craquez - Realistic Football Game

A realistic football/soccer game built with Unreal Engine 5.4+, featuring procedural animation, advanced ball physics with Magnus effect, tactical AI, and high-fidelity rendering.

## 🎮 Game Features

### Core Gameplay
- **11v11 Training Match** - Full team vs team with tactical AI
- **1v1 Striker vs Keeper** - Penalty/duel mode
- **Realistic Ball Physics** - Magnus effect, friction, deformation, precise collision
- **Procedural Animation** - Control Rig IK, Motion Matching, Physics-based
- **Tactical AI** - Behavior Trees, StateTree, GOAP planning

### Technical Highlights
- **Deterministic Networking** - Fixed-point math, frame-accurate replication
- **Advanced Rendering** - PBR, Subsurface Scattering, Cloth Simulation, Niagara VFX
- **Procedural Content** - PCG Stadium, Grass, Crowd generation
- **Audio** - Positional stadium audio, crowd chants, dynamic reverb

---

## 🏗️ Project Structure

```
Craquez-Unreal/
├── Craquez.uproject              # Main project file
├── Config/                       # Engine configuration
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   ├── DefaultInput.ini
│   └── DefaultScalability.ini
├── Source/                       # C++ Source Code
│   └── Craquez/                  # Main game module
├── Plugins/
│   └── FootballCore/             # Core football plugin
│       ├── FootballCore.uplugin
│       └── Source/
│           ├── FootballCore/           # Base types, Ball physics
│           ├── FootballCoreGameplay/   # Teams, Players, GameMode
│           ├── FootballCoreAI/         # BehaviorTree, StateTree, GOAP
│           ├── FootballCoreNetworking/ # Deterministic replication
│           └── FootballCoreUI/         # HUD, Widgets, Menus
├── Content/                      # Assets & Blueprints
│   ├── Assets/                   # Imported assets (balls, pitch)
│   ├── Levels/                   # MainMenu, Stadium levels
│   ├── Characters/               # Player characters, uniforms
│   ├── Animations/               # Control Rigs, Motion Matching
│   ├── Materials/                # Pitch, Ball, Uniform, Effects
│   ├── Effects/                  # Niagara systems, Compute Shaders
│   ├── PCG/                      # Procedural Generation Graphs
│   ├── Audio/                    # Crowd, Stadium, Ball sounds
│   └── Blueprints/               # Gameplay Blueprints
├── Build/                        # Platform build scripts
│   └── Android/                  # Android Gradle configuration
├── Documentation/                # Guides and references
│   ├── AssetImportGuide.md
│   └── APIReference.md
└── .gitignore
```

---

## 🔧 Modules Overview

### FootballCore (Base Module)
- **Ball Physics** (`FootballBallComponent`) - Magnus effect, deformation, collision
- **Core Types** - Gameplay tags, enums, structs, fixed-point math
- **Deterministic Math** - Fixed-vector/rotator for network replication

### FootballCoreGameplay
- **Player Character** (`AFootballPlayerCharacter`) - Full player with procedural animation
- **Team System** (`AFootballTeam`) - 11-player teams, formations, tactics
- **Team Manager** (`AFootballTeamManager`) - Match coordination, possession, stats
- **GameMode** (`AFootballGameMode`) - Match phases, rules, scoring
- **Tactical Component** - Team shape, positioning, set pieces

### FootballCoreAI
- **Behavior Tree Tasks** - MoveToBall, Intercept, Pass, Shoot, Tackle, Mark, Press, Support
- **StateTree** - Fixed Press→Intercept transition with proper evaluation
- **GOAP Planner** - Goal-oriented action planning for advanced AI

### FootballCoreNetworking
- **Deterministic Replication** - Fixed-timestep, state history, rollback
- **Network Prediction** - Client-side prediction with reconciliation
- **Lag Compensation** - Server-side rewind for fair hits
- **Blueprint Interface** - `IFootballDeterministicInterface`

### FootballCoreUI
- **HUD** (`AFootballHUD`) - Main HUD controller
- **Match Widget** - Score, time, possession, events
- **Player Widget** - Stamina, fatigue, action feedback
- **Tactical Widget** - Formation, instructions, player positions
- **Replay Widget** - Timeline, camera modes, analysis tools
- **Menu Widget** - Main/Pause/Settings/Team/Tactics/Replay menus

---

## 🎨 Assets & Visuals

### Provided Assets (in `Content/Assets/`)
- **Ball1** - `Meshy_AI_CraquezSoccerBall_20261116` (Gameplay ball)
- **Ball2** - `Meshy_AI_Soccer_Ball_Craquez_Remesh` (High-quality visual)
- **Pitch** - `Meshy_AI_Soccer_Training_Ground_3D_20261114` (Stadium pitch)

### Materials
- `M_Pitch_Field` - Grass with lines, wear, wind animation
- `M_PlayerUniform` - Team colors, numbers, names, sponsors, cloth physics
- `M_Ball_Material` - Ball with deformation support
- `M_CrowdAgent` - Team-colored crowd agents

### Cloth Simulation
- **UniformClothAsset** - Chaos Cloth setup for jerseys/shorts (SEPARATE from body)
- Vertex groups: BodyAttached, FreeHanging, SleeveCuff, Collar
- Body collision capsules matching skeleton

---

## 🤖 AI Systems

### Behavior Tree Tasks
| Task | Description |
|------|-------------|
| `UBTTask_MoveToBall` | Move to ball with configurable acceptance radius |
| `UBTTask_InterceptBall` | Predict and intercept ball trajectory |
| `UBTTask_PassToTeammate` | Find best pass target using tactical analysis |
| `UBTTask_ShootAtGoal` | Shoot with power/placement/chip options |
| `UBTTask_TackleOpponent` | Standing/sliding tackle with success chance |
| `UBTTask_MarkOpponent` | Man-to-man marking with goal-side/ball-side |
| `UBTTask_MoveToFormationPosition` | Maintain tactical formation position |
| `UBTTask_PressBallCarrier` | Press from angle forcing weak foot |
| `UBTTask_SupportBallCarrier` | Provide passing options |
| `UBTTask_GoalkeeperSave` | Dive/catch/punch with reaction time |
| `UBTTask_GoalkeeperDistribute` | Throw/kick distribution |

### StateTree (Fixed Press→Intercept)
- **Press State** - Jockeying, positioning at angle
- **Intercept State** - Sprint to predicted intercept point
- **Transition Fixed** - Uses `UInterceptionEvaluator` with confidence scoring

### GOAP Planner
- **Actions**: Pass, Shoot, Dribble, Intercept, Tackle, Move, Press, Support, Mark, Counter
- **Goals**: Score, CreateChance, Possession, RegainPossession, Defend, HoldPosition, CounterAttack
- **A* Search** with heuristic for optimal plans

---

## 🌐 Networking (Deterministic)

### Key Features
- **Fixed Timestep** - 60Hz simulation (1/60s)
- **Fixed-Point Math** - `FFootballFixedVector`, `FFootballFixedRotator`
- **State History** - 120 frames (2 seconds) for rollback
- **Client Prediction** - Local simulation with server reconciliation
- **Lag Compensation** - Server rewind up to 500ms
- **Blueprint Interface** - `GetDeterministicState`, `ApplyDeterministicState`

### Replication Flow
```
Server: Simulate @ 60Hz → Capture State → Send to Clients
Client: Receive State → Compare → Reconcile/Interpolate → Predict Next Frames
```

---

## 📱 Android Build

### Configuration (`Build/Android/`)
- `build.gradle` - Gradle 8.1, Kotlin 1.9, NDK r25c
- `settings.gradle` - Plugin management
- `gradle.properties` - JVM args, parallel builds, caching
- `AndroidManifest.xml` - Permissions, activities, features
- **Target**: Android 7.0+ (API 24), ARM64, Vulkan/OpenGL ES 3.2

### Mobile Optimizations
- Texture compression: ASTC 4x4/6x6
- Max texture size: 2048
- Disabled: Tessellation, complex collision, high-quality shadows
- Material quality: MobileBurley BRDF
- LOD bias: +1 for skeletal meshes

---

## 🚀 Getting Started

### Prerequisites
- Unreal Engine 5.4+
- Visual Studio 2022 (Windows) / Xcode (Mac)
- Android Studio + NDK r25c (for Android)
- Git LFS (for large assets)

### Setup
```bash
# Clone repository
git clone https://github.com/hellisonvidal/Craquez-Unreal.git
cd Craquez-Unreal

# Initialize Git LFS
git lfs install
git lfs pull

# Open in UE5
# Double-click Craquez.uproject or use Epic Games Launcher
```

### First Run
1. Open project in UE5.4+
2. Wait for C++ compilation (FootballCore plugin)
3. Open `Content/Levels/Craquez_MainMenu.umap`
4. Press Play - Main Menu should appear
5. Click "Start Match" to load Stadium level

---

## 📖 Documentation

- [Asset Import Guide](Documentation/AssetImportGuide.md) - How to import balls/pitch without distortion
- [API Reference](Documentation/APIReference.md) - C++ class hierarchy and Blueprint APIs

---

## 🎯 Development Roadmap

### Phase 1 (Current) - Core Systems ✅
- [x] Project structure & FootballCore plugin
- [x] Ball physics with Magnus effect
- [x] Player character with procedural animation
- [x] Team/GameMode systems
- [x] Behavior Tree AI tasks
- [x] StateTree with fixed transitions
- [x] GOAP planner
- [x] Deterministic networking
- [x] UI/HUD system
- [x] Asset import pipeline

### Phase 2 - Visual Polish
- [ ] MetaHuman integration for players
- [ ] Full stadium with PCG generation
- [ ] Crowd system with chants
- [ ] Weather system (rain, fog, snow)
- [ ] Replay system with analysis tools
- [ ] Broadcast camera system

### Phase 3 - Gameplay Depth
- [ ] Career mode
- [ ] Tournament system
- [ ] Player development
- [ ] Tactics editor
- [ ] Set piece creator
- [ ] Commentary system

### Phase 4 - Multiplayer
- [ ] Online matchmaking
- [ ] Dedicated server support
- [ ] Cross-platform play
- [ ] Replay sharing
- [ ] Spectator mode

### Phase 5 - Platforms
- [ ] iOS build
- [ ] Console (PS5, Xbox Series X)
- [ ] PC Steam release
- [ ] Cloud gaming support

---

## 🤝 Contributing

1. Fork the repository
2. Create feature branch: `git checkout -b feature/amazing-feature`
3. Commit changes: `git commit -m 'Add amazing feature'`
4. Push to branch: `git push origin feature/amazing-feature`
5. Open Pull Request

### Code Style
- Follow Unreal Engine coding standards
- Use `PascalCase` for classes, `camelCase` for variables
- Document public APIs with `///` comments
- Keep functions small and focused

---

## 📄 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **Unreal Engine** by Epic Games
- **Meshy AI** for generated assets (balls, pitch)
- **Chaos Physics** for cloth/destruction
- **Niagara** for VFX
- **PCG** for procedural generation
- **Gameplay Ability System** for abilities

---

## 📞 Contact

- **Repository**: https://github.com/hellisonvidal/Craquez-Unreal
- **Issues**: GitHub Issues
- **Discussions**: GitHub Discussions

---

*Built with ❤️ using Unreal Engine 5.4+*
*Craquez - Where Football Meets Simulation*