# BrogueCE iOS - Xcode Setup Guide

This guide walks you through setting up the Xcode project to build BrogueCE for iOS.

## Prerequisites

- Xcode 14+ installed
- iOS 14.0+ SDK
- Apple Developer account (for device testing)

## Project Structure

```
BrogueCE-iOS/
├── src/
│   ├── brogue/          -> symlink to BrogueCE source
│   ├── variants/        -> symlink to BrogueCE variants
│   └── platform/        -> iOS platform implementation
│       ├── main.c
│       ├── display.c
│       ├── input.c
│       ├── config.c
│       └── include/
├── libs/
│   ├── SDL2/            -> SDL2 source
│   └── SDL2_ttf/        -> SDL2_ttf source
├── assets/              -> Game assets (tiles, fonts)
└── Info.plist
```

## Setup Steps

### 1. Create New Xcode Project

1. Open Xcode
2. File → New → Project
3. Select "iOS" → "App"
4. Product Name: `BrogueCE`
5. Language: **Objective-C**
6. Interface: **Storyboard**
7. Save to: `BrogueCE-iOS/` directory

### 2. Clean Up Generated Files

Delete these auto-generated files:
- `main.m`
- `AppDelegate.h` / `AppDelegate.m`
- `SceneDelegate.h` / `SceneDelegate.m`
- `ViewController.h` / `ViewController.m`
- `Main.storyboard`

Keep:
- `Assets.xcassets`
- `LaunchScreen.storyboard`

### 3. Configure Info.plist

1. Select the project in navigator
2. Select target → Info tab
3. Under "Custom iOS Target Properties":
   - Delete "Main storyboard file base name"
   - Delete "Application Scene Manifest" (entire section)

Or replace with the provided `Info.plist`.

### 4. Add SDL2 as Subproject

1. Right-click on the project → "Add Files to BrogueCE..."
2. Navigate to `libs/SDL2/Xcode/SDL/SDL.xcodeproj`
3. Add it

### 5. Add SDL2_ttf as Subproject

1. Right-click on the project → "Add Files to BrogueCE..."
2. Navigate to `libs/SDL2_ttf/Xcode/SDL_ttf.xcodeproj`
3. Add it

### 6. Add Source Files

Add these folders/files to the project:

**Platform Code:**
- `src/platform/main.c`
- `src/platform/display.c`
- `src/platform/input.c`
- `src/platform/config.c`

**BrogueCE Source (from symlinked folder):**
- All `.c` files from `src/brogue/`
- All `.c` files from `src/variants/`

**SDL2 UIKit Main:**
- Drag `SDL_uikit_main.c` from SDL2 subproject:
  `SDL.xcodeproj → Library Source → main → uikit → SDL_uikit_main.c`

### 7. Configure Header Search Paths

1. Select project → Build Settings → All
2. Find "Header Search Paths"
3. Add these paths:
   ```
   $(SRCROOT)/src/brogue
   $(SRCROOT)/src/platform/include
   $(SRCROOT)/src/variants
   $(SRCROOT)/libs/SDL2/include
   $(SRCROOT)/libs/SDL2_ttf
   ```

### 8. Link Frameworks

1. Select target → Build Phases → Link Binary With Libraries
2. Add from SDL2.xcodeproj:
   - `SDL2.framework` (Framework-iOS target)
3. Add from SDL_ttf.xcodeproj:
   - `SDL2_ttf.framework` (iOS target)
4. Add system frameworks:
   - `AudioToolbox.framework`
   - `AVFoundation.framework`
   - `CoreGraphics.framework`
   - `CoreMotion.framework`
   - `Foundation.framework`
   - `GameController.framework`
   - `Metal.framework`
   - `OpenGLES.framework`
   - `QuartzCore.framework`
   - `UIKit.framework`

### 9. Embed Frameworks

1. Select target → General → Frameworks, Libraries, and Embedded Content
2. Set SDL2.framework to "Embed & Sign"
3. Set SDL2_ttf.framework to "Embed & Sign"

### 10. Add Resources

1. Right-click project → "Add Files to BrogueCE..."
2. Add `assets/` folder
3. Make sure "Copy items if needed" is checked
4. Select "Create folder references"

### 11. Add Font File

You need a TTF font file. Options:
- Copy `default.ttf` from Android port (uses modified DejaVu Sans Mono)
- Or use any monospace TTF font

Place it in `assets/` as `default.ttf`.

### 12. Build Settings

1. Select project → Build Settings
2. Set "iOS Deployment Target" to 14.0 or later
3. Under "Apple Clang - Language - C":
   - Set "C Language Dialect" to "C99 [-std=c99]"

### 13. Build and Run

1. Select an iOS Simulator (iPhone 14 Pro recommended)
2. Product → Build (⌘B)
3. Product → Run (⌘R)

## Troubleshooting

### "SDL.h not found"
- Verify Header Search Paths include SDL2/include directory
- Make sure paths are marked as "recursive" if needed

### "Undefined symbols for architecture"
- Ensure all frameworks are linked
- Check that SDL2.framework and SDL2_ttf.framework are embedded

### Font loading fails
- Verify `default.ttf` is in the app bundle
- Check that assets folder was added with "Create folder references"

### Black screen on launch
- Check console for SDL errors
- Verify SDL_uikit_main.c is included in the build

## Controls

- **D-Pad**: On-screen directional pad (bottom-left)
- **Long-press D-pad**: Toggle between movement/selection mode
- **Top-left corner**: Enter key
- **Bottom-left corner**: Escape key
- **Left edge**: Open keyboard
- **Two-finger pinch**: Zoom in/out
- **Two-finger tap**: Toggle zoom

## Files Reference

| File | Purpose |
|------|---------|
| main.c | Entry point, SDL init, game loop |
| display.c | Rendering, font handling |
| input.c | Touch and keyboard input |
| config.c | Settings management |
