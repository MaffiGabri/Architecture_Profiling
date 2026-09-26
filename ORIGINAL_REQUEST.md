# Original User Request

## 2026-09-19T14:05:04Z

# Teamwork Project Prompt — Draft

> Status: Launched
> Goal: Craft prompt → get user approval → delegate to teamwork_preview
> Requested team: Full team

Sviluppare un'app per Android (Kotlin/Jetpack Compose) e una compagna per Windows (C++ con framework UI moderno) che permettano all'utente di scoprire il proprio stile architettonico preferito tramite un torneo a scelta tra coppie di immagini. Al termine, l'app calcola e fornisce le categorie e gli stili preferiti con una percentuale di confidenza. Questa fase deve includere 10 immagini mock generate in locale e un JSON che faccia da mock per il futuro DB. 

Working directory: c:/Users/Maffione Gabriele/Progetti e lavori/Programmazione/Architettura
Integrity mode: demo
Repository: https://github.com/MaffiGabri/Architecture_Profiling

## Requirements

### R1. Applicazione Android
Creare un'app Android nativa (Kotlin/Jetpack Compose). L'app deve mostrare la UI per la votazione (torneo a coppie di immagini), scegliere il set di immagini, gestire un account locale (salvataggio progressi/preferenze), mostrare le proprie statistiche e il calcolo di confidenza finale. Deve supportare localizzazione e tema chiaro/scuro.

### R2. Applicazione Windows
Creare un'app desktop Windows in C++ usando una libreria UI moderna. L'app deve avere funzionalità analoghe a quella Android: UI per il torneo, gestione account locale, statistiche utente, punteggi di preferenza, localizzazione e tema chiaro/scuro.

### R3. Dati e Mocking
Generare uno script che crei 10 immagini numerate segnaposto e un file JSON contenente i metadati (categorie, flag, punteggi). Entrambe le app dovranno caricare questi asset per simulare il funzionamento con un database reale.

### R4. Logica del Torneo e Calcolo
Implementare una logica solida per calcolare le preferenze: l'utente sceglie l'immagine preferita tra due opzioni. Al termine (o in tempo reale), calcolare gli stili e categorie favoriti con una % di confidenza basata sulle scelte. Aggiungere anche funzionalità extra utili alla valutazione (il "110%").

## Acceptance Criteria

### Compilazione
- [ ] Il progetto Android si compila correttamente (es. `./gradlew assembleDebug` o equivalente) senza errori.
- [ ] Il progetto Windows si compila correttamente (es. CMake) senza errori.

### Funzionalità e Test
- [ ] Scrittura e superamento di Unit Test automatizzati per la logica matematica di calcolo delle preferenze (la parte più complessa).
- [ ] Generati con successo 10 immagini mock e il file JSON, caricati e letti correttamente dalle due app.
- [ ] Logica del torneo e avanzamento delle coppie di immagini funzionante.
- [ ] Gli agenti hanno controllato a vista/deduzione il codice della UI: deve includere account locale, localizzazione e tema chiaro/scuro.

## 2026-09-26T02:27:11Z

Hello teamwork_preview, the server restarted which halted your execution. The user has provided some new feedback on the project requirements:
1. For the Windows compilation: "Puoi creare delle actions su github e compilare lì direttamente, così da assicurarti che funzioni" (You can create GitHub actions and compile directly there to ensure it works).
2. For the extra features: "Aggiungere anche funzionalità UI e UX utili ad una comoda usufruizione delle app" (Add useful UI and UX features for comfortable use of the apps).

Please integrate this feedback as you proceed with Milestone M5 (Windows Desktop App) and any remaining tasks, and resume your work. Let me know when you reach your next milestone.
