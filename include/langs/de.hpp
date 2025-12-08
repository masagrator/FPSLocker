#include <array>

namespace GER {
	const std::array strings = {
		//Main Menu
		"Einstellungen löschen",
		"Patches löschen",
		"SaltyNX funktioniert nicht!",
		"NX-FPS Plugin wurde nicht\nauf SD-Karte gefunden!",
		"Spiel läuft nicht!",
		"Alle",
		"Alles",
		"Spielliste",
		"Anzeigeeinstellungen",
		"Spiel wurde geschlossen.\nOverlay deaktiviert!",
		"Spiel läuft nicht.\nOverlay deaktiviert!",
		"Spiel läuft.",
		"NX-FPS läuft nicht!",
		"NX-FPS läuft, wartet für einen Frame.",
		"Starte Overlay neu zum Überprüfen.",
		"NX-FPS läuft.",
		"Patch erzwingt nun 60 Hz.",
		"Erhöhe FPS-Ziel",
		"Senke FPS-Ziel",
		"Ändere FPS-Ziel",
		"Benutzerdefiniertes FPS-Ziel deaktivieren",
		"Erweiterte Einstellungen",
		"Interval Modus:\n\uE019 0 (Unbenutzt)",
		"Interval Modus:\n\uE019 %d (%.1f FPS)",
		"Interval Modus:\n\uE019 %d (%d FPS)",
		"Interval Modus:\n\uE019 %d (Falsch)",
		"Benutzerdefiniertes FPS-Ziel:\n\uE019 Deaktiviert",
		"Benutzerdefiniertes FPS-Ziel:\n\uE019 %d",

		//Advanced Settings
		"Puffer festlegen",
		"Es wird beim nächsten Spielstart angewendet.",
		"Doppelt",
		"Dreifach",
		"Dreifach (erzwungen)",
		"Vierfach (erzwungen)",
		"Vierfach",
		"NVN Window Sync Wait",
		"Modus",
		"Aktiviert",
		"Semi-Aktiviert",
		"Deaktiviert",
		"An",
		"Aus",
		"Semi",
		"Konfigurations-Datei nicht gefunden\nTID: %016lX\nBID: %016lX",
		"Konfiguration Fehler: 0x%X",
		"Patch-Datei existiert nicht.\nUse \"Konvertiere Konfiguration zu Patch-Datei\"\nzur Erstellung!",
		"Patch-Datei existiert nicht.",
		"Neue Konfiguration erfolgreich\nheruntergeladen.\nBenutze \"Konvertiere Konfiguration zu\nPatch-Datei\" um es verwendbar zu machen!",
		"Patch-Datei existiert.",
		"GPU API Interface: NVN",
		"Window Sync Wait",
		"GPU API Interface: EGL",
		"GPU API Interface: Vulkan",
		"FPSLocker Patches",
		"Ein!",
		"Starte Spiel nach der Konvertierung neu!",
		"Konvertiere Konfiguration, um Datei zu patchen",
		"Patch-Datei wurde erfolgreich erstellt.\nNeustarte das Spiel und ändere\nFPS-Ziel um den Patch anzuwenden!",
		"Fehler bei Patcherstellung: 0x%x",
		"Lösche Patch-Datei",
		"Patch-Datei erfolgreich gelöscht.",
		"Das kann bis zu %d Sekunden dauern.",
		"Konfigurations-Datei überprüfen/herunterladen",
		"Überprüfe Warehouse für Konfiguration...",
		"Sonstiges",
		"Unfokussiertes Spiel stoppen",
		"Patch wurde in das Spiel geladen.",
		"Master Write wurde in das Spiel geladen.",
		"Plugin konnte Patch nicht anwenden.",
		"Set/Aktive/Verfügbare Puffer: %d/%d/%d",
		"Aktive Puffer: %d",
		"Verbindungstimeout!",
		"Konfiguration ist nicht verfügbar!\nRC: 0x%x",
		"Konfiguration ist nicht verfügbar!\nÜberprüfung des Warehouse\nfür weitere Informationen...",
		"Konfiguration ist nicht verfügbar!\nÜberprüfung des Warehouse\nfür weitere Informationen...\nTimeout! Überprüfung dauerte zu lange.",
		"Konfiguration ist nicht verfügbar!\nÜberprüfung des Warehouse\nfür weitere Informationen...\nVerbindungsfehler!",
		"Keine neue Konfiguration verfügbar.",
		"Internetverbindung nicht verfügbar!",
		"Patch wird für dieses Spiel nicht benötigt!",
		"Dieses Spiel ist nicht im Warehouse aufgelistet!",
		"Dieses Spiel ist im Warehouse gelistet,\naber mit einer anderen Version.\n%s braucht keinen Patch,\ndeine Version vielleicht auch nicht!",
		"Dieses Spiel ist im Warehouse gelistet,\naber mit einer anderen Version.\n%s empfiehlt einen Patch,\naber Konfiguration ist nicht verfügbar!",
		"Dieses Spiel ist im Warehouse gelistet,\naber mit einer anderen Version.\n%s hat Patch verfügbar!",
		"Dieses Spiel ist im Warehouse gelistet\nmit einem empfohlenen Patch für \ndiese Version,\naber Konfiguration ist nicht verfügbar!",
		"Verbindungsfehler! RC: 0x%x",

		//Display Settings
		"Frameskip Tester",

		"Benutzung:\n"
		"1. Verwende eine Kamera mit\n"
		"Optionen: Manuelle Einstellung\n"
		"der Belichtungszeit und ISO.\n"
		"2. Setze Belichtungszeit zu 1/10s\n"
		"oder länger und ISO so,\n"
		"dass es nicht zu hell oder dunkel\n"
		"ist (normalerweise etwa 50 für\n"
		"1/10s).\n"
		"3. Drücke \uE0E0 um fortzusetzen.\n"
		"4. Mache ein Foto vom Display.\n"
		"5. Wenn alle Blöcke außer dem\n"
		"ersten und dem letzten verschieden\n"
		"hell sind, unterstützt dein Display\n"
		"die aktuelle Wiederholungsrate\n"
		"nicht nativ und nutzt eine\n"
		"andere Wiederholungsrate.\n\n"
		"Bedenke aber, dass selbst wenn\n"
		"das Display Bilder überspringt,\n"
		"es sieht immer noch besser aus als\n"
		"niedrigere FPS-Ziele, die nicht\n"
		"der Wiederholungsrate\n"
		"entsprechen, da Hardwarelösungen\n"
		"die besten Methoden hierfür sind.",

		"Drücke \uE0E1 zum Beenden",
		"Rendering dauert zu lange!\nSchließe Spiel, gehe zum Homescreen,\nversuche erneut.",

		"Dieses Menü geht durch alle\n"
		"unterstützten Wiederholungsraten unter 60 Hz:\n"
		"40, 45, 50, 55. Drücke den Knopf den du\n"
		"gefragt wirst zur Bestätigung, dass es funktioniert.\n"
		"Falls für 15 Sekunden nichts gedrückt wird,\n"
		"wird für die nächste Wiederholungsrate überprüft.",

		"Zum Starten, drücke X.",
		"Display-Underclock-Assistent",
		"Nicht unterstützt bei %dp!",
		"Beende Spiel zuerst!",
		"Drücke ZL um zu bestätigen, dass %d Hz funktioniert.",
		"Drücke X um zu bestätigen, dass %d Hz funktioniert.",
		"Drücke Y um zu bestätigen, dass %d Hz funktioniert.",
		"Drücke ZR um zu bestätigen, dass %d Hz funktioniert.",

		"Dieses Menü geht durch alle\n"
		"unterstützten Wiederholungsraten über 60 Hz:\n"
		"bis zu %d Hz.\n\n"
		"Drücke Knöpfe die du gefragt wirst\n"
		"um zu bestätigen, dass es funktioniert.\n"
		"Falls für 10 Sekunden nichts gedrückt wird,\n"
		"wird für die nächste Wiederholungsrate überprüft.\n"
		"Das kann bis zu %d Sekunden benötigen.",

		"Display-Overclock-Assistent",
		"Docked %dp Display manuelle Einstellungen",
		"Docked Display weitere Einstellungen",
		"Erlaube Patches um 60 Hz zu erwzingen",
		"Nutze niedrigste Wiederholungsrate für nicht übereinstimmende FPS-Ziele",
		"n/d",
		"Maximale Wiederholungsrate verfügbar: %u Hz\nmyDP Link Rate: %s\nKonfigurations ID: %08X",
		"Docked Display Einstellungen",

		"Erlaube %dp Wiederholungsraten",
		"%dp Overclock-Assistent",
		"Weitere Einstellungen",

		"Du bist nicht in docked Modus.\n"
		"Docke deine Switch\n"
		"und komme zurück.",

		"Ändere Wiederholungsrate",
		"Erhöhe Wiederholungsrate",
		"Senke Wiederholungsrate",
		"Wiederholungsrate mit FPS-Ziel abgleichen.",
		"Handheld-Displaysynchronisierung",
		"Docked Einstellung",
		"Retro Remake Modus",
		"Display Wiederholungsrate: %d Hz",

		"DAS IST EINE EXPERIMENTELLE\nFUNKTION!\n\n"
		"Es kann zu permanentem Schaden\n"
		"am Display führen.\n\n"
		"Durch Drücken von Akzeptieren\n"
		"übernimmst duvolle Verantwortung\n"
		"für alles, das wegen diesem Tool\n"
		"passieren kann.",

		"Display Einstellungen Warnung",
		"Ablehnen",
		"Akzeptieren",
		"Erzwinge Englische Sprache",
		"Dock-Displaysynchronisierung",
		"Nur Handheld",
		"60 Hz im Homemenü"
	};

	const std::array teslaStrings = {
	   "Zurück",
	   "OK"
	};

	static_assert(teslaStrings.size() == 2);
}
