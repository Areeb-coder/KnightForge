# Publish this repository to GitHub (Windows PowerShell)

1. Extract this ZIP. Open PowerShell in the extracted `MyChessEngine-GitHub-Branches` folder.
2. Confirm Git is installed: `git --version`.
3. Create a **new empty** GitHub repository named `MyChessEngine` (do not initialize it with a README, license, or `.gitignore`).
4. Run the helper, replacing the URL with your own repository URL:

```powershell
.\push_all_branches.ps1 -GitHubUrl "https://github.com/YOUR_USERNAME/MyChessEngine.git"
```

5. If prompted, authenticate with GitHub.
6. Refresh the GitHub repository. The `main` branch is the SAN version; use the branch selector to view the other three snapshots.
7. In GitHub, open **Settings → General → Default branch** and confirm `main` is the default branch if needed.

## Manual alternative

```powershell
git remote add origin https://github.com/YOUR_USERNAME/MyChessEngine.git
git push -u origin --all
```

This ZIP already contains a local `.git` repository with all four branches and their commits. **Do not run `git init` again** and do not upload the ZIP file itself through GitHub's web uploader.
