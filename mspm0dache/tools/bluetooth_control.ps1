param(
    [Parameter(Mandatory = $true)]
    [string] $Port,

    [switch] $Run,

    [switch] $Line,

    [switch] $Lap
)

$ErrorActionPreference = "Stop"
$motionOptionCount = @($Run, $Line, $Lap).Where({ $_ }).Count
if ($motionOptionCount -gt 1) {
    throw "Choose only one motion request: -Run, -Line, or -Lap."
}
$serial = [System.IO.Ports.SerialPort]::new(
    $Port,
    9600,
    [System.IO.Ports.Parity]::None,
    8,
    [System.IO.Ports.StopBits]::One
)
$serial.NewLine = "`r`n"
$serial.ReadTimeout = 100
$serial.WriteTimeout = 500

function Read-Reply {
    Start-Sleep -Milliseconds 120
    $text = $serial.ReadExisting()
    if (-not [string]::IsNullOrWhiteSpace($text)) {
        Write-Host $text.TrimEnd()
    }
}

try {
    $serial.Open()
    Start-Sleep -Milliseconds 300
    $serial.DiscardInBuffer()

    $serial.WriteLine("PING")
    Read-Reply
    $serial.WriteLine("STATUS")
    Read-Reply

    if ($motionOptionCount -eq 0) {
        Write-Host "Link test only. Add -Run, -Line (5 s), or -Lap (90 s)."
        return
    }

    Write-Warning "Lift the wheels and keep the physical motor-power switch reachable."
    $serial.WriteLine("ARM")
    Read-Reply
    $motionCommand = if ($Lap) {
        "LAP"
    } elseif ($Line) {
        "LINE"
    } else {
        "START"
    }
    $serial.WriteLine($motionCommand)
    Read-Reply

    $begin = [DateTime]::UtcNow
    $nextStatus = $begin
    $monitorSeconds = if ($Lap) {
        95
    } elseif ($Line) {
        10
    } else {
        24
    }
    while (([DateTime]::UtcNow - $begin).TotalSeconds -lt $monitorSeconds) {
        $serial.WriteLine("KEEP")
        if ([DateTime]::UtcNow -ge $nextStatus) {
            $serial.WriteLine("STATUS")
            $nextStatus = [DateTime]::UtcNow.AddSeconds(1)
        }
        Read-Reply
        Start-Sleep -Milliseconds 300
    }
}
finally {
    if ($serial.IsOpen) {
        try {
            $serial.WriteLine("STOP")
            Read-Reply
        }
        finally {
            $serial.Close()
        }
    }
}
