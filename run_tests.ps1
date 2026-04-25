$base_dir = "testes/testes-t1"
$out_dir = "saida_testes"

# Create output directory
if (-Not (Test-Path $out_dir)) {
    New-Item -ItemType Directory -Force -Path $out_dir | Out-Null
}

$cases = @("c1", "c2", "c3")

foreach ($c in $cases) {
    # Run base case without query
    $out_case = "$out_dir/$c\_base"
    if (-Not (Test-Path $out_case)) { New-Item -ItemType Directory -Force -Path $out_case | Out-Null }
    Write-Host "Running base case for $c..."
    ./ted.exe -e $base_dir -f "$c.geo" -pm "$c.pm" -o $out_case

    # Run with queries
    if (Test-Path "$base_dir/$c") {
        $qry_files = Get-ChildItem -Path "$base_dir/$c" -Filter "*.qry"
        foreach ($qry in $qry_files) {
            $qry_name = $qry.BaseName
            $out_case_qry = "$out_dir/$c\_$qry_name"
            if (-Not (Test-Path $out_case_qry)) { New-Item -ItemType Directory -Force -Path $out_case_qry | Out-Null }
            Write-Host "Running $c with query $qry_name..."
            ./ted.exe -e $base_dir -f "$c.geo" -pm "$c.pm" -q "$c/$($qry.Name)" -o $out_case_qry
        }
    }
}
Write-Host "All tests completed."
