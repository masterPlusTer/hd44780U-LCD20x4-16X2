LCD de 20x4 conectado al arduino nano, sin librerias ni protocolos como spy o i2c, coneccion directa de todos los pines del LCD al arduino nano, conectado tambien el pin RW para poder hacer uso de la busy flag y lectura del contenido del chip del LCD. 

Coneccion de los pines del display a los pines del nano:
display   --- arduino nano
       
       RS --- D9
       RW --- D10
       E  --- D11
       
       D0 --- D2    
       D1 --- D3   
       D2 --- D4   
       D3 --- D5   
       D4 --- D6   
       D5 --- D7   
       D6 --- D8   
       D7 --- D12   

//he intentado conectar el D7 en el D9 del nano para respetar cierto orden pero no se pudo , por algun motivo el RS no quiere //conectar en el D12 asi que por ahora quedo asi
