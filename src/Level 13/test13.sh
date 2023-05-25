# Luis Barca
# test13.sh
clear
make clean
make

echo -e "\x1B[38;2;17;245;120m##############################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120m                          SIMULACIÓN\x1b[0m"
echo -e "\x1B[38;2;17;245;120m##############################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkfs disco 100000\x1b[0m"
./mi_mkfs disco 100000
echo
echo -e "\x1B[38;2;17;245;120m$ ./simulacion disco /simul_20210524105645\x1b[0m"
./simulacion disco /simul_20210524105645
echo
echo -e "\x1B[38;2;17;245;120m$ time ./verificacion disco /simul_20210524105645\x1b[0m"
time ./verificacion disco /simul_20210524105645
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /simul_20210524105645/informe.txt > resultado.txt\x1b[0m"
./mi_cat disco /simul_20210524105645/informe.txt > resultado.txt
echo
echo -e "\x1B[38;2;17;245;120m$ ls -l resultado.txt\x1b[0m"
ls -l resultado.txt
echo
echo -e "\x1B[38;2;17;245;120m$ cat resultado.txt\x1b[0m"
cat resultado.txt
echo
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco
echo

make clean