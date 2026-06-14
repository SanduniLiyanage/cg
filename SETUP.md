# HOMEWARD — Setup guide (start from zero)

This is a step-by-step guide to build and run the project on a **fresh Windows
PC** that has nothing installed yet. Follow it top to bottom. It takes ~15–20
minutes (mostly downloads).

> We use **MSYS2 (UCRT64)** — a free toolchain that gives us `gcc`, `cmake` and
> the OpenGL/FreeGLUT libraries on Windows. Everything below is copy-paste.

---

## Step 1 — Install MSYS2

1. Go to **https://www.msys2.org**
2. Download the installer (`msys2-x86_64-*.exe`) and run it.
3. Keep the default install location: **`C:\msys64`**. Click through to finish.

That's the only thing you install by hand.

---

## Step 2 — Open the correct shell

MSYS2 installs several terminals. We need exactly one:

- Open the Start menu, type **“MSYS2 UCRT64”**, and open it.
- The window prompt must say **`UCRT64`** in it (purple/teal text).

⚠️ Do **not** use “MSYS2 MSYS”, “MINGW32”, or “CLANG64”. It must be **UCRT64**,
or the build will fail to find the libraries.

---

## Step 3 — Install the compiler and libraries

Paste this into the UCRT64 shell and press Enter (if it asks `Proceed? [Y/n]`,
press Enter):

```bash
pacman -Syu
```

If it closes the window at the end (it sometimes does after a core update),
**reopen MSYS2 UCRT64** and run it once more:

```bash
pacman -Syu
```

Then install everything we need in one command:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-freeglut mingw-w64-ucrt-x86_64-glm
```

Check it worked:

```bash
gcc --version
cmake --version
```

You should see version numbers (e.g. `gcc 16.x`, `cmake 4.x`). If you get
“command not found”, you’re in the wrong shell — go back to Step 2.

---

## Step 4 — Get the project code

Pick **one** of these.

**Option A — clone from our shared repo (recommended).** Replace the URL with
our actual GitHub/GitLab link:

```bash
cd /c/Users/$USER/Desktop
git clone <OUR-REPO-URL> cg
cd cg
```

> `git` is already included with MSYS2. If you don’t have a repo URL yet, ask the
> repo owner to share it, or use Option B.

**Option B — copy the folder.** If a teammate sent you the `cg` folder, copy it
to your Desktop, then in the shell:

```bash
cd /c/Users/$USER/Desktop/cg
```

> In the MSYS2 shell, your Windows `C:\` drive is `/c/`. So
> `C:\Users\YourName\Desktop\cg` is `/c/Users/YourName/Desktop/cg`.
> You can also type `cd ` (with a space) and **drag the folder** into the window.

Confirm you’re in the right place — this must list `CMakeLists.txt` and `src/`:

```bash
pwd
ls
```

---

## Step 5 — Build

From inside the `cg` folder:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

The first line configures, the second compiles. When it finishes you’ll have
`build/homeward.exe`.

---

## Step 6 — Run

```bash
./build/homeward.exe
```

A window opens and the film plays automatically (a balloon at dawn → day →
sunset → night city → touchdown, then it loops).

### Controls

| Key | Does |
|---|---|
| `L` | Show/hide concept captions (and the clip-window box + software Z-buffer panel) |
| `Space` | Pause / resume |
| `0`–`5` | Jump to a segment |
| `←` / `→` | Step 3 seconds back / forward |
| `Esc` | Quit |

---

## Re-running later

Every new session: open **MSYS2 UCRT64**, then:

```bash
cd /c/Users/$USER/Desktop/cg
./build/homeward.exe
```

If you changed code, rebuild first (close the running window so the file isn’t
locked):

```bash
cmake --build build
```

---

## Troubleshooting

| Problem | Fix |
|---|---|
| `gcc: command not found` / `cmake: command not found` | You’re in the wrong terminal. Use **MSYS2 UCRT64** (Step 2). |
| `The source directory … does not appear to contain CMakeLists.txt` | You didn’t `cd` into the `cg` folder. Run `pwd` — it must end in `/cg`. |
| `cannot open output file homeward.exe: Permission denied` | The app is still open. Close the HOMEWARD window, then build again. |
| Window opens but is black / closes instantly | Update graphics drivers; make sure you ran the `pacman -S …` line fully. |
| Pasting adds weird `^[[200~` text | Paste with **Shift+Insert**, or just type the line manually. |
| `git: command not found` | Run `pacman -S --needed git`, or use Option B (copy the folder). |
| App runs from the shell but not by double-clicking the `.exe` | Launch it from the UCRT64 shell, or add `C:\msys64\ucrt64\bin` to your Windows PATH so it can find the OpenGL DLLs. |

---

## What each teammate should know about the code

The project is split into five systems (folders under `src/`), one per member.
See **[CONCEPTS.md](CONCEPTS.md)** for the full map of all 23 CG concepts to the
exact file, function, and on-screen moment.

| Folder | Member role | Owns |
|---|---|---|
| `src/m1_primitives/` | The Draftsman | points, DDA, Bresenham, midpoint circle |
| `src/m2_fills_transforms/` | The Painter | boundary/flood/scan-line fill + 2D transforms + the 2D world |
| `src/m3_camera_clipping/` | The Camera | window→viewport, Cohen–Sutherland, Liang–Barsky |
| `src/m4_city_3d/` | The Architect | 3D city, parallel/perspective projection, orbit |
| `src/m5_depth_timeline/` | The Director | depth test, back-face cull, Z-buffer, keyframe timeline |
