param(
    [Parameter(Mandatory=$true)]
    [string]$GitHubUrl
)
$ErrorActionPreference = "Stop"
git remote remove origin 2>$null
git remote add origin $GitHubUrl
git push -u origin --all
git push origin --tags
Write-Host "All local branches pushed. Check the GitHub branch selector."
