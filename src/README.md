# Fyle System ext2 Linux

## Sistemas Operativos II - University of Balearic Islands

### *Authors:* Jorge Lumbreras Camps & Luis Barca Pons

## *Syntax:*

- leer.c: `./leer <nombre_dispositivo> <numero_inodo>`
- leer_sf.c: `./leer_sf <nombre_dispositivo>`
- mi_cat.c: `./mi_cat </ruta_fichero>`
- mi_chmod.c: `./mi_chmod <disco> <permisos> </ruta>`
- escribir.c: `./escribir <nombre_dispositivo> <"$(cat fichero)"> <diferentes_inodos>`
- mi_link.c: `./mi_link <disco> </ruta_fichero_original> </ruta_enlace>`
- mi_ls.c: `./mi_ls </ruta_directorio>`
- mi_mkdir.c: `./mi_mkdir <disco> <permisos> </ruta>`
- mi_mkfs.c: `./mi_mkfs <device name> <block size>`
- mi_rm: `./mi_rm <disco> </ruta>`
- mi_rmdir: `./mi_rmdir <disco> </ruta>`
- mi_stat: `./mi_stat <disco> </ruta>`
- mi_touch: `./mi_touch <disco> <permisos> </ruta>`
- permitir.c: `./permitir <nombre_dispositivo>`
- truncar.c: `./truncar <nombre_dispositivo>`
- simulacion.c: `./simulacion <nombre_dispositivo>`
- verificacion.c: `./verificacion <disco> <directorio_simulacion>`

## *Improvements:*

- [ ] Mejora librar_bloques_inodo
- [x] Entradas buffer --> buscar_entradas() & mi_dir()
- [x] Diversas mejoras del mi_ls
- [x] Caché directorios LRU & FIFO
- [x] mi_touch & mi_rmdir
- [x] mi_rm_r
- [x] mi_rn
- [x] mi_cp
- [x] mi_mv
- [x] mmap
- [x] Secciones críticas capa de ficheros
- [x] Sellos de tiempo en microsegundos

The final version of the project is in the directory `Entrega 3`
