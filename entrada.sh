fdisk -Size=300 -path=/home/Disco1.dk -name=Particion1
fdisk -type=E -path=/home/Disco2.dk -Unit=K -name=Particion2 -size=300
fdisk -size=1 -type=L -unit=M -fit=BF -path="/home/mis discos/Disco3.dk"-name="Particion3"
fdisk -type=E -path=/home/Disco2.dk -name=Part3 -Unit=K -size=200
fdisk -delete=fast -name="Particion1" -path=/home/Disco1.dk
fdisk -name=Particion1 -delete=full -path=/home/Disco1.dk
fdisk -add=-500 -size=10 -unit=K -path="/home/misdiscos/Disco4.dk" -name=Particion4
fdisk -add=1 -unit=M -path="/home/mis discos/Disco4.dk" -name="Particion 4"

mount -path=/home/mia/Disco1.dk -name=Particion4
mount -path="/home/mia/primer semestre/entrada1/Disco3.dk" -name=Particion2
rep -id=08A1 -path=/home/samuel/mia/reporte1.jpg -name=mbr
rep -id=08B1 -path=/home/samuel/mia/reporte2.jpg -name=mbr

rep -id=08A1 -path=/home/samuel/mia/reporte11.jpg -name=disk
rep -id=08B1 -path=/home/samuel/mia/reporte22.jpg -name=DIsK

exec -path=/home/samuel/Descargas/prueba.sh
exec -path=/home/samuel/Documentos/GitHub/MIA_Proyecto1_201902308/entrada.script

fdisk -add=1 -path=/home/mia/Disco1.dk -unit=m -name=Particion2
fdisk -add=-500 -path=/home/mia/Disco1.dk -unit=k -name=Particion2