echo "\033[32mbuild managed plugins\033[m"
cd `dirname $0`
dotnet.exe build -c release -o ../lib/

rm ../lib/Stride*.dll
rm ../lib/VL.Core*.dll