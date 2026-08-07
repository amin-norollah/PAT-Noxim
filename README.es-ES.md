

# PAT-Noxim - el simulador de NoC

![PAT-Noxim - Simulador de NoC de Potencia, Área y Temperatura](pat-noxim.jpg)

![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white)
![SystemC](https://img.shields.io/badge/SystemC-2.2.0-005A9C?style=flat&logo=c&logoColor=white)
![Make](https://img.shields.io/badge/Make-Build-A42E2B?style=flat&logo=gnu&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat&logo=linux&logoColor=black)
![Orion 3.0](https://img.shields.io/badge/Power-Orion%203.0-4B8BBE?style=flat)
![McPAT](https://img.shields.io/badge/PE%20Power%2FArea-McPAT-6A5ACD?style=flat)
![HotSpot 6.0](https://img.shields.io/badge/Thermal-HotSpot%206.0-E25822?style=flat)
![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)

Bienvenido a PAT-Noxim, el simulador de Network-on-Chip (NoC) preciso a nivel de ciclo.

## Descripción

Las Redes en el Chip (NoC) han demostrado ser de baja latencia y altamente escalables en arquitecturas de muchos núcleos. Debido a la importancia de la escalabilidad, los diseñadores intentan optimizar la latencia, la potencia y la temperatura en toda la red. Por lo tanto, desarrollar una herramienta precisa para calcular los atributos mencionados es de suma importancia. Los diseñadores necesitan evaluar sus técnicas propuestas en un entorno simulado de NoC. Así, proponemos PAT-Noxim para abordar las deficiencias en las etapas de diseño y post-diseño. PAT-Noxim, desarrollado sobre la base de Access-Noxim, proporciona un entorno para simular un NoC en modelos de consumo de potencia, área, retardo y temperatura.

PAT-Noxim utiliza un modelo de potencia y térmico que tiene en cuenta los efectos tanto del tráfico de NoC como del entorno circundante. El modelo es altamente configurable, lo que permite a los usuarios ajustar una variedad de parámetros y configuraciones para reflejar sus casos de uso específicos.

Los resultados de los experimentos realizados con PAT-Noxim demuestran su efectividad para modelar con precisión el consumo de potencia y el comportamiento térmico en NoCs. El simulador se puede utilizar para identificar y abordar problemas potenciales relacionados con la gestión de potencia y térmica en NoCs, lo que conduce a un mejor rendimiento y confiabilidad del sistema. En general, PAT-Noxim representa un avance significativo en el campo de la simulación de NoC y tiene el potencial de ser una herramienta valiosa para investigadores y profesionales por igual.

PAT-Noxim se desarrolló para soportar varias arquitecturas predefinidas y personalizadas. Se puede descargar bajo los términos de la licencia GPL.

**Documentación y Comentarios del Código:**

PAT-Noxim se distingue de otros simuladores de NoC por su documentación exhaustiva dentro del código. El código fuente contiene comentarios y explicaciones extensas en todos los componentes principales, lo que lo hace significativamente más accesible para desarrolladores e investigadores. Estos comentarios proporcionan explicaciones detalladas de algoritmos, estructuras de datos, conexiones de señales y detalles de implementación que suelen faltar en otros simuladores. Este nivel de documentación facilita la comprensión de los mecanismos subyacentes, permite modificaciones y extensiones más sencillas y apoya fines tanto educativos como de investigación. Dicha documentación exhaustiva del código es una característica notable que mejora la mantenibilidad y usabilidad en comparación con otros marcos de simulación de NoC disponibles.

**Si utiliza PAT-Noxim en su investigación, agradeceríamos la siguiente cita en cualquier publicación a la que haya contribuido:**

A. Norollah, D. Derafshi, H. Beitollahi and A. Patooghy, "PAT-Noxim: A Precise Power & Thermal Cycle-Accurate NoC Simulator," 2018 31st IEEE International System-on-Chip Conference (SOCC), Arlington, VA, USA, 2018, pp. 163-168. doi: [10.1109/SOCC.2018.8618491](https://doi.org/10.1109/SOCC.2018.8618491)

> Póngase en contacto conmigo por [a.norollah.official@gmail.com](mailto:a.norollah.official@gmail.com)

## Estructura

PAT-Noxim funciona en armonía con tres simuladores diferentes para cubrir los modelos mencionados:

1. Orion 3.0 mide el consumo de potencia y el área de los enrutadores utilizando el modelo Orion. Modificamos este simulador para que funcione con PAT-Noxim.
2. McPAT calcula el área y el consumo de potencia de diferentes elementos de procesamiento, implementados por el diseñador.
3. Hotspot 6.0 recibe el área de los enrutadores y PEs de PAT-Noxim para calcular la temperatura con mayor precisión que Access-Noxim.

## ¿Qué hay de nuevo?

Lista de cambios para la última versión de PAT-Noxim:

1. Agregar canales virtuales
2. Implementar varias arquitecturas de enrutador
3. Capacidad para cambiar arquitecturas de canalización como canalizaciones de 3, 4 y 5 etapas en tiempo de ejecución
4. Agregar señales de crédito a cada enrutador
5. Modelo de potencia mejorado para enrutadores a través de Orion 3.0
6. Modelo de área mejorado para enrutadores a través de Orion 3.0 para medir la temperatura con mayor precisión en Hotspot 6.0
7. Agregar modelos de potencia y área de varios procesadores prácticos a través de McPAT
8. Actualización de la potencia y el área del NoC con cada cambio en la arquitectura del enrutador.
9. Agregar soporte para tecnologías de fabricación de 22, 32, 45, 65, 90 nm.
10. Cálculo de la corriente de fuga mediante Inversión del Efecto de Temperatura (TEI), teniendo en cuenta la corriente de fuga inicial
11. Medición de la corriente de fuga por TEI a través del mejorado Orion 3.0
12. Obtención de retroalimentación de temperatura de los bloques para calcular la corriente de fuga por TEI en un intervalo de tiempo específico.
13. Aumento de la precisión de las mediciones de potencia y térmicas en tecnologías de fabricación sub-90 nm.
14. Informar la temperatura de la superficie del chip una vez cada 100,000 ciclos.
15. Comentarios y documentación exhaustivos del código en todo el código fuente para facilitar la comprensión y modificación
16. Corrección de errores

## Cómo instalar

Primero, debe instalar SystemC 2.2.0 (Siga el enlace: https://github.com/systemc/systemc-2.2.0)

1. Cambie al directorio de nivel superior (systemc-2.2)

2. Cree un directorio temporal, por ejemplo,


    $ mkdir objdir

3. Cambie al directorio temporal, por ejemplo,


    $ cd objdir
    $ sudo apt-get install tcsh
    $ tcsh
    $ setenv CXX g++

4. Configure el paquete para su sistema, por ejemplo,
   (El script de configuración se explica a continuación.)


    $ ../configure

5. Compile el paquete.


    $ gmake
    $ gmake install
    $ cd ..
    $ rm -rf objdir
    $ exit

6. ¡Asegúrese de que el directorio "lib-linux" exista! Si el nombre es "lib-linux64", cambie su nombre a "lib-linux".

Para instalar PAT-Noxim, debe seguir unos pocos pasos sencillos.

    $ cd PAT-Noxim/bin
    $ make
    $ make install

## Documentación exhaustiva

**¡Hemos creado documentación extensa para ayudar tanto a usuarios como a desarrolladores!**

Además de los archivos de documentación detallada, PAT-Noxim cuenta con comentarios de código en línea exhaustivos en todo el código fuente. A diferencia de muchos otros simuladores de NoC que brindan documentación mínima o nula del código, el código fuente de PAT-Noxim contiene comentarios extensos que explican estructuras de datos, algoritmos, conexiones de señales y detalles de implementación. Este nivel de documentación del código hace que el simulador sea significativamente más accesible para desarrolladores e investigadores, lo que permite una comprensión, modificación y extensión más sencillas de la base de código.

Toda la documentación se encuentra en la carpeta **`docs/`** con las siguientes guías:

### Guías de inicio rápido

- **[docs/INDEX.md](docs/INDEX.md)** - Guía de navegación a toda la documentación
- **[docs/INSTALLATION.md](docs/INSTALLATION.md)** - Guía completa de instalación y configuración
- **[docs/RUNNING_SIMULATIONS.md](docs/RUNNING_SIMULATIONS.md)** - Ejemplos desde simulaciones básicas hasta avanzadas

### Configuración y Referencia

- **[docs/CONFIGURATION.md](docs/CONFIGURATION.md)** - Todos los parámetros y opciones de configuración
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - Visión general de la arquitectura y el diseño del sistema

### Para desarrolladores

- **[docs/FILE_GUIDE.md](docs/FILE_GUIDE.md)** - Descripción detallada de cada archivo de código fuente
- **[docs/DEVELOPER_GUIDE.md](docs/DEVELOPER_GUIDE.md)** - Cómo modificar y extender PAT-Noxim
- **[docs/MODELLING.md](docs/MODELLING.md)** - Detalles de los modelos de Potencia (ORION) y Térmico (Hotspot)

### Referencia rápida de archivos clave

| Archivos                      | Descripción                                                                                 |
| -------------------------- | ------------------------------------------------------------------------------------------- |
| NoximParameters.h          | Todas las configuraciones de precompilación se encuentran en este archivo (para PAT-Noxim, Orion y Hotspot)         |
| NoximMain.cpp              | Vincula el simulador a varios componentes                                                    |
| NoximNoC.cpp               | Define la estructura general de la red                                                |
| NoximRouter.cpp            | Incluye arquitecturas de enrutador. Los algoritmos de enrutamiento se definen en este archivo                  |
| NoximProcessingElement.cpp | Incluye la arquitectura del Elemento de Procesamiento (PE) para enviar y recibir mensajes         |
| NoximVLink.cpp             | Determina la política de comunicación entre los bloques en la tercera dimensión             |
| NoximTile.h                | Define y conecta los componentes de un bloque, que consisten en el enrutador y el PE           |
| NoximVCState.cpp           | Define los estados de los canales virtuales en el enrutador                                            |
| NoximPower.cpp             | El consumo de potencia y el área de los componentes de la red se calculan en esta sección |

**Para descripciones completas de los archivos, consulte [docs/FILE_GUIDE.md](docs/FILE_GUIDE.md)**

## Descripción de los componentes del simulador
