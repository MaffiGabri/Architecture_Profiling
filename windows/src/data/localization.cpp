#include "architecture/localization.hpp"

namespace arch::ui {

const char* const LocalizationManager::kStringTable[static_cast<size_t>(Language::Count)][static_cast<size_t>(StringId::TotalStrings)] = {
    // -------------------------------------------------------------------------
    // ENGLISH (Language::EN = 0)
    // -------------------------------------------------------------------------
    {
        /* AppName */                        "Architecture Profiling",
        /* AppTagline */                     "Discover Your Architectural Identity",
        /* NavWelcome */                     "Welcome",
        /* NavTournament */                  "Tournament",
        /* NavResults */                     "Results",
        /* NavHistory */                     "History",
        /* NavSettings */                    "Settings",
        /* WelcomeHeadline */                "Architectural Style Tournament",
        /* WelcomeSubtitle */                "Discover your aesthetic personality through pairwise comparison of 10 iconic architectural movements.",
        /* SelectModePrompt */               "Select Tournament Format:",
        /* ModeFullTitle */                  "Full Tournament (45 Matches)",
        /* ModeFullDesc */                   "Complete round-robin evaluation of all 45 pairings. Maximizes statistical confidence, Bradley-Terry affinity estimation, and Kendall transitivity consistency.",
        /* ModeQuickTitle */                 "Quick Profiling (15 Matches)",
        /* ModeQuickDesc */                  "Rapid 15-match tournament balancing speedy profiling with transitivity verification. Ideal for a swift aesthetic impression.",
        /* BtnStartTournament */             "Begin Tournament",
        /* TournamentStepFmt */              "Match %d of %d",
        /* TournamentPrompt */               "Which architectural aesthetic resonates more with you?",
        /* BtnVoteLeft */                    "Vote Option A [1]",
        /* BtnVoteRight */                   "Vote Option B [2]",
        /* BtnUndo */                        "Undo (Ctrl+Z)",
        /* BtnRedo */                        "Redo (Ctrl+Y)",
        /* BtnRestart */                     "Restart",
        /* DialogRestartTitle */             "Restart Tournament?",
        /* DialogRestartDesc */              "Are you sure you want to restart? All current match votes will be cleared.",
        /* ResultsTitle */                   "Your Architectural Aesthetic Profile",
        /* ResultsChampionLabel */           "🏆 Champion Style",
        /* ResultsConfidenceBadgeFmt */      "Confidence: %.1f%%",
        /* ResultsConsistencyFmt */          "Decision Consistency (zeta): %.3f",
        /* ResultsMarginFmt */               "Victory Margin (Delta): +%.1f%%",
        /* ResultsTriadsFmt */               "Contradictions (Triads): %d",
        /* ResultsRadarHeader */             "5-Axis Aesthetic Radar Profile",
        /* ResultsCategoryBreakdownHeader */ "Category Affinity Breakdown",
        /* ResultsStyleRankingsHeader */     "Complete Style Rankings",
        /* BtnSaveHistory */                 "Saved to History",
        /* BtnViewHistory */                 "View History",
        /* BtnRetakeTournament */            "Retake Tournament",
        /* HistoryTitle */                   "Tournament History & Past Insights",
        /* HistoryEmptyTitle */              "No Tournaments Completed Yet",
        /* HistoryEmptyDesc */               "Complete a tournament to generate your architectural aesthetic history and track how your tastes evolve.",
        /* BtnClearHistory */                "Clear All History",
        /* SettingsTitle */                  "Preferences & Customization",
        /* SettingsThemeMode */              "Color Theme",
        /* ThemeLightLabel */                "Travertine Light",
        /* ThemeDarkLabel */                 "Deep Basalt Dark",
        /* SettingsLanguage */               "Language",
        /* BtnConfirm */                     "Confirm",
        /* BtnCancel */                      "Cancel",
        /* RadarAxisEra */                   "Era",
        /* RadarAxisOrnamentation */         "Ornamentation",
        /* RadarAxisStructuralHonesty */     "Structural Honesty",
        /* RadarAxisGeometricOrder */        "Geometric Order",
        /* RadarAxisMaterialWarmth */        "Material Warmth",
        /* UserAccountLabel */               "User Profile",
        /* CreateUserLabel */                "New Profile Name:",
        /* ActiveUserLabel */                "Active User",
        /* BtnNewProfile */                  "Create Profile",
        /* ShortcutsTitle */                 "Keyboard Shortcuts",
        /* ShortcutsKeyHeader */             "Shortcut Key",
        /* ShortcutsActionHeader */          "Assigned Action",
        /* ShortcutsVoteA */                 "Vote for Option A (Left)",
        /* ShortcutsVoteB */                 "Vote for Option B (Right)",
        /* ShortcutsUndo */                  "Undo last match vote",
        /* ShortcutsRedo */                  "Redo previously undone vote",
        /* ShortcutsTheme */                 "Toggle Theme (Light / Dark)",
        /* ShortcutsLang */                  "Toggle Language (EN / IT)",
        /* ShortcutsBack */                  "Back / Dismiss dialog / Cancel",
        /* ShortcutsHelp */                  "Open this Keyboard Shortcuts cheat sheet",
        /* DialogClearHistoryTitle */        "Clear All History?",
        /* DialogClearHistoryDesc */         "Are you sure you want to delete all saved tournament runs? This action cannot be undone.",
        /* DialogAbandonTitle */             "Abandon Tournament?",
        /* DialogAbandonDesc */              "A tournament is currently in progress. Exiting to the welcome screen will discard current match votes.",
        /* BtnResumeTournament */            "Continue Tournament",
        /* BtnAbandonTournament */           "Abandon & Exit",
        /* BtnConfirmDelete */               "Confirm Delete"
    },

    // -------------------------------------------------------------------------
    // ITALIAN (Language::IT = 1)
    // -------------------------------------------------------------------------
    {
        /* AppName */                        "Profilazione Architettonica",
        /* AppTagline */                     "Scopri la Tua Identità Architettonica",
        /* NavWelcome */                     "Benvenuto",
        /* NavTournament */                  "Torneo",
        /* NavResults */                     "Risultati",
        /* NavHistory */                     "Cronologia",
        /* NavSettings */                    "Impostazioni",
        /* WelcomeHeadline */                "Torneo di Stili Architettonici",
        /* WelcomeSubtitle */                "Scopri la tua personalità estetica attraverso il confronto a coppie di 10 movimenti architettonici iconici.",
        /* SelectModePrompt */               "Seleziona la Modalità del Torneo:",
        /* ModeFullTitle */                  "Torneo Completo (45 Confronti)",
        /* ModeFullDesc */                   "Valutazione round-robin esaustiva di tutte le 45 coppie. Massimizza la confidenza statistica, la stima di affinità Bradley-Terry e la coerenza di transitività di Kendall.",
        /* ModeQuickTitle */                 "Profilazione Rapida (15 Confronti)",
        /* ModeQuickDesc */                  "Torneo rapido di 15 confronti che bilancia velocità di profilazione e verifica di transitività. Ideale per una rapida impressione estetica.",
        /* BtnStartTournament */             "Inizia il Torneo",
        /* TournamentStepFmt */              "Confronto %d di %d",
        /* TournamentPrompt */               "Quale estetica architettonica risuona di più con te?",
        /* BtnVoteLeft */                    "Vota Opzione A [1]",
        /* BtnVoteRight */                   "Vota Opzione B [2]",
        /* BtnUndo */                        "Annulla (Ctrl+Z)",
        /* BtnRedo */                        "Ripeti (Ctrl+Y)",
        /* BtnRestart */                     "Ricomincia",
        /* DialogRestartTitle */             "Ricominciare il Torneo?",
        /* DialogRestartDesc */              "Sei sicuro di voler ricominciare? Tutti i voti del torneo attuale andranno persi.",
        /* ResultsTitle */                   "Il Tuo Profilo Estetico Architettonico",
        /* ResultsChampionLabel */           "🏆 Stile Campione",
        /* ResultsConfidenceBadgeFmt */      "Confidenza: %.1f%%",
        /* ResultsConsistencyFmt */          "Coerenza Decisionale (zeta): %.3f",
        /* ResultsMarginFmt */               "Margine di Vittoria (Delta): +%.1f%%",
        /* ResultsTriadsFmt */               "Contraddizioni (Triadi): %d",
        /* ResultsRadarHeader */             "Profilo Radar Estetico su 5 Assi",
        /* ResultsCategoryBreakdownHeader */ "Distribuzione di Affinità per Categoria",
        /* ResultsStyleRankingsHeader */     "Classifica Completa degli Stili",
        /* BtnSaveHistory */                 "Salvato nella Cronologia",
        /* BtnViewHistory */                 "Vedi Cronologia",
        /* BtnRetakeTournament */            "Ripeti Torneo",
        /* HistoryTitle */                   "Cronologia Tornei e Analisi Precedenti",
        /* HistoryEmptyTitle */              "Nessun Torneo Ancora Completato",
        /* HistoryEmptyDesc */               "Completa un torneo per generare la tua cronologia estetica e osservare l'evoluzione dei tuoi gusti.",
        /* BtnClearHistory */                "Cancella Tutta la Cronologia",
        /* SettingsTitle */                  "Preferenze e Personalizzazione",
        /* SettingsThemeMode */              "Tema Visivo",
        /* ThemeLightLabel */                "Travertino Chiaro",
        /* ThemeDarkLabel */                 "Basalto Scuro",
        /* SettingsLanguage */               "Lingua",
        /* BtnConfirm */                     "Conferma",
        /* BtnCancel */                      "Annulla",
        /* RadarAxisEra */                   "Epoca",
        /* RadarAxisOrnamentation */         "Ornamentazione",
        /* RadarAxisStructuralHonesty */     "Onestà Strutturale",
        /* RadarAxisGeometricOrder */        "Ordine Geometrico",
        /* RadarAxisMaterialWarmth */        "Calore dei Materiali",
        /* UserAccountLabel */               "Profilo Utente",
        /* CreateUserLabel */                "Nome Nuovo Profilo:",
        /* ActiveUserLabel */                "Utente Attivo",
        /* BtnNewProfile */                  "Crea Profilo",
        /* ShortcutsTitle */                 "Scorciatoie da Tastiera",
        /* ShortcutsKeyHeader */             "Tasto Scorciatoia",
        /* ShortcutsActionHeader */          "Azione Assegnata",
        /* ShortcutsVoteA */                 "Vota Opzione A (Sinistra)",
        /* ShortcutsVoteB */                 "Vota Opzione B (Destra)",
        /* ShortcutsUndo */                  "Annulla ultimo voto del confronto",
        /* ShortcutsRedo */                  "Ripristina voto precedentemente annullato",
        /* ShortcutsTheme */                 "Cambia Tema (Chiaro / Scuro)",
        /* ShortcutsLang */                  "Cambia Lingua (Italiano / Inglese)",
        /* ShortcutsBack */                  "Indietro / Chiudi dialogo / Annulla",
        /* ShortcutsHelp */                  "Apri questo riepilogo delle scorciatoie",
        /* DialogClearHistoryTitle */        "Cancellare Tutta la Cronologia?",
        /* DialogClearHistoryDesc */         "Sei sicuro di voler eliminare tutti i tornei salvati? L'operazione è definitiva e non potrà essere annullata.",
        /* DialogAbandonTitle */             "Abbandonare il Torneo?",
        /* DialogAbandonDesc */              "C'è un torneo in corso. Uscire alla schermata iniziale comporterà la perdita dei voti del torneo attuale.",
        /* BtnResumeTournament */            "Continua Torneo",
        /* BtnAbandonTournament */           "Abbandona ed Esci",
        /* BtnConfirmDelete */               "Conferma Eliminazione"
    }
};

const char* LocalizationManager::get(StringId id) const noexcept {
    auto lang_idx = static_cast<size_t>(current_lang_);
    auto str_idx = static_cast<size_t>(id);
    if (lang_idx < static_cast<size_t>(Language::Count) && str_idx < static_cast<size_t>(StringId::TotalStrings)) {
        return kStringTable[lang_idx][str_idx];
    }
    return "<missing_string>";
}

const char* LocalizationManager::get_radar_axis_label(int axis_index) const noexcept {
    static constexpr StringId kAxes[5] = {
        StringId::RadarAxisEra,
        StringId::RadarAxisOrnamentation,
        StringId::RadarAxisStructuralHonesty,
        StringId::RadarAxisGeometricOrder,
        StringId::RadarAxisMaterialWarmth
    };
    if (axis_index >= 0 && axis_index < 5) {
        return get(kAxes[axis_index]);
    }
    return "";
}

} // namespace arch::ui
