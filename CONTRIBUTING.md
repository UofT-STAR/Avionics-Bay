# Contributing to firmware

Complete the [README setup checklist](README.md#terminal-setup-checklist) first. Coordinate avionics tasks and firmware review with Tymon Cui (@ccTymon); ask Kai (@KaiYT1) about GitHub access, signing, or workflow errors. If you need signing setup, follow [the guided session](docs/signing.md).

## Make a focused change

1. In GitHub Desktop, select **Avionics-Bay** and fetch changes. Update your branch when your working tree is clean.
2. Create a descriptive feature branch, such as `docs/clarify-setup`. Do not work directly on `main`.
3. Agree on a small task with Tymon, make the change, and inspect every affected file in **Changes**.
4. Run the checks relevant to your change. See [development.md](docs/development.md#testing-and-evidence).
5. Commit only the intended files using your verified signing setup. Publish the branch and confirm GitHub shows **Verified** on the commit. The [signing guide](docs/signing.md#5-make-your-first-real-signed-commit) includes a terminal path if Desktop signing is not yet working.

Keep generated build files, credentials, and personal serial-port changes out of commits. Avoid mixing unrelated formatting changes with behavior changes.

## Open a pull request and get review

1. Open a PR from your feature branch into `main`.
2. Under **Reviewers**, request **UofT-STAR/Avionics**. GitHub may already have requested the team automatically; keep that request. If it is unavailable, ask Kai to check team access. Coordinate a suitable firmware reviewer with Tymon.
3. Include the purpose, affected source/environment, checks performed, results, and anything you could not test. For hardware tests, include the board/revision, wiring reference, setup, and relevant observations.
4. Ask an Avionics teammate other than the author/latest pusher to review and approve. After further pushes, ensure the latest reviewable push receives the required approval before merging.
5. Merge only when GitHub reports the requirements are satisfied. Ask Kai about a blocked merge rather than bypassing the rules.

The current `main` rules require **signed commits**, **at least one approving review**, **an Avionics-team approval**, and **approval of the latest reviewable push by someone other than its pusher**. A single eligible Avionics reviewer can satisfy the review requirements; these do not necessarily mean two approvals. An approval from outside Avionics alone is insufficient. Force pushes and branch deletion are also blocked on `main`.

A build of `test1.cpp` does not validate edits to the excluded `main.cpp`. State this if your change touches it. For documentation-only changes, preview Markdown and check links and claims against the source; hardware tests are not needed unless validating hardware instructions.

## Asking for help

For Git or GitHub issues, contact Kai with your operating system, branch, command/action, expected result, and relevant error. For avionics behavior or hardware, coordinate with Tymon and include the setup and observations. Never include tokens or private keys.
