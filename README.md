# BrogueCE for iOS

An iOS port of [BrogueCE](https://github.com/tmewett/BrogueCE), the community edition of Brogue, a roguelike dungeon crawler.

## Requirements

- macOS with Xcode 14.0 or later
- iOS 14.0+ device or simulator

## Cloning

Clone with the `--recursive` flag to include the BrogueCE submodule:

```bash
git clone --recursive https://github.com/KJ5HST/BrogueCE-iOS.git
```

If you already cloned without `--recursive`, run:

```bash
git submodule update --init --recursive
```

## Building

1. Open `BrogueCE.xcodeproj` in Xcode
2. Select your target device (simulator or physical device)
3. Click **Product > Build** (or press Cmd+B)
4. Click **Product > Run** (or press Cmd+R) to launch

### Building for Physical Device

The included frameworks support both simulator and device builds. For physical devices, you'll need to configure signing:

1. Select the BrogueCE target in Xcode
2. Go to **Signing & Capabilities**
3. Select your development team
4. Xcode will automatically manage signing

## Controls

- **Tap** to move or interact
- **Two-finger tap** to toggle zoom
- **Three-finger tap** for Ctrl modifier
- **D-pad overlay** appears during gameplay for movement

## Credits

- Original Brogue by Brian Walker
- BrogueCE maintained by tmewett and contributors
- iOS port adapted from the Android port

## License

See the BrogueCE submodule for license information.
