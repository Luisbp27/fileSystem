#!/bin/bash
echo -e $'\n'"\e[91m-- Script Nivel 13 --\e[0m" $'\n'
make clean
make

#Creamos dispositivo
./mi_mkfs disco 100000

#Sacamos la fecha
current_date=`date +%Y%m%d%H%M%S` #yearmonthdayHourMinuteSecond
#Simulamos
echo  -e $'\n'"\e[94m-- Start simulación -- \e[0m"
echo -e  "\e[32mtime ./simulacion disco\e[0m"
time ./simulacion disco 

#Modificamos el string para la verificación
sim_dir="simul_"$current_date #simul_aaaammddhhmmss
echo -e $'\n'"\e[36mDirectorio del script: \e[31m$sim_dir"

#Script para comprimir los archivos y dejarlos listos para entregar
echo -e $'\n'$'\n'"-- Verificación Nivel 13 --"
echo -e "time ./verificacion disco /$sim_dir/\e[0m"
#verificacion <nombre_dispositivo> <directorio_simulación>
time ./verificacion disco /$sim_dir/

#Guardamos el informe en resultado.txt
echo -e $'\n'"./mi_cat disco /$sim_dir/informe.txt > resultado.txt\e[0m"
./mi_cat disco /$sim_dir/informe.txt > resultado.txt

#Revisamos si se ha creado resultado.txt
echo -e $'\n'"ls -l resultado.txt\e[0m"
ls -l resultado.txt

#Mostramos el resultado
echo -e $'\n'"\e[32mcat resultado.txt\e[0m"
cat resultado.txt

#Leemos los 'stats' del disco
echo -e $'\n'"\e[32m./leer_sf disco\e[0m"
./leer_sf disco

make clean