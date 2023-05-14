# Sistema de Ficheros

## Entrega 2 - Sistemas Operativos II

### Universidad de las Islas Baleares

## *Autores:*

- Jorge Lumbreras Camps
- Lluis Barca Pons

## *Sintaxi:*

- escribir.c: `escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>`
- leer.c: `leer <nombre_dispositivo> <numero_inodo>`
- permitir.c: `permitir <nombre_dispositivo> <ninodo> <permisos>`
- leer_sf.c: `leer_sf <nombre_dispositivo>`
- mi_fks.c : `mi_fks <nombre del fichero> <numero de bloques>`

## *Mejoras:*

- Entradas buffer --> buscar_entradas() & mi_dir()
- Diversas mejoras del mi_ls
- Caché directorios
- mi_touch & mi_rmdir
