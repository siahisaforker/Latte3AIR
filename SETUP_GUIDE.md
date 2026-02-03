# Sonic 3 A.I.R. Wii U Setup Guide

## 🎮 Required File Structure

### SD Card Setup
Create this exact structure on your Wii U SD card:

```
/vol/external01/sonic3air/
├── rom/
│   └── Sonic_Knuckles_wSonic3.bin
├── save/     (auto-created)
├── config/   (auto-created)
└── mods/     (optional)
```

## 📁 ROM File

**Required ROM Name:** `Sonic_Knuckles_wSonic3.bin`

**Place it in:** `/vol/external01/sonic3air/rom/`

## 🚀 Installation Steps

1. **Copy the .rpx file** to your Wii U (Aroma)
2. **Create the folder structure** on your SD card
3. **Copy your ROM file** to the correct location
4. **Launch Sonic 3 A.I.R.** from Aroma

## 🎯 What the Game Expects

- **ROM Path:** `/vol/external01/sonic3air/rom/Sonic_Knuckles_wSonic3.bin`
- **Save Path:** `/vol/external01/sonic3air/save/`
- **Config Path:** `/vol/external01/sonic3air/config/`

## ⚠️ Important Notes

- The ROM file **must** be named exactly `Sonic_Knuckles_wSonic3.bin`
- The game will **not work** without the ROM file
- Save and config folders are created automatically
- Mods folder is optional for additional content

## 🔧 Troubleshooting

**Game crashes on startup:**
- Check that `Sonic_Knuckles_wSonic3.bin` exists in the correct location
- Verify the ROM file is not corrupted
- Ensure SD card is properly formatted

**No video output:**
- Make sure the ROM file is present
- Check that the folder structure is correct

## 📱 Testing

The current test version shows:
- **Red screen**: ROM not found or error state
- **Blue screen**: App is running but waiting for ROM
- **Green screen**: ROM found and game loading

Once you have the proper ROM file in place, the game should load successfully!
