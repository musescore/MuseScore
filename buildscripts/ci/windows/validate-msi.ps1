# SPDX-License-Identifier: GPL-3.0-only
param([Parameter(Mandatory=$true)][string]$Path)
$ErrorActionPreference = 'Stop'
$installer = $null
$db = $null
try {
    $resolved = (Resolve-Path -LiteralPath $Path).Path
    $installer = New-Object -ComObject WindowsInstaller.Installer
    $db = $installer.OpenDatabase($resolved, 0)
    function Read-Rows([string]$query, [int]$columns) {
        $view = $db.OpenView($query)
        try {
            [void]$view.Execute()
            while ($record = $view.Fetch()) {
                $values = @()
                for ($i=1; $i -le $columns; $i++) { $values += $record.StringData($i) }
                [pscustomobject]@{ Values = $values }
                [void][Runtime.InteropServices.Marshal]::ReleaseComObject($record)
            }
        } finally { [void][Runtime.InteropServices.Marshal]::ReleaseComObject($view) }
    }
    $files = @(Read-Rows 'SELECT `FileName`, `FileSize` FROM `File`' 2)
    if ($files.Count -eq 0) { throw 'MSI contains no application files.' }
    $exe = @($files | Where-Object { $_.Values[0] -match '(^|\|)MuseScore4\.exe$' })
    if ($exe.Count -ne 1 -or [long]$exe[0].Values[1] -le 0) { throw 'MSI does not contain MuseScore4.exe.' }
    $dirs = @(Read-Rows 'SELECT `Directory` FROM `Directory`' 1)
    if (-not ($dirs | Where-Object { $_.Values[0] -eq 'INSTALL_ROOT' })) { throw 'MSI lacks INSTALL_ROOT directory.' }
    $components = @(Read-Rows 'SELECT `Component_` FROM `FeatureComponents`' 1)
    if ($components.Count -eq 0) { throw 'MSI contains no feature component links.' }
    Write-Output ("MSI payload validated: {0} files; executable and install directory present." -f $files.Count)
} catch {
    Write-Error $_
    exit 1
} finally {
    if ($db) { [void][Runtime.InteropServices.Marshal]::ReleaseComObject($db) }
    if ($installer) { [void][Runtime.InteropServices.Marshal]::ReleaseComObject($installer) }
}

