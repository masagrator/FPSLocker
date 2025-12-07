#include <array>

namespace FRA {
	const std::array strings = {
		//Main Menu
		"Supprimer les paramètres",
		"Supprimer les patches",
		"SaltyNX ne fonctionne pas!",
		"Aucun plugin NX-FPS trouvé\nsur la carte SD!",
		"Aucun Jeu en cours!",
		"Tout",
		"Liste complète",
		"Liste des Jeux",
		"Paramètres de l'écran",
		"Le jeu a quitter! Overlay désactivé!",
		"Pas de jeu en cours! Overlay désactivé!",
		"Jeu en cours.",
		"NX-FPS est inactif!",
		"NX-FPS est actif,\nen attente d'une frame.",
		"Redémarrer l'overlay puis reéssayer.",
		"NX-FPS est actif.",
		"Le patch force les 60 Hz.",
		"Augmenter les FPS",
		"Réduire les FPS",
		"Changer les FPS",
		"Rétablir les FPS par défaut",
		"Paramètres avancés",
		"Mode d’intervalle:\n\uE019 0 (Inutilisé)",
		"Mode d’intervalle:\n\uE019 %d (%.1f FPS)",
		"Mode d’intervalle:\n\uE019 %d (%d FPS)",
		"Mode d’intervalle:\n\uE019 %d (Incorrect)",
		"FPS personnalisé:\n\uE019 désactivé",
		"FPS personnalisé:\n\uE019 %d",

		//Advanced Settings
		"Mode de Buffering",
		"Sera appliqué lors du prochain démarrage du jeu.",
		"Double",
		"Triple",
		"Triple (force)",
		"Quadruple (force)",
		"Quadruple",
		"NVN Window Sync Wait",
		"Mode",
		"Activé",
		"Semi-Actif",
		"Désactivé",
		"On",
		"Off",
		"Semi",
		"Fichier de config du jeu introuvable\nTID: %016lX\nBID: %016lX",
		"Erreur de config du jeu: 0x%X",
		"Patch introuvable.\nUtiliser \"Convertir la config en patch\"\npour le générer!",
		"Patch introuvable.",
		"Nouvelle config télécharger avec succès.\nUtiliser \"Convertir la config en patch\"\npour le rendre applicable!",
		"Patch existe.",
		"GPU API Interface: NVN",
		"Window Sync Wait",
		"GPU API Interface: EGL",
		"GPU API Interface: Vulkan",
		"FPSLocker Patches",
		"Fichier de config valide trouvé!",
		"Redémarrer le jeu après conversion!!!",
		"Convertir la config en patch",
		"Patch créé avec succès.\nRedémarrer le Jeu et changer\nles FPS pour appliquer le patch!",
		"Erreur lors de la création du patch: 0x%x",
		"Effacer le patch",
		"Patch effacé avec succès.",
		"Cela peut prendre jusqu'à %d secondes.",
		"Vérifier/télécharger le fichier de config",
		"Vérification du Warehouse pour\nla config...",
		"Divers",
		"Suspendre le jeu en arrière-plan",
		"Le patch est chargé en jeu.",
		"Master Write est chargé en jeu.",
		"Plugin n’a pas patché le jeu.",
		"Tampons Set/Actifs/Disponibles:\n\uE019 %d/%d/%d",
		"Tampons Actifs: %d",
		"Connexion expirée!",
		"Configuration indisponible! RC: 0x%x",
		"Configuration indisponible!\nAppel du Warehouse pour plus d'info...",
		"Configuration indisponible!\nAppel du Warehouse pour plus d'info...\nConnexion expirée! La requête pris trop de temps.",
		"Configuration indisponible!\nAppel du Warehouse pour plus d'info...\nErreur de connexion!",
		"Pas de nouvelle config disponible.",
		"Pas d'internet!",
		"Aucun patch n'est nécessaire pour ce jeu!",
		"Ce jeu n'est pas listé dans le Warehouse!",
		"Ce jeu est listé dans le Warehouse,\nmais avec une version différente.\n%s ne nécessitent pas de patch, votre\nversion non plus probablement!",
		"Ce jeu est listé dans le Warehouse,\nmais avec une version différente.\n%s recommandes l'utilisation de patch,\nmais aucune config n'est disponible!",
		"Ce jeu est listé dans le Warehouse,\nmais avec une version différente.\n%s ont une config disponible!",
		"Ce jeu est listé dans le Warehouse\navec le patch recommandé pour cette\nversion, mais la config est indisponible!",
		"Erreur de connexion! RC: 0x%x",
		
		//Display Settings
		"Test de saut d’images",

		"Comment l'utiliser:\n"
		"1. Obtenir une app Photo qui permet\n"
		"le réglage manuel du shutter speed\n"
		"et ISO.\n"
		"2. Régler shutter speed sur 1/10s\n"
		"ou plus, et ISO sur quelque chose\n"
		"ni trop clair ou sombre\n"
		"(En général autour de 50 pour 1/10s).\n"
		"3. Appuyer sur \uE0E0 pour continuer.\n"
		"4. Prendre une photo de l'écran.\n"
		"5. Si tous les blocs sauf le premier\n"
		"et le dernier sont inégalement\n"
		"lumineux, votre écran ne prend pas\n"
		"en charge nativement la fréquence\n"
		"actuelle, et fonctionne à une autre\n"
		"fréquence.\n\n"
		"Gardez à l'esprit que même si l'écran\n"
		"saute des images, le rendu reste\n"
		"infiniment meilleur qu'un réglage FPS\n"
		"plus bas et non adapté à la\n"
		"fréquence de l'écran. Le matériel\n"
		"assure une intervalle d’image\n"
		"constante.",

		"Appuyer sur \uE0E1 pour quitter",
		"Le rendu est trop long!\nFermer le jeu, aller sur le menu HOME,\nRéessayer.",
		
		"Ce menu passera en revue toutes\n"
		"les fréq. d'écran pris en charge sous 60 Hz:\n"
		"40, 45, 50, 55. Appuyez sur le button\n"
		"demandé pour confirmer si ça fonctionne.\n"
		"Si aucun button n'ai pressé dans les 15 secondes,\n"
		"il passera à la prochaine fréq. d'écran.",

		"Pour commencer appuyer sur X.",
		"Assistant d'underclock de l'écran",
		"Non supporté à %dp!",
		"Commencer par quitter le jeu!",
		"Appuyer sur ZL pour confirmer que %d Hz fonctionne.",
		"Appuyer sur X pour confirmer que %d Hz fonctionne.",
		"Appuyer sur Y pour confirmer que %d Hz fonctionne.",
		"Appuyer sur ZR pour confirmer que %d Hz fonctionne.",

		"Ce menu passera en revue toutes\n"
		"les fréq. d'écran pris en charge\n"
		"au-dessus de 60 Hz jusqu'a %d Hz.\n\n"
		"Appuyez sur le button demandé\n"
		"pour confirmer si ça fonctionne.\n"
		"Si aucun button n'ai pressé dans les\n"
		"10 secondes, il passera à la prochaine\n"
		"fréq. d'écran.\n"
		"Cela peut prendre jusqu'a %d secondes.",

		"Assistant d'overclock de l'écran",
		"Paramètres manuels en %dp docké",
		"Réglages suppl. écran docké",
		"Authoriser les patches à forcer 60 Hz",
		"Fréq. minimum pour les réglages FPS non sync.",
		"n/d",
		"Fréq. d'écran Max disponible: %u Hz\nTaux de transfert myDP: %s\nConfig ID: %08X",
		"Réglages écran docké",

		"Fréq. d'écran authorisée en %dp",
		"Assistant d'overclock %dp",
		"Paramètres additionnels",

		"La console n’est pas dockée.\n"
		"Revenez en arrière, placez votre\n"
		"Switch sur la station d'accueil\n"
		"et revenez.",

		"Changer Fréq. Écran",
		"Augmenter Fréq. Écran",
		"Réduire Fréq. Écran",
		"Aligner Fréq. Écran sur les FPS.",
		"Sync Écran Portable",
		"Paramètres Docké",
		"Mode Retro Remake",
		"Taux rafraîch.: %d Hz",

		"CETTE OPTION EST EXPÉRIMENTALE!\n\n"
		"Cela peut causer des dommages\n"
		"irréparables à votre écran.\n\n"
		"En cliquant sur Accepter vous\n"
		"assumez l'entière responsabilité\n"
		"de tout ce qu'il pourrait survenir\n"
		"à cause de cet outil.",

		"Alerte sur les paramètres d'affichage",
		"Refuser",
		"Accepter",
		"Forcer langue Anglais",
		"Sync Écran Docké",
		"Seulement en mode portable",
		"HOME Menu à 60Hz"
	};

	const std::array teslaStrings = {
	   "Retour",
	   "OK"
	};

	static_assert(teslaStrings.size() == 2);
}