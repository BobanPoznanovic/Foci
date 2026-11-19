# Foci

An Unreal Engine 5.5 C++ project pre-configured with the **UnrealMCP** (`Model Context Protocol`) C++ Editor plugin and Enhanced Input gameplay framework classes.

## Core C++ Classes (`Source/Foci/`)
- **`AFociGameMode`** (`FociGameMode.h` / `.cpp`): Default GameMode configuring `AFociCharacter` and `AFociPlayerController`.
- **`AFociCharacter`** (`FociCharacter.h` / `.cpp`): Third-person playable character with `USpringArmComponent`, `UCameraComponent`, and Enhanced Input bindings (`MoveAction`, `LookAction`, `JumpAction`).
- **`AFociPlayerController`** (`FociPlayerController.h` / `.cpp`): PlayerController registering the Enhanced Input `DefaultMappingContext` on `BeginPlay`.

## UnrealMCP Plugin (`Plugins/UnrealMCP/`)
- Runs a TCP command server inside the Unreal Editor (`127.0.0.1:55557`) so AI assistants can create/modify Actors, Blueprints, Node Graphs, and UMG widgets via the Python MCP server (`unreal-mcp-server`).

## macOS Installer (`Packaging/Mac/`)
- Run `./Packaging/Mac/build_and_package.sh --version 1.0.0` on macOS (with Unreal Engine 5.5 + Xcode) to package `Foci.app` and generate `.pkg` and `.dmg` installers in `Dist/MacInstaller/`.
- See [Packaging/Mac/README.md](Packaging/Mac/README.md) for code signing, notarization, and standalone `.app` packaging options.
