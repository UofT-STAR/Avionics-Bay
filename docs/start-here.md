# Start here: your first avionics contribution

**Goal:** join the team, build the sensor experiment, and prepare a small pull request. You do not need hardware for the first build.

**Avionics lead:** Tymon Cui, [@ccTymon](https://github.com/ccTymon). The [club website source](https://github.com/UofT-STAR/Website/blob/main/js/website-data.js) lists him as Director of Avionics. He coordinates avionics work and hardware sessions. **GitHub support:** Kai ([@KaiYT1](https://github.com/KaiYT1)) handles access, signing, permissions, and GitHub workflow troubleshooting.

## 1. Get access

1. Create a GitHub account if you do not have one. Coordinate joining avionics with Tymon, then give Kai your **GitHub username** for organization and team access. The club's [joining page](https://utstar.sa.utoronto.ca/join-us.html) points students to its Discord community.
2. If you are not already a UofT-STAR organization member, an organization owner must invite you. Accept the invitation using the intended GitHub account.
3. Kai helps arrange your membership in the [Avionics team](https://github.com/orgs/UofT-STAR/teams/avionics). Organization membership alone gives Read access; Avionics membership supplies Write access to this repository.
4. Sign into GitHub Desktop with the same account. If publishing a branch is denied, ask Kai to check your active organization and team membership.

The repository is public, so you can read, clone, and build while waiting for access. Cloning successfully does not prove you can push. Do not share passwords or access tokens. Members work on branches in this repository; a personal fork is not required for that workflow.

Before continuing, work through the [terminal setup checklist](../README.md#terminal-setup-checklist) to check GitHub access, your clone, signing, and PlatformIO.

## 2. Clone and build

1. Install GitHub Desktop and VS Code. In VS Code, install the **PlatformIO IDE** extension.
2. In GitHub Desktop, choose **File > Clone repository > URL** and enter `https://github.com/UofT-STAR/Avionics-Bay`.
3. Open the cloned folder in VS Code. You should see `platformio.ini` at the root.
4. Open a **PlatformIO terminal** and run:

   ```sh
   pio run -e legacy_uno
   ```

5. Wait for dependency installation and compilation. Your checkpoint is a **SUCCESS** result. If `pio` is not found, follow the [PowerShell fix](../README.md#if-powershell-cannot-find-pio).

`legacy_uno` preserves the originally working configuration. The physical board is described as a Nano, but its exact variant and bootloader still need identification. This first step only compiles; do not upload until a teammate confirms the hardware setup.

## 3. Learn these three files

| File | What you need to know |
| --- | --- |
| `platformio.ini` | Selects the environment, libraries, and source files. |
| `src/test1.cpp` | The program you just built: an ICM-20948 sensor experiment with serial output. |
| `src/main.cpp` | Larger avionics program, currently excluded from builds. Editing it does not change the default binary. |

## 4. Make your first pull request

1. Agree on one small task with Tymon. A correction to unclear setup instructions is a good hardware-free starting point.
2. In GitHub Desktop, create a branch such as `docs/clarify-first-build`.
3. Edit the file, inspect the diff, and run the appropriate check. Preview documentation; build firmware changes using the environment that actually includes them.
4. Commit your change and choose **Publish branch**. Open a pull request targeting `main`.
5. Describe the change, checks performed, and any checks you could not run. Request review from Tymon or the reviewer he designates.

**Existing merge rules:** `main` requires signed commits and a pull request with at least one approval, including approval of the latest reviewable push by someone other than its pusher. Do not assume a normal local commit is signed: check your signing setup with Kai before the first merge. A PR can be reviewed while a signing issue is resolved. Do not bypass the rules or push directly to `main` to complete onboarding.

See [CONTRIBUTING.md](../CONTRIBUTING.md) for the full workflow and [development.md](development.md) for debugging and testing.

## Ready for your next session

- [ ] Accepted the organization invitation and joined Avionics.
- [ ] Cloned the repository and built the sensor experiment successfully.
- [ ] Can explain which source file is built and why.
- [ ] Published a branch and opened a small pull request.
- [ ] Know the reviewer and have a pushed signed commit marked Verified on GitHub.

After that, arrange a supervised sensor-only bench session using the [hardware instructions](../README.md#first-hardware-session). Hardware validation and flight approval are separate from completing this software onboarding.
