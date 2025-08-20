$principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "Not running as admin. Relaunching with elevation..."
    Start-Process powershell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -NoExit -File `"$PSCommandPath`"" -Verb RunAs
    exit
}

$appPath = Join-Path $PSScriptRoot "build\OctosSetup.msixbundle"
try {
    Add-AppPackage -Path $appPath -AllowUnsigned
} catch {
    $cert = New-SelfSignedCertificate `
        -Type Custom `
        -KeyUsage DigitalSignature `
        -CertStoreLocation "Cert:\CurrentUser\My" `
        -TextExtension @(
            "2.5.29.37={text}1.3.6.1.5.5.7.3.3",
            "2.5.29.19={text}"
        ) `
        -Subject "CN=Underpig" `
        -FriendlyName "Underpig"
    $password = ConvertTo-SecureString -String "PASSWORD" -Force -AsPlainText
    $certPath = "$env:USERPROFILE\Downloads\Underpig.pfx"
    Export-PfxCertificate -Cert "Cert:\CurrentUser\My\$($cert.Thumbprint)" -FilePath $certPath -Password $password
    Write-Host "Certificate exported to $certPath"
    Import-PfxCertificate -CertStoreLocation "Cert:\LocalMachine\TrustedPeople" -Password $password -FilePath $certPath
    $signtool = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x86\signtool.exe"
    & "$signtool" sign /fd SHA256 /a /f "$certPath" /p "PASSWORD" "$appPath"
    # Set-AuthenticodeSignature -FilePath $appPath -Certificate $cert -HashAlgorithm "SHA256" -TimestampServer "http://timestamp.digicert.com"
}