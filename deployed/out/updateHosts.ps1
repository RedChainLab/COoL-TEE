# Define the directory containing the JSON files and the base hosts file
$configFolder = "config"
$hostsBaseFile = "C:\Windows\System32\drivers\etc\hosts.base"
$hostsFile = "hosts"

# Copy the contents of hosts.base to the new hosts file
Copy-Item -Path $hostsBaseFile -Destination $hostsFile -Force

# Get all JSON files in the config folder
$jsonFiles = Get-ChildItem -Path $configFolder -Filter *.json

# Loop through each JSON file
foreach ($file in $jsonFiles) {
    # Extract the filename without the extension
    $hostname = [System.IO.Path]::GetFileNameWithoutExtension($file.Name)
    
    # Read the content of the JSON file
    $jsonContent = Get-Content -Path $file.FullName | Out-String | ConvertFrom-Json
    
    # Extract the value of publicIpAddress
    $publicIpAddress = $jsonContent.publicIpAddress
    
    # Prepare the <IP, hostname> pair line
    $line = "$publicIpAddress    $hostname"
    
    # Append the line to the hosts file
    Add-Content -Path $hostsFile -Value $line
}

Write-Output "Hosts file created successfully with updated entries."