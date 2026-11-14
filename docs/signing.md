# Set up commit signing with Kai

Use this guide during your first setup session with **Kai ([@KaiYT1](https://github.com/KaiYT1))**, who handles GitHub support. Tymon coordinates avionics tasks and firmware review. Signing proves who signed a commit; it does not log you into GitHub or grant repository access.

This walkthrough is for **Windows PowerShell and SSH signing**. On macOS/Linux, or if you already have working GPG/SSH signing, keep your setup and ask Kai to check it rather than replacing it. No GitHub CLI is needed.

Run one block at a time in the Avionics-Bay terminal. Example outputs are illustrative. Complete the README's identity checks first. These instructions configure this repository only.

## 1. Check the tools and existing configuration

**Run:**

```powershell
git --version
Get-Command ssh-keygen
```

**Expect:** a Git version and a command entry for `ssh-keygen`. SSH signing needs Git 2.34 or newer. If either tool is unavailable, ask Kai to finish the Git/OpenSSH installation before continuing.

**Then run:**

```powershell
git config --get gpg.format
git config --get user.signingkey
```

**Expect:** either existing settings or no output. A blank format defaults to OpenPGP. If a key is already configured, have Kai check it; skip key creation if it works. Do not replace another project's signing setup.

## 2. Create a dedicated key only if you need one

The example uses a dedicated filename to avoid overwriting a normal SSH login key.

**Check both filenames first:**

```powershell
Test-Path "$env:USERPROFILE/.ssh/uoft_star_signing"
Test-Path "$env:USERPROFILE/.ssh/uoft_star_signing.pub"
```

**Expect for a new setup:** `False` twice. If either returns `True`, stop and ask Kai to inspect the existing key. Do not overwrite it. If you use another filename, use that same filename throughout the guide.

**Create the folder if needed:**

```powershell
New-Item -ItemType Directory -Force "$env:USERPROFILE/.ssh" | Out-Null
```

**Expect:** no output on success.

**Generate the key:**

```powershell
ssh-keygen -t ed25519 -C "UofT STAR commit signing" -f "$env:USERPROFILE/.ssh/uoft_star_signing"
```

**Expect:** prompts for a passphrase and its confirmation, then a fingerprint. Choose a passphrase; typing it may not display characters. If an overwrite prompt appears, answer **no** and stop. Keep the file without `.pub` private; only the `.pub` file is shared with GitHub.

## 3. Register the public key as a signing key

**Copy only the public key:**

```powershell
Get-Content "$env:USERPROFILE/.ssh/uoft_star_signing.pub" | Set-Clipboard
```

**Expect:** no output; the public key is now on your clipboard.

On GitHub, open **Settings > SSH and GPG keys > New SSH key**. Choose **Signing Key** as the key type, give it a recognizable title such as “Laptop – UofT STAR signing,” paste, and save. An Authentication Key entry alone does not register it for signing. Check you are signed into your own account.

## 4. Configure this repository

These commands use the private-key path so the terminal can request its passphrase directly; an SSH agent is not required for this path. No private-key contents are copied into Git configuration or the repository.

**Run:**

```powershell
git config --local gpg.format ssh
git config --local user.signingkey "$env:USERPROFILE/.ssh/uoft_star_signing"
```

**Expect:** no output on success.

**Then run:**

```powershell
git config --local gpg.ssh.program (Get-Command ssh-keygen).Source
git config --local commit.gpgsign true
```

**Expect:** no output. The first command selects the same signing tool you checked earlier.

**Check the result:**

```powershell
git config --get gpg.format
git config --get commit.gpgsign
```

**Example output:**

```text
ssh
true
```

GitHub Desktop may need additional passphrase/agent setup on your machine. Use the terminal for the first signed commit below; Kai can then help check signing in Desktop if that is how you want to commit regularly.

## 5. Make your first real signed commit

Agree on a small task with Tymon. Create a feature branch in GitHub Desktop, make the edit, and review it there. The following example assumes your task changes **only `README.md`**; otherwise substitute the exact intended file path. Do not stage unrelated files.

**Run:**

```powershell
git branch --show-current
git status --short
```

**Example output:**

```text
docs/clarify-setup
 M README.md
```

**Continue only if:** you are on your feature branch, not `main`, and understand every listed change.

**Stage and inspect the intended change:**

```powershell
git add -- README.md
git diff --cached
```

**Expect:** a diff showing exactly the edit you intend to commit. If the display opens in a pager, press `q` to leave it. Stop if unrelated changes are included.

**Commit with signing:**

```powershell
git commit -S -m "docs: clarify setup instructions"
```

Use a message matching your task. **Expect:** a key-passphrase prompt when needed, then a commit summary. On a signing error, ask Kai for help; do not disable signing or remove the key's passphrase to bypass the error.

## 6. Check GitHub's verification

In GitHub Desktop, publish or push your feature branch. Open its latest commit on GitHub and check for **Verified**. This is the completion checkpoint: local configuration or a successful commit alone does not prove GitHub accepts the signature.

If the badge is absent or Unverified, click its details and ask Kai to check the account, key type, and commit email. Do not create extra test commits or rewrite published history without coordinating. A local SSH verification command may also require an allowed-signers file; that is separate from GitHub's verification.

Continue with [the PR and review instructions](../CONTRIBUTING.md#open-a-pull-request-and-get-review). Signing and a successful push do not replace the required Avionics-team approval.

References: [GitHub signing configuration](https://docs.github.com/en/authentication/managing-commit-signature-verification/telling-git-about-your-signing-key), [Git signing-key configuration](https://git-scm.com/docs/git-config#Documentation/git-config.txt-usersigningKey), and [commit-signing behavior](https://docs.github.com/en/authentication/managing-commit-signature-verification/signing-commits).
