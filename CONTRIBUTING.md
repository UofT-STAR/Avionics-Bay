# Contributing to firmware

Start with [joining and setup](docs/start-here.md) and complete a local build. Coordinate tasks and review with Tymon Cui ([@ccTymon](https://github.com/ccTymon)), the avionics lead. The existing main-branch rules require a pull request, at least one approval, approval of the latest reviewable push by someone other than its pusher, and signed commits. Confirm your signing setup before your first merge; ask Kai ([@KaiYT1](https://github.com/KaiYT1)) for help with Git, GitHub Desktop, signing, access, or PR workflow errors. The team still needs to agree on firmware-specific test requirements.

## Make a focused change

1. In GitHub Desktop, select **Avionics-Bay** and fetch the latest changes. Update your branch when your working tree is clean.
2. Create a branch with a descriptive name, such as `docs/setup-guide` or `fix/sensor-diagnostics`.
3. Agree on a small task with a teammate. Good first tasks include improving a setup explanation or adding clearer diagnostics to the sensor experiment.
4. Make the change and inspect every affected file in GitHub Desktop's **Changes** tab.
5. Run the checks relevant to your change and record the results. See [the testing guide](docs/development.md#testing-and-evidence).
6. Commit the intended files, publish the branch, and open a pull request for review.

Keep generated build files, credentials, and personal serial-port changes out of commits. Avoid mixing unrelated formatting changes with behavior changes.

## Include in your pull request

- What problem you addressed and how behavior changes.
- Which source file and PlatformIO environment you actually built.
- Commands run and their results.
- For hardware tests: board/revision, wiring reference, setup, expected result, actual result, and relevant serial output.
- Checks you could not perform and any remaining questions.

A build of `test1.cpp` does not validate edits to `main.cpp`. State this explicitly if your change touches the excluded program. For documentation-only changes, preview the Markdown and check the paths, links, and claims against the source; hardware tests are unnecessary unless the instructions themselves are being validated.

## Asking for help

Include your operating system, board model, current branch or commit, command or action attempted, and the full relevant error output. Explain what you expected and what happened. Never include tokens or other credentials.
