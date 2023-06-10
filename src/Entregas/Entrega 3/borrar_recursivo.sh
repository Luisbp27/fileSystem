echo "$ #Borrado recursivo de un directorio no vacío"

echo "$ ./mi_rm_r disco    /dir3/"
./mi_rm disco /dir3/ -r
echo "$ ./leer_sf disco"
./leer_sf disco

echo "$ ./mi_rm_r disco /dir2/"
./mi_rm disco /dir2/ -r
echo "$ ./leer_sf disco"
./leer_sf disco

echo "$ ./mi_rm_r disco /dir1/"
./mi_rm disco /dir1/ -r
echo "$ ./leer_sf disco"
./leer_sf disco