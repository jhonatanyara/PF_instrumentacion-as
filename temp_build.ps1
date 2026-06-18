Set-Location 'C:\Users\Jhona\Desktop\Instrumentacion\softap_sta'
$env:IDF_PATH='C:\esp\v6.0.1\esp-idf'
$env:PATH='C:\Users\Jhona\.espressif\python_env\idf6.0_py3.14_env\Scripts;' + $env:PATH
& 'C:\esp\v6.0.1\esp-idf\export.ps1'
idf.py build
