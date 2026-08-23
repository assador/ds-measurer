# ds-measurer
**v2.0.0** | [Русская версия](README.ru.md) | [Mail](mailto:assador@gmail.com) | [Donate](https://boosty.to/assador/donate)

`ds-measurer` is an on-screen measurement tool that comes bundled with region screenshotting and a color picker. The program is very useful, mostly as a helper for poor frontend developers and markup layout engineers, but not limited to them. Free software, AGPLv3. AppImage builds are available (x86_64/Linux/Wayland, [description below](#code-and-builds)).

Here is the gist. A transparent layer is created on top of the screen without windows or borders — meaning, without anything at all — along with a full-screen crosshair cursor (everything underneath the layer keeps living its own life: the screen is interactive, no initial static screenshots in the buffer, etc.).

The user can measure whatever is actually on their screen — coordinates, width and height, distance between points, angles, and so on. Each measurement (which can be “frozen” and saved to a stack) also has its own stack of displayed/hidden grids governed by rules defined in the [YAML config](https://raw.githubusercontent.com/assador/ds-measurer/refs/heads/main/config/ds-measurer.yaml) (e.g., center, golden ratio, thirds, fifths, etc.). You can also add guides for these measurements to snap to. The measurements themselves — both the active one and frozen ones — can be modified and dragged around.

Features include color scheme support with hotkey switching, selection from the center and/or with a fixed aspect ratio, screenshots of selected areas, color sampling under the cursor in a bunch of different CSS formats, and a ton of customizable hotkeys. Coffee and ladies will probably be introduced in future major releases. A bit more detail, item by item:

## What this hellish machine can do

1. **Measure everything inside and out**. A measurement displays: width, height, their ratio as a common and decimal fraction, coordinates of the start and end points, the angle and length of the segment between them, as well as currently enabled grids for each specific measurement from the set defined in the [YAML config](https://raw.githubusercontent.com/assador/ds-measurer/refs/heads/main/config/ds-measurer.yaml). All parts of this rich data set can be individually shown or hidden via hotkeys for each measurement right during the selection process.
    
    The measurement itself is made with the mouse while holding down — surprise — LMB, but RMB allows you to drag this measurement to a more suitable place, even while measuring. The current measurement exists as long as at least one mouse button is pressed. As soon as you release them all, it disappears so as not to be an eyesore. However, if it’s dear to your memory, you can:

2. **Save measurements to a stack**. During the measurement process, you can save it (“freeze” it, saving it for better times) by pressing a key on the keyboard (`f` by default). There is a stack of such saved measurements where you can sequentially and cyclically select the active one using another key (`Tab` by default). The active one is highlighted and can then be modified just like a newly created one: LMB for dimensions, RMB for position on the screen, and various hotkeys to show/hide its values, grid, or diagonal with angle. Other hotkeys allow you to remove the active measurement from the stack, the last added one, or all of them (by default: `Delete`, `BackSpace`, and `j`, respectively).

3. **Guides and snapping**. Generally speaking, there are three stacks in the Measurer. The first one contains the measurements themselves, another holds “color” points (for each, a list of CSS color values underneath is displayed; [more on this below](#colorpicker)). And now we are talking about the stack for guides (by the way, you can and should switch between stacks using the corresponding keys, which can be rebound, like almost everything, in the [YAML config](https://raw.githubusercontent.com/assador/ds-measurer/refs/heads/main/config/ds-measurer.yaml); by default: `m` for the measurements stack, `g` for guides, and `h` for color points).
    
    So. Just like in any self-respecting graphics editor — which the Measurer is not and shouldn’t be — there is a guide mechanism to which everything snaps. Both during the measurement process itself (LMB) and while dragging a measurement (RMB), if any side of the measurement gets close to a guide, it snaps to it (the snapping radius is set in the config). In short, as usual everywhere.
    
    The guides themselves are placed at the cursor location via hotkeys (`[` for horizontal, `]` for vertical). Working with this stack and its guides is completely identical to working with the measurements stack, including cycling through them, dragging them (RMB), deleting current/last/all, etc.

4. **<span id="colorpicker">Color picker</span> and color sampling**. Just by moving the mouse around the screen, or in the middle of anything else, you can press the `d` key (by default), and you will get the color values for the point under the mouse cursor. The values themselves are represented as a multi-line text block, one line per format, for example:
    
    ```
    #3b460f
    rgb(59 70 15)
    rgb(23.137% 27.451% 5.882%)
    hsl(72 64.706% 16.667%)
    ```
    
    Just like the [screenshooter described below](#screenshooter), where the color picker puts this text depends on the `target` key in the corresponding config section: `target: clipboard  # both | clipboard | stack`. **both** is self-explanatory: both places. **clipboard** is also clear: straight to the clipboard. And **stack** adds a point — a small crosshair with this text next to it — at the current mouse location into our third stack. Working with this third stack is completely identical to the previous two. You can cycle through these points, drag them around, delete them, etc. When dragging a point, the color values underneath naturally update (during the drag itself, however, given the cost of color sampling, the color picker mumbles `mmm…` and updates the color only after RMB is released; we could have sampled periodically, but doing that just for real-time digits would be silly).

5. **<span id="screenshooter">Screenshooter</span> for measurement areas**. And while we’re at it, why not use this Measurer as an alternative to some bloated and overly important tool like Spectacle? For instance, the author bound calling `ds-measurer` to `Ctrl+Alt+z`. Press it, select an area with the mouse, hit `s` (by config default), and the screenshot of the selection is ready.
    
    Where it goes also depends on the `target` key in the corresponding config section: `target: clipboard  # both | clipboard | file`. **both** is self-explanatory: both places. **clipboard** is also clear: straight to the clipboard. And **file** writes directly to disk in PNG format. Where exactly on disk is specified by a pattern on the line below in the same config — `file: "~/screenshot_%Y_%m_%d_%H_%M_%S.png"`. Everything here seems self-evident too. Path, filename, timestamps supported.
    
    You can take a screenshot right while measuring with LMB held down, or while dragging a measurement with RMB held down. You can screenshot both a newly created measurement and a selected saved one.    

## Tech Stack

### C++

The author decided that everything would be blatantly stupid, polymorphic, and encapsulated to the extreme. Nobody knows anything about anybody, but everyone uses each other. Inhumane, of course. A heartless market, a consumer society. But necessary. So it’s OOP, but without fanaticism or complex hierarchies like “thing → animal → chordate → with coordinates”. Besides, he just wanted it that way, in C++.

There are basic structs like `Point`, `Color`, and `Rect`, and classes like `Measurement`, which knows its starting `Point`, ending `Point`, its stack of `Grid` instances (another mini-class), and a few other things. It knows how to tell everyone about its distances, angles, and aspect ratios, and how to change and move when prompted by a unceremonious and maximally declarative orchestrator in `app.cpp`. Pure math and algorithms.

There are separate modules for UI, hotkeys, screenshooter, and color picker. Finally, there are folks responsible for rendering all this stuff, each doing what they can and know best:

### GTK…

Even though the author doesn’t particularly love it, building a bright communist future with Qt wasn’t appealing at all. Building a bright communist future takes too long. Moreover, he decided right away to strictly separate business logic and rendering, allocating GTK its own sandbox (well, okay, a third of the Sahara). So right now, switching to another toolkit would only require replacing a single .cpp file. Yes, about 400 lines including all comments, but still. By the way, it’s the largest file in the project.

Oh. Well, a tiny bit of GTK leaked into screenshots and the color picker, but those are also separated from the core logic, and further split in terms of platform dependencies between `wlr-screencopy` and `org.kde.KWin.ScreenShot2`. This brings us smoothly to

### Wayland

We already have measurement tools for X11 (or so it seems). But trying to explain to Wayland that we aren’t attempting to steal a credit card number from the screen behind, but simply want to take a screenshot… well, fine, measure the height of an input field. And on top of that, trying to lodge `wlr-screencopy` and `org.kde.KWin.ScreenShot2` in the same communal apartment without them blowing up the shared soup pot…

The author hid all this chaos inside `namespace platform` of a dedicated module for the screenshooter and color picker, using build flags and macros (`#if defined(HAS_KDE_SCREENSHOT)` and `#if defined(HAS_WLR_SCREENCOPY)`) to ask them politely not to fight.

And on top of all this — `gtk4-layer-shell`, so that all of our stuff lays down as a transparent layer over the entire system.

### And Cairo

There were no doubts here before, and none appeared now. Why reinvent the wheel? A wonderful library for rendering all this stuff in 2D vector graphics.

## Code and Builds

Here are two AppImage builds, both for x86_64. Linux/Wayland, naturally…

Both builds differ only in the screenshooter and color picker parts. Both work in KDE and God knows where else, provided `wlroots` is present:

- The first one — [`ds-measurer-kde-x86_64.AppImage`](https://github.com/assador/ds-measurer/releases/download/latest/ds-measurer-kde-x86_64.AppImage) — takes screenshots and samples colors under the cursor only in KDE via their `org.kde.KWin.ScreenShot2`.
- The second one — [`ds-measurer-wlr-x86_64.AppImage`](https://github.com/assador/ds-measurer/releases/download/latest/ds-measurer-wlr-x86_64.AppImage) — takes screenshots and samples colors under the cursor only on `wlroots` via their `wlr-screencopy`.

When building from source manually, this is controlled by the `ENABLE_KDE_SCREENSHOT` and `ENABLE_WLR_SCREENCOPY` flags in `CMakeLists.txt`.

Once again, if either build tries to take a screenshot or sample color outside its native environment, it simply grumbles into the console and ignores the request, while everything else works properly. So if you don’t need screenshots and the color picker, it doesn’t matter which one you download.

Say what you will, but we had to bundle a fair bit of GTK into these, seeing as the app relies on it to get off the couch. As a result, our neat, sub-100KB standalone binary inflates to a couple of dozen megabytes inside the AppImage wrapper. Here’s what’s in their little bellies:

1. **The `ds-measurer` binary itself**, naturally
2. **GTK4 & the core GUI stack:**
    - `libgtk-4.so`
    - `libglib-2.0.so`, `libgobject-2.0.so`, `libgio-2.0.so` (GLib essentials)
    - `libcairo.so` and `libpango-1.0.so` (graphics and text rendering)
    - `libgdk_pixbuf-2.0.so` (for screenshots and color picking)
    - `libharfbuzz`, `libfreetype`, `libfontconfig` (text layout and fonts)
3. **Layer Shell:** a standalone build of `libgtk4-layer-shell.so` (for screen overlay functionality)
4. **Configuration parser:** `libyaml-cpp.so` (for reading config files)
5. **Assets:** `ds-measurer.desktop` and `ds-measurer.svg` (desktop entry and icon :o))

What has been excised from the bundle and is left to the host system instead:

- **Wayland & EGL stack:** `libwayland-client`, `libwayland-server`, `libwayland-egl`, `libwayland-cursor` (on the safe assumption that a Wayland utility will be running under Wayland)
- **X11 stack:** `libX11`, `libX11-xcb` (for the very same reason)
- **GStreamer:** GTK4’s media module has been completely gutted (maybe one day Measurer will sing the blues or play questionable videos in the corner while working… but not today)

So overall, the AppImage is fairly self-contained, packing **GTK4**, **Cairo** and <nobr>**gtk4-layer-shell**</nobr>. The only system requirement is an active **Wayland compositor** with <nobr>**wlr-layer-shell**</nobr> support (Sway, Hyprland, KDE Plasma, River, etc.).

---

So there you go. Welcome, feel free to use, criticize, scold, and praise. You can even [donate](https://boosty.to/assador/donate) for some sausage.

---

## Third-Party Libraries

* [tinyexpr](https://github.com/codeplea/tinyexpr) by Lewis Van Winkle (zlib License) — tiny recursive descent expression parser, compiler, and evaluation engine for math expressions.
