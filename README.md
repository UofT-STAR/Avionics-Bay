# Avionics Bay

Arduino C++ code for UofT STAR's avionics bay, using PlatformIO. This repository includes an ICM-20948 sensor experiment and a larger program for sensor readings, SD logging, and flight-state handling.

## For those new to avionics

Start with **[Join the team and make your first contribution](docs/start-here.md)**. It covers access, cloning, the first build, and your first pull request. No hardware is needed to start.

**Avionics lead:** Tymon Cui ([@ccTymon](https://github.com/ccTymon)). For GitHub access, signing, permissions, or workflow problems, contact **Kai ([@KaiYT1](https://github.com/KaiYT1))**. Tymon coordinates avionics tasks and hardware sessions.

## What currently builds

| File | Role |
| --- | --- |
| `platformio.ini` | Build settings, board selection, libraries, and serial settings. |
| `src/test1.cpp` | **Current build target:** reads an ICM-20948 over I2C and prints X/Y acceleration. |
| `src/main.cpp` | Larger avionics program; **excluded from the current build**. Contains deployment-output control. |
| `.vscode/extensions.json` | Recommends the PlatformIO IDE extension. |

The board is described as an **Arduino Nano**, but its exact variant and bootloader remain unconfirmed. The default `legacy_uno` environment preserves the original working `board = uno` configuration; its name does not identify the physical board. The optional `nano` and `nano_old` environments target classic ATmega328P Nanos with newer and older bootloaders respectively. Confirm the hardware before choosing an upload environment.

## Terminal setup checklist

New to Git? A **repository** is the project and its saved history. A **clone** is your local copy. A **commit** saves a set of changes locally; **pushing** sends commits to GitHub. A **branch** keeps your work separate while it is reviewed in a **pull request (PR)**.

Work through one step at a time. Copy only the **Run** block, press Enter, and compare the result before continuing. The output blocks below are **illustrative examples, not screenshots or commands**. Versions, names, paths, and hashes will differ on your computer. A command that finishes normally returns to the terminal prompt. Do not type the prompt itself.

**GitHub CLI (`gh`) is not required.** We use GitHub Desktop and the website for signing in and publishing. You also need Git available in your terminal for the checks below.

### 1. Open a terminal and check Git

In VS Code, choose **Terminal > New Terminal**. On Windows, PowerShell is fine. This first check works from any folder.

**Run:**

```sh
git --version
```

**Example output:**

```text
git version 2.55.0.windows.3
```

**Pass:** a Git version appears. If you see “not recognized” or “command not found,” install [Git](https://git-scm.com/downloads), restart VS Code, and try again. GitHub Desktop includes Git for itself, but that does not necessarily make the command available in your terminal.

### 2. Sign in and clone with GitHub Desktop

No commands for this step:

1. Sign into GitHub Desktop with the GitHub account you gave Kai.
2. Accept your organization invitation and ask Kai to confirm your Avionics team membership.
3. In Desktop, select **File > Clone repository > URL**. Enter `https://github.com/UofT-STAR/Avionics-Bay` and choose a local folder.
4. Open that folder in VS Code, then choose **Terminal > New Terminal**.

**Pass:** VS Code's file list contains `platformio.ini` and a `src` folder. If you already cloned the project, open that existing folder rather than cloning again. A public repository can be cloned before you join the team; this does not prove you have permission to push.

### 3. Check that the terminal is inside the clone

**Run:**

```sh
git rev-parse --show-toplevel
```

**Example output on Windows:**

```text
C:/Users/YourName/Documents/GitHub/Avionics-Bay
```

**Pass:** the path is your Avionics-Bay folder. If you see “not a git repository,” open the cloned folder in VS Code and create a new terminal there. Do not run `git init` to fix this; it would create a new repository instead of locating your clone.

### 4. Check which GitHub project it connects to

`origin` is Git's usual name for the remote repository you cloned.

**Run:**

```sh
git remote -v
```

**Example output:**

```text
origin  https://github.com/UofT-STAR/Avionics-Bay.git (fetch)
origin  https://github.com/UofT-STAR/Avionics-Bay.git (push)
```

**Pass:** both lines identify `UofT-STAR/Avionics-Bay`. An SSH URL beginning `git@github.com:` is also valid. If a different project appears, stop and ask Kai to help check which folder you opened.

### 5. Check your starting state

**Run:**

```sh
git status
```

**Example output for a fresh clone:**

```text
On branch main
Your branch is up to date with 'origin/main'.

nothing to commit, working tree clean
```

**Pass:** a fresh clone is clean. If files are listed as modified or untracked, you have local changes. That is normal after editing, but understand those changes before switching branches; do not delete or reset them just to match this example. Another branch name is fine if you deliberately created it.

### 6. Check the name and email attached to commits

**Run these two commands:**

```sh
git config --get user.name
git config --get user.email
```

**Example output:**

```text
Alex Student
12345678+alexstudent@users.noreply.github.com
```

**Pass:** your name and an email associated with your GitHub account appear. Your GitHub-provided noreply address is fine; find it in GitHub's email settings. These values identify your commits; they do not log you into GitHub.

If either is blank or incorrect, replace the example values below with your own. Keep the quotes. These settings apply only to this repository.

**Run only if needed:**

```sh
git config --local user.name "Your Name"
git config --local user.email "YOUR_GITHUB_VERIFIED_EMAIL"
```

**Expected output:** none on success. Repeat the two read commands above to confirm your values were saved.

### 7. Check commit signing

A signature lets GitHub verify who signed a commit. Your GitHub password and a signing key are different things. The repository's `main` rules require signed commits.

**Run these two commands:**

```sh
git config --get commit.gpgsign
git config --get user.signingkey
```

**Example output for one SSH signing setup:**

```text
true
C:/Users/YourName/.ssh/id_ed25519.pub
```

**Pass for this configuration check:** automatic signing is `true` and a key is configured. A GPG key ID instead of a file path can also be valid. A blank value means that setting is not configured through this Git configuration; do not assume a key is missing from the computer.

**If signing is not configured:** ask Kai to help set it up using [GitHub's signing guide](https://docs.github.com/en/authentication/managing-commit-signature-verification/telling-git-about-your-signing-key). Existing working SSH or GPG setups are both acceptable. Do not overwrite an existing key or share a private key. For SSH signing, GitHub must have the public key registered as a **Signing key**, not only as an Authentication key.

These settings alone do not prove that the private key is available or that signing works. The first real commit in step 10 is the end-to-end check. Desktop can have different signing behavior from your terminal, so verify the client you actually use.

### 8. Check PlatformIO

Install the **PlatformIO IDE** extension in VS Code and let setup finish. Open its PlatformIO terminal if `pio` is unavailable in your normal terminal.

**Run:**

```sh
pio --version
```

**Example output:**

```text
PlatformIO Core, version 6.2.0
```

**Pass:** a version appears. If the command is not found, follow the [PowerShell instructions](#if-powershell-cannot-find-pio), then return here. You do not need a board connected.

### 9. Build the sensor experiment

Keep the terminal inside the repository folder.

**Run:**

```sh
pio run -e legacy_uno
```

**Example ending of a successful build; intermediate output omitted:**

```text
Environment    Status    Duration
-------------  --------  ------------
legacy_uno     SUCCESS   00:00:06.366
```

**Pass:** PlatformIO reports **SUCCESS**. The first run downloads dependencies and may take longer. If it fails, save the first meaningful error and ask for help; do not continue to uploading.

This builds `src/test1.cpp` only. It does not upload anything, test the sensors, or validate `src/main.cpp`. `legacy_uno` preserves the previously working configuration; the actual board and bootloader still need confirmation before a hardware session.

### 10. Confirm Write access and signing with a real contribution

Do this once you have a small task agreed with Tymon:

1. In GitHub Desktop, create a new branch, such as `docs/clarify-setup`. A branch is where your change will live before review.
2. Make the agreed edit. Review the **Changes** tab and select only files belonging to your task.
3. With your signing setup ready, commit the change with a short description. A commit saves it locally; it does not publish it yet.
4. Click **Publish branch**. **Pass:** the branch appears on GitHub. If permission is denied, check Desktop's signed-in account and ask Kai to confirm team membership.
5. Open the commit on GitHub and look for **Verified**. **Pass:** GitHub recognizes the signature. If it is unsigned or unverified, ask Kai to help resolve signing before merging; do not disable the rule.
6. Open a pull request targeting `main`, record what you checked, and request review.

Do not use a direct push to `main` as an access test. A successful Desktop publish verifies Desktop's authentication, not a separate terminal push configuration.

**Ready:** you have the right clone, a successful build, a published feature branch, and a Verified signed commit. You can learn hardware testing in a separate supervised session.


## First build: no hardware required

1. Install Git or GitHub Desktop, Visual Studio Code, and the **PlatformIO IDE** extension in VS Code.
2. Clone this repository and open its root folder in VS Code. The root contains `platformio.ini`; do not open only `src/`.
3. Let PlatformIO initialize and install the platform and libraries declared in `platformio.ini`. The first build requires internet access.
4. Open a **PlatformIO terminal** in VS Code and run:

   ```sh
   pio run -e legacy_uno
   ```

5. Confirm that the command reports success. Save the output if it fails so a teammate can help.

The source filter explicitly includes only `test1.cpp`. Editing `main.cpp` will not change the default binary. Both files define `setup()` and `loop()`, so they must not simply be included together in one build.

## If PowerShell cannot find `pio`

The VS Code extension can install PlatformIO without adding it to a normal terminal's PATH. Open a PlatformIO terminal from the extension, or check the installation directly in PowerShell:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" --version
```

If that works, enable `pio` in the current terminal with:

```powershell
$env:Path = "$env:USERPROFILE\.platformio\penv\Scripts;$env:Path"
pio --version
```

For future terminals, add `%USERPROFILE%\.platformio\penv\Scripts` to your **user Path** in Windows Environment Variables, preserving existing entries. Restart the terminal application (and VS Code if using its terminal). If the executable is missing, finish PlatformIO IDE setup first. See the [official shell setup guide](https://docs.platformio.org/en/stable/core/installation/shell-commands.html).

## First hardware session

Use a teammate-approved sensor-only bench setup with deployment devices disconnected. Review the board, sensor breakout voltage requirements, power supply, and I2C wiring together before powering it. This repository does not yet provide a verified wiring diagram.

Once the hardware matches the selected environment:

1. Connect the board using a data-capable USB cable.
2. Find its serial port:

   ```sh
   pio device list
   ```

3. Upload the sensor experiment. Replace `YOUR_PORT` with the detected port, such as `COM7` on Windows:

   ```sh
   pio run -e legacy_uno -t upload --upload-port YOUR_PORT
   ```

4. Open the serial monitor:

   ```sh
   pio device monitor --port YOUR_PORT --baud 115200
   ```

The configuration currently hardcodes `COM5` for upload and monitoring. Explicit port arguments avoid changing the shared configuration for your machine. Close other serial monitors before uploading; use Ctrl+C to exit the CLI monitor.

**Expected observation:** two numeric columns, X and Y acceleration, when the IMU supplies data. Gently changing the sensor's orientation should change the readings. The experiment does not print an initialization success message or report initialization errors; silence is not proof that the sensor is working. Record the board, port, source revision, wiring reference, and observations.

## Learn the workflow

- [Development, debugging, and testing](docs/development.md)
- [Making your first contribution](CONTRIBUTING.md)

## Details the team needs to confirm

These are open documentation questions, not established team policy:

- Nano bootloader, hardware revision, and authoritative firmware branch or release.
- Verified wiring diagram, sensor breakout models, voltage levels, and approved bench setup.
- The support channel and reviewer rotation with Tymon, the avionics lead.
- Required checks for merging and the separate process for hardware or flight approval.

The default sensor exercise is a starting point for onboarding. This documentation does not establish flight readiness for `main.cpp`.
