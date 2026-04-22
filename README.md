# Particle Physics Sandbox (SFML)

Un simulatore interattivo di particelle scritto in **C++** con **SFML**. Questo progetto dimostra l'uso della matematica vettoriale e della fisica applicata allo sviluppo di videogiochi.

## Funzionalità
- **Fisica Realistica**: Gravità, attrito dell'aria e rimbalzi elastici.
- **Collisioni**: Rimbalzi su ostacoli circolari tramite riflessione vettoriale (Prodotto Scalare).
- **Interazione**: Clicca con il mouse per generare esplosioni radiali che respingono le particelle.
- **Ottimizzazione**: Sistema di riciclo delle particelle (Object Pooling) per mantenere alte prestazioni.
- **Grafica**: Effetto scia e colori dinamici basati sulla velocità.

## Requisiti
- Compilatore C++ (C++17 o superiore)
- Libreria SFML 3.0+

## Controlli
- **Mouse SX**: Genera una spinta esplosiva dalla posizione del cursore.
