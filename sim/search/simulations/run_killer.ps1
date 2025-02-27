# Check if the correct number of arguments is passed
if ($args.Length -ne 3) {
    Write-Host "Usage: .\script.ps1 <directory_path> <executable_name> <number_of_files>"
    exit
}

# Get script arguments
$directory = $args[0]
$executableName = $args[1]
$fileCount = [int]$args[2]

# String to search for in the files
$searchString = "Simulation time limit reached"

# Command to execute when the string is found
$stopCommand = "Stop-Process -Name $executableName"

# Infinite loop to keep checking
while ($true) {
    # Get the latest N files in the directory
    $latestFiles = Get-ChildItem -Path $directory -File | Sort-Object LastWriteTime -Descending | Select-Object -First $fileCount
    
    Write-Host "Latest $fileCount files:"
    $latestFiles | ForEach-Object { Write-Host $_.FullName }

    # Check if all files contain the search string
    $allFilesContainString = $true
    foreach ($file in $latestFiles) {
        # $fileContent = Get-Content -Path $file.FullName -ErrorAction SilentlyContinue

        # if ($fileContent -notcontains $searchString) {
        #     Write-Host "'$searchString' not found in file: $($file.FullName)"
        #     $allFilesContainString = $false
        # }
        # else {
        #     Write-Host "'$searchString' found in file: $($file.FullName)"
        # }

        # Get the last 5 lines of the file
        $lastLines = Get-Content -Path $file.FullName -ErrorAction SilentlyContinue | Select-Object -Last 5

        # Check if any of the last 5 lines contain the string "abc"
        $containsStr = $lastLines | Where-Object { $_ -match $searchString }

        if ($containsStr) {
            Write-Host "$($file.FullName)'s last 5 lines contain '$searchString'."
        } else {
            Write-Host "$($file.FullName)'s last 5 lines do not contain '$searchString'."
            $allFilesContainString = $false
        }
    }

    # If all files contain the string, execute the stop process command
    if ($allFilesContainString) {
        Write-Host "All $fileCount files contain the string '$searchString'. Executing command..."
        Invoke-Expression $stopCommand
    }

    # Wait for a while before checking again (e.g., 10 seconds)
    Start-Sleep -Seconds 10
}
