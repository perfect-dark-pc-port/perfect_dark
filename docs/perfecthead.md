# PerfectHead

PerfectHead is a feature Rare cut from Perfect Dark shortly before release. You
take a photo with a Game Boy Camera plugged into the N64 through a Transfer
Pak, put it on a head, edit the head, save it to a Controller Pak and play as
it, or against it, in the Combat Simulator and the solo missions. Most of the
code and all of the menu text survived in the released game. This restores
the rest, for the N64 ROM and the PC port.

Everything is behind the `PERFECTHEAD` compile flag (`src/include/versions.h`,
off by default), so builds without it are unchanged and matching builds of the
original ROMs stay byte-identical.

The same game code lives in two repositories:

| Repository | Builds | PerfectHead specifics |
|---|---|---|
| PC port (`perfect_dark`, this document's home) | PC executables (CMake) | Virtual Game Boy Camera fed by the PC's webcam, virtual Controller Pak, hair mask export |
| N64 decompilation (`perfect_dark_n64`) | N64 ROMs (`make`) | Runs on real hardware and accurate emulators |

## Building and setting up

### N64 ROM

In the decomp repository, with your own ROM extracted (`make extract`):

```bash
make MATCHING=0 COMPILER=gcc PERFECTHEAD=1
```

The Makefile only supports gcc for ntsc-final and ntsc-1.0. With PerfectHead
off, a plain `make` still builds the retail ROM byte for byte.

On the console PerfectHead needs:

- the **Expansion Pak** (the PerfectHead entry is hidden without it, as the
  camera engine only allocates its state with 8 MB);
- a **Transfer Pak** with a **Game Boy Camera** in any controller;
- a **Controller Pak** in any controller, to save heads.

### PC port

```bash
cmake -G"Unix Makefiles" -Bbuild -DPERFECTHEAD=ON .
cmake --build build
```

Settings in `pd.ini`:

| Section | Key | Default | Meaning |
|---|---|---|---|
| `[PerfectHead]` | `VirtualCameraController` | 0 | Controller (1-4) with a virtual Transfer Pak and Game Boy Camera; 0 = none. That controller loses rumble, as on the console. |
| `[PerfectHead]` | `CameraSource` | 1 | 1 = the PC's camera if there is one, 0 = always the test card |
| `[PerfectHead]` | `CameraDevice` | 0 | Which camera to use (0 = the first) |
| `[PerfectHead]` | `CameraMirror` | 1 | 1 = flip left/right like a mirror |
| `[PerfectHead]` | `SDL3Library` | (empty) | Full path to the SDL3 library, if it isn't found by name |
| `[Input.PlayerN]` | `ControllerPak` | 0 | 1 = controller N has a virtual Controller Pak (instead of rumble) |
| `[Input]` | `FakeGamepads` | 0 | Extra virtual controllers, to plug a Controller Pak into when you have one pad |

A typical setup is the camera on controller 1 and a Controller Pak on
controller 2: `VirtualCameraController=1`, `[Input.Player2] ControllerPak=1`,
and `FakeGamepads=2` with a single pad.

The webcam is used through SDL3, which is loaded at run time and is not needed
to build. Without SDL3 the camera shows an animated test card. The README has
install notes for each OS. On macOS an app bundle needs
`NSCameraUsageDescription` in its Info.plist for the permission prompt; a
binary run from a terminal uses the terminal's permission.

Files in the save folder:

| File | What |
|---|---|
| `controllerpak<N>.mpk` | Controller N's virtual Controller Pak: a standard 32 KB `.mpk` image, interchangeable with emulators and dumps of real paks |
| `gbcam.sav` | The virtual camera's 128 KB save RAM. Drop in a save dumped from a real Game Boy Camera to use its photos with Load From Camera. |

Command-line options (testing aids): `--phead-menu` opens PerfectHead from
the main menu, and `--phead-texdump` exports the base heads' textures for the
hair mask editor (see [Base heads](#base-heads-where-they-are-and-how-to-adjust-them)).

### Compatible cameras

The camera is recognised by its header: Nintendo's licensee code with a title
starting `GAMEBOYCAMERA` or `POCKETCAMERA` (the check the NTSC beta had), or
any cartridge whose type byte is `0xFC`, the camera's own MAC-GBD mapper,
whatever its title. That covers:

- Game Boy Camera (USA, Europe) and Game Boy Camera Gold (Zelda edition);
- Pocket Camera (Japan) and its revision;
- the Hello Kitty Pocket Camera and Debug Game Tester prototypes;
- camera cartridges reflashed with homebrew camera software (Photo!,
  2bit PXLR Studio).

## Using it

Combat Simulator → Player Setup → **PerfectHead**:

- **Load A Saved Head**, **Make A New Head** and **Edit A Loaded Head** (once
  one is loaded). Up to four heads are loaded at a time; with all four in use,
  you choose one to replace.
- Make and Load first ask **"Where do you want to be able to use this
  head?"**: Combat Simulator, All Missions or Both.
- **Camera Setup** searches the controllers for the camera, then **Take
  Picture** (live preview with a placement guide) or **Load From Camera** (the
  camera's photo album).
- The **PerfectHead Editor** shows the head in 3D, bobbing: Take Another
  Picture, Change Head (the base head), Position Picture (crop and
  brightness), Color Face & Hair, Shape Head (depth), Remove Head, Cancel
  Changes and Keep Changes.
- **Keep Changes** offers to save the head to a Controller Pak, then **Head
  Complete!** says where the head can now be found.
- **B** in the editor, after changing a head that was already loaded, asks
  **"You have edited this head. Do you want to keep your changes?"**: Keep
  (as Keep Changes) or Revert (as Cancel Changes). B there goes back to the
  editor. With no changes, or for a new head, B just leaves.

Combat Simulator heads appear after the normal heads in the Character menu of
Player Setup and of each Simulant. Mission heads replace some of the guards'
heads in solo missions.

## What survived in the released game

| Area | Where | State in the released game |
|---|---|---|
| Camera engine: image buffers, 8x8 DCT compression, skin tones and hair colours, thumbnails, Controller Pak load/save, the album reader | `src/game/camdraw.c` | Present and compiled, mostly unnamed (`func0f14xxxx`). Nothing drove it: the menus were gone. |
| Game Boy Camera registers, capture timing, 2bpp tile conversion, dither matrix | `src/game/pak.c` | Present |
| Camera detection and the per-frame capture state machine | `src/game/pak.c` (`gbpakIdentifyGame`, `pak0f117f94nb`, `pak0f116d1cnb`, `gbpak0f1172c8nb`, `PAKSTATE_12`) | NTSC beta only; stubbed out in 1.0 and final |
| Camera files on the Controller Pak | `PAKFILETYPE_CAMERA`, `_pakCreateCameraFile`, `filelist.c`, `filemgr.c` | Present, except the file manager's save/load cases |
| PerfectHeads in the Combat Simulator | `mplayer.c`, `setup.c`, `player.c`, `menu.c` | Wired up (head numbers past the normal heads), but the carousel count left them out |
| Mission heads | `body.c`, `bodyreset.c`, `mplayer.c` | Negative head numbers and a per-stage model table remain; nothing filled them |
| Menu text | `src/assets/*/lang/mpmenu.json`, `L_MPMENU_294`-`414`, and the camera banners | Present in every version and language |
| Menu dialogs | n/a | Missing: rebuilt from the text |
| Base head models (`HEAD_BEAU2`-`6`) and the code that put a picture on a head (`Cam_RebuildHead`, `Cam_ColourHeadOnePart`, `Cam_SquashOnePart`, `Cam_AllocAndCopyAllVtx`) | `g_MpBeauHeads`, camdraw's debug strings | Missing: the heads all alias the Beau guard head and no head has the parts camdraw expected. Rebuilt as a repaint of existing heads. |

## How it works

### Camera detection and capture (`pak.c`)

- Under `PERFECTHEAD`, 1.0 and final compile the beta's capture code. The
  capture runs a step per frame: the menu moves the pak from `READY` to
  `PAKSTATE_12`, `pakTickState` runs one step of `pak0f117f94nb` and
  `joy00014238` moves it back to `READY`. The picture arrives in 0x400-byte
  blocks over several frames.
- The camera's registers follow the Pan Docs layout: RAM bank `0x10` written
  to `0x4000` maps them at `A000` (capture trigger and busy flag), `A001`-`A005`
  (sensor) and a 4x4x3 dither matrix (`pak0f11d214`). The picture is read from
  SRAM bank 0 at `A100`.
- The beta compared the camera's licensee against `ROM_COMPANYCODE`, which was
  `0x3031` ("01", Nintendo) only in the beta, so the restored check uses
  `GBCAMERA_COMPANYCODE` explicitly. See [Compatible cameras](#compatible-cameras)
  for the rest of the check.
- `gbpakIdentifyGame`'s result is stored in a 1-bit field, so the camera
  returns `GBGAME_CAMERA` (1). The beta's camera branch had no `return` and
  fell through to `GBGAME_OTHER`.

### Menus (`pheadmenu.c`)

The dialogs are rebuilt from the original text, one label per line of text as
the game's other menus do. Where the text doesn't say, the layout follows the
game's other menus and the E3 1999 footage.

- **Entry:** Player Setup → PerfectHead (`setup.c`), hidden without the
  Expansion Pak.
- **One player at a time:** there is one camera editor, so while one player is
  in PerfectHead the others get "PerfectHead is in use by *name*". That message
  isn't in the game's text, so it is built in code (in English). The owner is
  released when their PerfectHead dialog closes, or found stale if their menus
  closed another way.
- **Messages on the right screen:** camdraw showed "Calibrating Camera" and the
  other banners on the menu of the camera's controller number. They now go to
  the player using PerfectHead (`CAM_MENUPLAYER`, `pheadMenuPlayer`), whichever
  controller the camera is in.
- **Split screen:** with two or more players each dialog has half the width or
  a quarter of the screen. The sentences are then joined into word-wrapped
  paragraphs (`pheadMenuFitLabels`), in the small font in a quarter, where a
  few sentences that the buttons repeat are left out. Pictures and the head
  shrink to the space the dialog has (`pheadMenuFitModels`), and the head is
  centred on its dialog, not the screen. Position Picture's rows ask for no
  more width than the dialog has, or its Brightness bar and value are drawn
  past the dialog's edge; Brightness shares a row with its bar only where the
  row fits (`pheadMenuFitPosition`).
- **Calibration:** Take Picture shows no picture and Shoot! stays disabled until
  calibration has finished and one capture with the new exposure has arrived;
  the earlier captures are badly exposed and look zoomed in.
- **Cancelling Take Picture** puts back the picture you had: the live preview
  writes into the editor's picture, so it is copied to a spare slot
  (`PHEAD_PICTURESLOT`) while Take Picture is open.
- **Load From Camera** lists the camera's album in album order and has an OK
  item to download the chosen photo. The original text for that dialog has
  only Cancel, and carousels have no button action, so how the original
  confirmed a photo is unknown.
- **Position Picture:** the C buttons move the crop frame and L/R resize it.
  The frame can reach 16 pixels past each edge of the picture
  (`PHEAD_CROP_MARGIN`), so a face that fills the picture can still be framed
  whole; past the picture the head keeps its own face. No version has text for
  the controls, so the frame shows them: arrows and corner marks that pulse
  until the frame is first moved and light up while a button is held.
- **Color Face & Hair:** four skin tones and five hair colours. "Normal" (the
  base head's own hair) is added before the original Blonde, Black, Auburn and
  Blue Rinse, using the game's existing "Normal" text; it is the default for a
  new head.
- **Load PerfectHead** lists the files on every inserted Controller Pak by
  their thumbnails. The file with the highlight is shown as its 3D head once
  the highlight has rested on it for a quarter of a second, so scrolling
  doesn't read every file.

### Painting the head (`phead.c`)

The heads Rare made for PerfectHead are not in any ROM, so a PerfectHead is
drawn on one of the Combat Simulator's own heads (the *base head*, chosen with
Change Head). The base heads share one texture layout:

- a face texture, CI8 32x64 with an RGBA16 palette, stored upside down (row 0
  is the chin);
- two side textures, CI8 32x64, mirror images of each other (ears and hair).

Only the private copy of the head being shown is changed; the head everywhere
else in the game is untouched.

- **Skin and hair:** when a head is loaded, `pheadClassifyTexels` sorts its
  texels into skin and hair by comparing them with the cheeks: far from the
  skin's hue, much darker, or much less red (blonde hair) is hair. A 5x5
  majority filter removes speckle; on the face, hair only counts if it joins
  the hairline; on the sides, small "skin" islands away from the neck are
  hair. Hand-painted masks (`pheadmasks.c`) replace the result where the
  rules get a head wrong.
- **The picture:** the face texture's skin gets the picture through the crop
  frame. A square (the same as the placement guide) limits it at the sides and
  the chin, and it fades into plain skin at the square's edge, next to the hair
  and below the hairline.
- **Levels:** the Game Boy Camera has four shades, so the picture is stretched
  over the skin ramp, using only the painted part of the face and ignoring its
  darkest and brightest 4%. The face's median shade lands on the head's cheek
  shade, so the picture blends into the rest of the head. Position Picture's
  Brightness then moves it up or down the ramp.
- **Palettes:** each face and side palette is split in two: the first half a
  ramp of the chosen skin tone, the rest a ramp of the hair colour. The ramps
  run from each colour's average shade, darker and lighter, because the
  original colour table's six shades of a colour are nearly the same
  brightness. The palette sizes are the textures' own (100 to 256 colours),
  read from the texture records. Other CI8 hair textures (a ponytail's back)
  are recoloured through their palettes.
- **Shape Head** moves the front of the face in or out (depth); the back of
  the head, the ears and the hair keep their shape. The vertices are recorded
  when the head loads.
- **Elvis** is a Maian with one 32x48 CI4 skin texture and no hair, so
  `pheadPaintElvis` maps the picture onto the part of his skin under his eyes
  and keeps his own grey palette. **The dataDyne Sniper** has only a face
  texture; the rest of his head is helmet and visor, which stay on.
- **On N64** the texture configs hold physical addresses, so they are
  converted before the CPU writes them (`PHEAD_TEXDATA`), odd texture rows are
  TMEM-swizzled, and the data cache is written back after painting.

### Loaded heads and the Controller Pak

- **Slots:** camdraw has 23 slots: heads 0-17, then the editor, undo, two work
  slots and the Take Picture backup. Only the first `PHEAD_NUM_HEADS` (4) head
  slots get buffers; they are allocated once, from the permanent pool, so
  loaded heads last until the console is switched off. The editor's and work
  slots' buffers come from the stage pool while PerfectHead is in use.
- **The editor** works on its own slot. Opening it copies the slot to the undo
  slot, so leaving without Keep Changes reverts; Keep Changes copies it into
  the head being edited (`pheadKeepEditor`). camdraw's crop and depth
  setters also write the head being edited (its picture builder reads the
  crop from there), so those values are saved and restored with the undo slot
  (`pheadBeginEdit`, `pheadRevertEdit`).
- **Keep / Revert** (`L_MPMENU_381`-`385`, text the original never showed)
  is the prompt for backing out of an edited head. The editor's tick sees B
  before the menu does and opens it instead when the player has changed the
  head (`g_PheadEditorChanged`: a new picture, or a change in Change Head,
  Position Picture, Color Face & Hair or Shape Head that wasn't cancelled)
  and the head was already loaded (`g_PheadMakingNew` is false). A new head
  isn't asked about: backing out of it is backing out of making it, as
  before.
- **Where a head is used** (Combat Simulator, missions or both) is kept with
  the loaded head in memory and asked again whenever a head is made or loaded.
  It isn't saved in the camera file: the file's two flag bits that look free
  are camdraw's picture settings (`Cam_SetAutoDeArtefact`, and a side-lighting
  correction that camdraw turns on again with every capture).
- **Saving** is the file manager's `FILEOP_005` (`pheadSaveHead`), loading
  `FILEOP_103` (`pheadLoadHead`); the file manager had both operations but no
  code for them. Perfect Dark replaces a file by writing a vacant file of the
  same type and marking the old one vacant. Camera files are large, so they
  aren't made with the game's note: `pheadSaveHead` creates them on demand, one
  to write a new head into and one to swap with.
- **The camera file** is camdraw's own format, unchanged: settings, the
  16x16 thumbnail and the picture compressed with an 8x8 DCT to fit 1 KB.
  The crop is read back as signed bytes so a frame past the picture survives;
  older files only hold 0-64 and read the same.

### Using heads in the game

- **Combat Simulator:** the head carousel has the loaded Combat Simulator heads
  after the normal heads, skipping empty slots. In a match each player and
  Simulant with a PerfectHead gets their own copy of the base head
  (`pheadLoadHeadModel`), fitted to their body and painted. The copy has a
  16 KB texture pool of its own: in the shared texture cache, painting would
  repaint every character with that base head. If the stage has no room for
  the copy, the head is loaded unpainted like any other head. The character
  uses the base head's height, gender and voice.
- **Player files** keep the head's Controller Pak file ID (as the original
  did). Loading a player finds the head among the loaded heads, or loads it
  from the pak if it is inserted and a slot is free.
- **Missions:** once a stage has chosen its guard heads (`bodiesReset`),
  `pheadAddMissionHeads` swaps up to half of the male and the female heads for
  mission heads of that gender. Each mission head is loaded and painted once
  per stage and shared by its guards.
- **Missing heads:** a head number that no longer names a loaded head (a head
  removed, or a setup file from another session) falls back to Joanna's head.

### Fixes to the original code

These are behind `PERFECTHEAD` in shared code and affect PerfectHead only.

| Problem | Fix |
|---|---|
| Auto exposure ran away to white within seconds (`func0f14b484`): the PID's "derivative" was the error divided by the time step, with the wrong sign | Uses the error's rate of change, with standard damping. This may be part of why the feature was cut. |
| On a real camera in a bright room the picture stayed white (reported on hardware). The auto exposure set the exposure from an index into its table that still pointed at entry 30 (`0x085a`) after calibration, throwing the calibrated exposure away; the table's shortest exposure (`0x0200`) is longer than a bright room needs (Pan Docs: `$0300` out of direct sunlight, `$0030` in it); and the gain could never change, as the index was clamped to the table before the checks for running off its ends. Calibration also started at 23 dB, above the 14-20 dB the Game Boy Camera ROM uses in light | The exposure is stepped from where calibration left it, down to `0x0030`; the gain changes at the exposure's limits, between 14 and 32 dB (the camera ROM's range); calibration starts at 20 dB (`editor_step_exposure`) |
| On a real camera the live picture was white in any light, even at the shortest exposure (reported on hardware). The sensor's reference voltage V and offset O (registers A004 and A005, `pak0f11d118`) were taken from byte 8 of the camera's calibration vector, which the Game Boy Camera writes to its save RAM (`$AFF2`, echo `$BFF2`). That byte is a black-level threshold (typically `$7C`-`$7D`), so V became about 2.5 V and O about +0.9 V, where the camera itself uses 1.5 V and +0.06 to +0.48 V: black sat above every threshold | The camera's own live values: A004 `$03` (V = 1.5 V) and A005 `$A5` (O = +160 mV, in the middle of the offsets real cameras find for themselves). Source: Raphael Boichot's Game Boy Camera disassembly (`docs/19_sensor_guide.md`, `docs/06_calibration.md`) |
| The album reader took a photo's album number as its memory slot, so after a photo was deleted on the camera it showed the wrong photos, including deleted ones | Lists the album in order, each photo from its own slot (the state vector at SRAM `0x11B2` maps slots to album numbers) |
| A camera never switched on (blank save) gave 30 junk album entries | The album is only read when the camera's "Magic" signature follows it |
| An empty album crashed the game (a `% 0`) when Load From Camera opened | Skipped when the album is empty |
| The album's "thumbnail loaded" flags were never cleared while a camera was present, so no thumbnail loaded | Cleared when Load From Camera opens |
| The album reader stalled behind a finished live capture | The capture is acknowledged first |
| Loading a head with the camera off crashed: the thumbnail was rebuilt from the capture buffer, which only exists while the camera runs | Skipped without a capture buffer; the file brings its own thumbnail |
| Cam_Stop freed the capture buffers but kept the pointers | Cleared |
| The compressed picture reused when saving wasn't invalidated by a new photo | Invalidated when a picture is kept |
| An empty list (`menuitemListRender`) left its clipping in place, hiding the items under it | The clipping is restored |
| Reading a small file whose block was in the pak's header cache (straight after saving it) copied the whole 32-byte Controller Pak block into a 16-byte buffer (`pak0f11b86c`). Harmless on N64, but macOS's stack protector stops the game | The buffer holds a whole block (`AVOID_UB` builds: the port and the decomp's gcc build) |
| The crop and depth setters (`func0f14ec2c` and `func0f14eeb0`) write the head being edited as well as the editor, so throwing changes away left a moved crop or a new depth on the head | The head's crop and depth are kept when the editor opens and put back with the editor (`pheadBeginEdit`, `pheadRevertEdit`) |

### PC port

- **Virtual Game Boy Camera (`port/src/gbcam.c`):** the camera cartridge behind
  the `osGbpak*` calls: the mapper, the registers at `A000`-`A035`, 128 KB of
  save RAM (`gbcam.sav`) and the capture itself, with the documented busy time,
  exposure, gain, edge enhancement, dither matrix and tile format. The sensor's
  analog response is approximated: the PC's camera picture counts as a room
  with good light, which Pan Docs puts at exposure `$0800`, taken at 14 dB.
  (It used to be anchored at the game's own default exposure, which hid the
  auto exposure bug above.) The reference voltage V and offset O move the
  black level, and a save without one gets the calibration record a real
  camera writes, so wrong sensor settings show here as they do on hardware.
  The game's capture code runs unchanged.
- **Frame source (`gbcamsrc.c`, `webcam.c`):** the PC's camera through SDL3's
  camera API (Media Foundation, AVFoundation, V4L2/PipeWire), or the test card.
  SDL2 and SDL3 can't be linked into one program, so SDL3 is loaded at run time
  and only its camera functions are used, as sdl2-compat does. The camera is
  opened on the first capture in its smallest mode of at least 160x120; each
  frame's middle is cropped to the Game Boy Camera's 128x112 and scaled down
  in greyscale. It closes 3 seconds after the last capture, so its light goes
  off when you leave the menus, and at exit.
- **Virtual Controller Pak (`port/src/mempak.c`):** the libultra Controller Pak
  file system (`osPfs*`) on a standard `.mpk` image, including Perfect Dark's
  own `osPfsReSizeFile`. libultra's own code reads raw big-endian blocks into C
  structs, so it can't run on little-endian PCs; this works on the image's
  bytes directly. A new pak is created formatted.
- **Renderer details:** camdraw writes its textures with the N64's odd-row
  TMEM swap, which the port's renderer doesn't undo, so the swap is N64-only
  (`CAM_TMEMSWAP`); the 4-bit thumbnail writer has a byte-wise PC path; painted
  textures are dropped from the renderer's texture cache.

## Memory and performance

PerfectHead needs the Expansion Pak (8 MB), like the camera engine itself.

| What | When | Size |
|---|---|---|
| Camera slots | Always, from boot (permanent pool) | About 23 KB |
| Four loaded heads: face texture, thumbnail, compressed picture | Always, from boot (permanent pool) | About 21 KB |
| Work slots and the Take Picture backup | While PerfectHead is in use (stage pool) | About 56 KB |
| camdraw's capture buffers (Cam_Start) | While the camera runs (`mema`, freed after) | About 151 KB |
| camdraw's album thumbnails | Once per stage, when the album is read | About 135 KB |
| Painting backup (texel classes, vertex depths) | From the first head painted in a stage (stage pool) | About 15 KB |
| Each player or Simulant with a PerfectHead | Per match (stage pool) | The head model plus a 16 KB texture pool |
| Each mission head in use | Per stage (stage pool) | The head model plus a 16 KB texture pool |
| Hand-painted hair masks | ROM and RAM | 256 bytes per mask, about 10 KB |

- **Per frame:** nothing extra in play; a painted head draws like any other.
  Painting is a one-off cost when a head loads (a few passes over three 2 KB
  textures).
- **Capturing:** each capture step re-identifies the Game Boy pak (as the beta
  did), a few extra controller transfers per frame while the camera menus are
  open.
- **Split screen:** four players with four different PerfectHeads add about
  4 × 16 KB of stage pool to the normal head models.

## Base heads: where they are and how to adjust them

The base heads in Change Head → Choose Head are existing Combat Simulator
heads. Their models and textures are **not stored in this repository**: each is
a model file inside the ROM (`data/pd.<romid>.z64`), loaded at run time. The
PerfectHead code doesn't ship replacement textures. Instead, every time a head
is loaded, `pheadPaintHead` repaints the game's private copy of that head's
textures in memory.

### The table: `g_PheadTemplates` in `src/game/phead.c`

One row per base head, in the order Choose Head shows them:

```c
{ headnum, facetex, { sidetex, sidetex }, eyerow, mouthrow, hairrow, hairlum, sidered },
```

| Field | Meaning | How to find or tune it |
|---|---|---|
| `headnum` | `HEAD_*` number (`src/include/constants.h`). `g_HeadsAndBodies` (`src/game/modeldata/robot.c`) maps it to the ROM file. | Must be a head whose face and both sides are CI8 32x64 textures. |
| `facetex` | Index of the face texture in the head's texconfigs (stored upside down: row 0 is the chin) | The face texture is symmetric left/right. |
| `sidetex` | Indices of the two side textures (ears and hair), or `PHEAD_NOSIDE` for a head with only a face | They are mirror images of each other. |
| `eyerow`, `mouthrow` | Face texture rows of the eyes and the mouth (0 = chin) | Place the placement guide's lines and the photo's square limits. If the guide's lines don't sit on the head's eyes and mouth, adjust these. |
| `hairrow` | Face texture rows from here up are always hair (the hairline) | Lower it if the photo runs into the hair; raise it for a bald or high forehead. |
| `hairlum` | Side texels darker than this % of the cheeks are hair (default 50) | Raise it (e.g. 65) when hair stays skin-coloured. |
| `sidered` | Side texels less red than this % of the cheeks are hair (default 80) | Lower it (e.g. 55) when temples or ears turn into hair patches. |

Current heads (`FILE_*` names are in `src/include/constants.h`; the ROM file
names are listed in `src/assets/<romid>/files/list.c`, e.g. `CheadbeauZ`):

| # | Head | Model file | Notes |
|---|---|---|---|
| 0 | `HEAD_BEAU1` | `FILE_CHEADBEAU` | Ponytail. Pale skin and ash brown hair: `hairlum` 65, `sidered` 55. |
| 1 | `HEAD_CHRIST` | `FILE_CHEADCHRIST` | Grey-brown hair: `hairlum` 65. |
| 2 | `HEAD_DAVEC` | `FILE_CHEADDAVEC` | |
| 3 | `HEAD_NEIL2` | `FILE_CHEADNEIL2` | |
| 4 | `HEAD_PRYCE` | `FILE_CHEADPRYCE` | Bob. |
| 5 | `HEAD_WONG` | `FILE_CHEADWONG` | |
| 6 | `HEAD_TINTIN` | `FILE_CHEADTINTIN` | |
| 7 | `HEAD_PEER_S` | `FILE_CHEADPEER_S` | |
| 8 | `HEAD_ALEX` | `FILE_CHEADALEX` | A fourth texture for the back of the ponytail, recoloured by palette. |
| 9 | `HEAD_JULIANNE` | `FILE_CHEADJULIANNE` | |
| 10 | `HEAD_LAURA` | `FILE_CHEADLAURA` | |
| 11 | `HEAD_EILEEN_H` | `FILE_CHEADEILEEN_H` | |
| 12 | `HEAD_SCOTT_H` | `FILE_CHEADSCOTT_H` | |
| 13 | `HEAD_ELVIS` | `FILE_CHEADELVIS` | Painted by `pheadPaintElvis`; offered once Elvis is unlocked in the Combat Simulator. |
| 14 | `HEAD_DDSNIPER` | `FILE_CHEADDDSNIPER` | Face only (`PHEAD_NOSIDE`): the rest is his helmet and purple visor. Offered once the villa characters are unlocked. |
| 15 | `HEAD_MOTO` | `FILE_CHEADMOTO` | Shigeru Miyamoto. Not in the JPN version, so he is last and the JPN build stops at row 14. |

A new head starts on a random base head among those unlocked.

### Other settings in `src/game/phead.c`

- `g_PheadColours`: the 4 skin tones and 4 hair colours (Blonde, Black, Auburn,
  Blue Rinse), copied from camdraw's colour table (the decomp declares each
  entry as a separate array, so only the N64 build lays them out together).
  `pheadWriteRamp` uses only each colour's average: the ramp is that shade at
  the anchor (`PHEAD_SKINSHADE` or `PHEAD_HAIRSHADE`), darkening evenly to 15%
  below it and lightening towards white above it.
- **Normal hair** (`PHEAD_HAIRCOLOUR_NORMAL`, hair colour 4, after the
  original four) keeps the base head's own hair colour.
  `pheadCaptureHairColours` records the average original colour of the hair
  texels in 16 bands by painted shade, and the hair ramp interpolates between
  them. The value is saved in the existing hair colour byte.
- `PHEAD_SKINSHADE` / `PHEAD_SKINSHADE_MAX`: where the cheeks sit on the skin
  ramp, and the brightest skin allowed.
- `PHEAD_HAIRSHADE` / `PHEAD_HAIRSHADE_MAX`: the same for hair.
- `PHEAD_FACEBOX_RX` (`src/include/game/phead.h`): half the width of the photo
  square, in face texels (15 of the face texture's 32). The placement guide
  uses it too.
- In `src/game/pheadmenu.c`, `pheadMenuDrawGuide` draws the guide; its square
  must match the painter's, so change both together. `PHEAD_GUIDE_SCALE` (80)
  draws the guide at that percentage of the painter's square, so the player
  frames their face a little smaller, which maps onto the head better.

### Adding or checking a head

1. Check the head's textures. A head works only if its face and two sides are
   CI8 32x64 textures with one level of detail (the palette, of any size,
   straight after the indices). Most Rare staff and guest heads are; the story
   characters, aliens and masked heads are not. The game checks this when it
   loads the head and doesn't paint one that fails.
2. Find the texture indices and rows. Export the textures with
   `--phead-texdump` (below) or open the extracted model file in a texture
   viewer: the sides are mirror images, the face is symmetric, the eyes are
   the darkest band of rows and the mouth the reddest row.
3. Add a row and open Choose Head:
   - Patchy hair or skin means `hairlum`/`sidered` need tuning, or a
     hand-painted mask.
   - If the guide doesn't line up with the head, adjust
     `eyerow`/`mouthrow`/`hairrow`.
4. Add the head's row to `g_PheadHairMasks` (`src/game/pheadmasks.c`), `NULL`
   if it has no masks; the build checks the two tables have the same length.

Changing the original pixel art itself, rather than these settings, means
editing the head model file in the ROM with a model or texture tool. That
would also change the head everywhere else in the game, so the runtime repaint
is preferred.

### Hand-painted hair masks (`tools/pheadmask`)

Where the automatic skin/hair split gets a head wrong (patches of skin colour
in the hair, or hair colour on an ear), paint the split by hand:

1. Export the base heads' textures: run a PC port PerfectHead build with
   `--phead-texdump` and go through every head in Change Head. Each head
   writes `pheadtex_<head>_<0 face, 1-2 sides>.bin` to the save folder.
2. Open `tools/pheadmask/pheadmask.html` in a browser (it works offline and
   sends nothing anywhere) and load those files.
   - Each head starts from the automatic split.
   - Left button paints hair, right button paints skin.
   - "Mirror the sides" paints both sides at once.
   - On the face, the dotted square is where the photo goes and the yellow line
     is `hairrow`.
3. Export, save as `src/game/pheadmasks.c` (in both repositories), and rebuild.
   - Loading that file into the tool again lets you carry on editing.
   - Work in progress is also kept in the browser.

`g_PheadHairMasks` holds, for each base head, the face and two side masks
(`NULL` = automatic): 64 rows of 32 bits, row 0 = the bottom of the
texture. A mask replaces the classifier's result for that texture;
`hairrow`, the face square and the hair colour still apply. On the face, the
mask's hairline is also where the photo stops.

The textures come from your ROM, so keep the exported `.bin` files to
yourself; only the masks go in the repository.

## Testing

Tested so far: the whole flow in Mupen64Plus (N64 ROM, ntsc-final, one to four
players) and on the PC port on Windows (x86_64, virtual camera with the test
card and a real webcam). Not tested yet: real N64 hardware with a Transfer Pak
and camera, the PAL and JPN ROMs, and the PC port on macOS and Linux.

## Known limitations

- **Base heads** are existing Combat Simulator heads repainted, not the heads
  Rare made for PerfectHead, which are not in any ROM. Shape Head has only
  depth: camdraw could scale 7 parts of the head, but the menu text has only
  "Depth".
- **Four heads** can be loaded at a time (`PHEAD_NUM_HEADS`); the camera engine
  had 18 slots. Each extra head costs about 5 KB of permanent memory.
- **Where a head is used** isn't saved with it, and mission heads last until
  the console is switched off: the game save in the cartridge's EEPROM has no
  room for them, and changing its format would break existing saves. Load them
  again with Load A Saved Head.
- **Setup files** save a Simulant's head slot, not which head was in it (as the
  original did). A different head in that slot later is used instead; an
  empty slot falls back to Joanna.
- **Dark hair colours** can leave patches of skin colour on the sides of some
  base heads where the skin/hair split is unsure; a hand-painted mask fixes
  that head.
- **"PerfectHead is in use by…"** is in English in every version: the game has
  no text for it.
- **Split screen with three or four players:** Save PerfectHead and Head
  Complete! (when a head is used for both) are taller than a quarter of the
  screen, so the menu scrolls them and their first line sits under the title.
- **PC port:** a webcam plugged in after "no camera found" is only picked up
  after a restart.

## References

- Pan Docs, Game Boy Camera: https://gbdev.io/pandocs/Gameboy_Camera.html
- MAME's Game Boy Camera driver (register layout): https://git.redump.net/mame/plain/src/devices/bus/gameboy/camera.cpp
- Game Boy Camera reverse engineering notes (AntonioND): https://www.skylyrac.net/p/2015-03-12-game-boy-camera-documentation/
- Game Boy Camera save format (Raphael Boichot): https://github.com/Raphael-Boichot/Inject-pictures-in-your-Game-Boy-Camera-saves
- Controller Pak file system: https://n64brew.dev/wiki/Controller_Pak/Filesystem
- SDL3 camera API: https://wiki.libsdl.org/SDL3/CategoryCamera
- Perfect Dark decompilation: https://github.com/n64decomp/perfect_dark
- E3 1999 footage of PerfectHead (Time Extension): https://www.timeextension.com/news/2022/09/new-e3-1999-footage-gives-closer-look-at-perfect-darks-perfect-head-mode
- Cut-feature background: https://www.gonintendo.com/contents/9689-footage-emerges-of-cut-perfect-dark-mode-that-let-players-scan-their-own-faces-into
