# Turbine Governor

En este proyecto se plantea la creación de un **regulador de turbina hidroeléctrica configurable**. 
Este regulador está diseñado para trabajar con los tipos de turbina más comunes del mercado y ofrecer una configuración totalmente customizable, para su efectiva regulación en los distintos entornos de trabajo.

El regulador de turbina es el encargado de regular la velocidad o carga que puede aportar un generador, mediante el control de una turbina hidráulica mecánicamente acoplada. Este tipo de regulador puede controlar los tipos de turbina **Pelton, Francis y Kaplan**, ofreciendo un control automático para varios modos de regulación o un control manual para mantenimiento, actuando directamente sobre el elemento de control de la turbina elegida.

El control se puede supervisar u operar remotamente desde un sistema SCADA o DCS, o localmente desde un HMI en el regulador. El control local impide el cambio y las acciones del remoto.
El control automático permite el envío de la consigna deseada para cada modo u ordenes al regulador como la de arranque, parada, etc.

Se implementarán como mínimo los siguientes **modos de regulación**:
- Regulación de velocidad con funcionamiento **en vacío**
- Regulación de velocidad con funcionamiento **en isla (red aislada)**
- Regulación de potencia activa **con conexión a red (red interconectada)**
- Regulación **de apertura**

Además incluirá las siguientes funciones:
- Anti-Bumping: Para evitar alteraciones sobre la turbina al cambiar de modo.
- Limitador de apertura: Límites en la salida de los PIDs para evitar alcanzar puntos peligrosos.
- Comparador de velocidad
- Descarga de potencia: Proceso de descarga para desacoplar la turbina de la red.
- Arranque: Proceso de arranque hasta velocidad nominal.
- Parada: Proceso de parada desde velocidad nominal.
- Interlocks y seguridad
- Regulación de posición de elementos de control de la turbina
- Medición de velocidad
- Medición de potencia

El siguiente diagrama de bloques muestra como será el sistema de regulación:

![diagBloques](https://github.com/DaniAlfa/Turbine_Governor/blob/ce5058c8600178d9100e99a69ffd2a84ee00aac9/doc/diag/RegTurbina_DiagramaBloques.png)


Para más información remitirse al documento de [descripción funcional](doc/Reg.Turbina_DescripcionFuncional.docx).

El regulador de turbina ofrecerá entradas y salidas digitales y analógicas para la comunicación con los otros equipos. Esto se hará utilizando un módulo EtherCAT, en concreto se utilizara el BECKHOFF EK1100, aunque la implementación será abierta al uso de otros módulos EtherCAT. Se hará uso de la librería [EtherCAT Master (SOEM)](https://openethercatsociety.github.io/) v1.4.0, cuya licencia es GPLv2 compatible con GPLv3 en este caso, al ser este un trabajo derivado.

Para la comunicación con el sistema SCADA o DCS se utilizará el protocolo MODBUS/TCP usando una interfaz ethernet, tanto para controlar y recibir el estado del regulador como para configurar el mismo. Para ello se hará uso de la librería [libmodbus](https://www.libmodbus.org/) v3.1.6, con licencia LGPL v2.1, compatible con GPLv3.

Para todo el sistema de configuración del regulador se utilizarán ficheros XML, los cuales serán manejados con la librería [libxml](http://www.xmlsoft.org/) v2.9.10, con licencia MIT. 

Despues de estudiar el sistema se plantea la siguiente solución, dividida en módulos independientes, algunos de los cuales podrían estar en una maquina diferente a la del regulador:

![diagArq](https://github.com/DaniAlfa/Turbine_Governor/blob/af39a6fc3ddf9ccb2ced78ba22292241451281ef/doc/diag/Diagrama_ArquitecturaPrincipal_2.png)


Para más información consultar el documento de [arquitectura de software](doc/Reg.Turbina_ArquitecturaSoftware.docx).

Este proyecto lleva parado desde el 06/2020, pero se espera la continuación de su desarrollo próximamente.
El proyecto ya tiene hecha toda la infraestructura principal, menos el módulo de configuración (los ficheros de configuración se generan a mano). También falta la lógica de control (proceso de arranque, sincronización de la turbina, etc). 

En la última compilacion se probó toda la infraestructura, desde la lectura y escritura de valores digitales y analógicos con el modulo físico EtherCAT, hasta su presentación y control desde el HMI, tanto local como remotamente.
La última compilación se llevó a cabo con Qt 5.12.4 y g++ 10.2.0. Ha sido probado en dos Manjaro Linux con kernel 5.12.0, aunque está pensado para ser multiplataforma y debería funcionar en Windows también.
